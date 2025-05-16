#include "CommandRouter.hpp"
#include "UserManager.hpp"
#include "TopicManager.hpp"

#include <nlohmann/json.hpp>
#include <regex>

namespace Chat
{
    CommandRouter::CommandRouter(UserManager& userManager, TopicManager& topicManager)
    : _userManager(userManager)
    , _topicManager(topicManager)
    {}

    void CommandRouter::handle(uWebSocket* ws, const std::string& message)
    {
        auto *user = ws->getUserData();

        const nlohmann::json error = 
        {
            { "type", "error" },
            { "message", "Unknow or malformed command" }
        };

        try
        {
            auto parsed_message = nlohmann::json::parse(message);

            if (!parsed_message.contains("type") || !parsed_message["type"].is_string())
            {
                ws->send(error.dump(), uWS::OpCode::TEXT);
            }

            const auto& type = parsed_message["type"].get<std::string>();

            if (type == "register")
            {
                if (!parsed_message.contains("name") || !parsed_message["name"].is_string())
                {
                    ws->send(error.dump(), uWS::OpCode::TEXT);
                    return;
                }
                user->userName = parsed_message["name"].get<std::string>();
                nlohmann::json response =
                    {
                        { "type", "registred" },
                        { "name", user->userName }
                    };

                ws->send(response.dump(), uWS::OpCode::TEXT);
                for (auto* socket : _userManager.getAllSockets())
                {
                    if (socket && socket != ws) broadscastUsersList(socket, 0);
                }
                return;
            }
            else if (type == "list")
            {
                int count = -1;
                if (parsed_message.contains("count") && parsed_message["count"].is_number_integer())
                {
                    count = parsed_message["count"].get<int>();
                }

                broadscastUsersList(ws, count);
                return;
            }
            else if (type == "pm")
            {
                if (!parsed_message.contains("to") || !parsed_message.contains("text") || 
                    !parsed_message["to"].is_string() || !parsed_message["text"].is_string())
                {
                    ws->send(error.dump(), uWS::OpCode::TEXT);
                    return;
                }
                auto socket = _userManager.getSocket(parsed_message["to"].get<std::string>());
                if (socket)
                {
                    socket->send(nlohmann::json{{"type", "private"}, 
                                                {"from", user->id}, 
                                                {"text", parsed_message["text"].get<std::string>()}}.dump(), uWS::OpCode::TEXT);
                }
                else
                {
                    ws->send(error.dump(), uWS::OpCode::TEXT);
                    return;
                }
                return;
            }
            else if (type == "join")
            {
                if (!parsed_message.contains("topic") || !parsed_message["topic"].is_string())
                {
                    ws->send(error.dump(), uWS::OpCode::TEXT);
                    return;
                }

                const std::string topic = parsed_message["topic"].get<std::string>();
                _topicManager.create(topic);
                _topicManager.join(topic, user->id);

                nlohmann::json response = 
                {
                    { "type", "joined" }, 
                    { "topic", "topic" }
                };

                ws->send(response.dump(), uWS::OpCode::TEXT);
                return;
            }
            else if (type == "topic")
            {
                if (!parsed_message.contains("topic") || !parsed_message.contains("text") ||
                    !parsed_message["topic"].is_string() || !parsed_message["text"].is_string())
                {
                    ws->send(error.dump(), uWS::OpCode::TEXT);
                    return;
                }

                const std::string topic = parsed_message["topic"].get<std::string>();
                const std::string text = parsed_message["text"].get<std::string>();

                auto members = _topicManager.members(topic);
                nlohmann::json response = 
                {
                    { "type", "topic" },
                    { "topic", topic },
                    { "from", user->userName },
                    { "text", text }
                };

                for (const auto& mem : members)
                {
                    if (mem == user->id) continue;
                    auto socket = _userManager.getSocket(mem);
                    if (socket) socket->send(response.dump(), uWS::OpCode::TEXT);
                }

                return;
            }
        }
        catch(const nlohmann::json::parse_error& e)
        {
            ws->send("Invalid JSON format", uWS::OpCode::TEXT);
        }
        catch(const std::exception& e)
        {
            ws->send("Internal error: " + std::string(e.what()), uWS::OpCode::TEXT);
        }
    }

    void CommandRouter::broadscastUsersList(uWebSocket* ws, int limit)
    {
        nlohmann::json userList = nlohmann::json::array();
        auto users = _userManager.listUsers(limit);

        for (const auto& u : users) userList.push_back({{"id", u->id}, {"name", u->userName}});

        nlohmann::json response =
        {
            { "type", "user_list"},
            { "users", userList }
        };

        ws->send(response.dump(), uWS::OpCode::TEXT);
    }

    void CommandRouter::broadscastDisconnectUser(const std::string& id)
    {
        nlohmann::json response = 
        {
            { "type", "disconnect" },
            { "id", id }
        };

        for (auto* socket : _userManager.getAllSockets())
        {
            if (socket) socket->send(response.dump(), uWS::OpCode::TEXT);
        }
    }
}