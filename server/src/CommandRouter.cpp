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

        std::smatch match;
        static const std::regex registerCommand(R"(^/register\s+(\w+)$)");
        static const std::regex listCommand(R"(^/list(?:\s+(\d+))?$)");
        static const std::regex privateMsgCommand(R"(^/pm\s+(\w+)\s+(.+)$)");
        static const std::regex joinCommand(R"(^/join\s+(\w+)$)");
        static const std::regex topicCommand(R"(^/topic\s+(\w+)\s+(.+)$)");

        if (std::regex_match(message, match, registerCommand))
        {
            std::string newName = match[1];
            user->userName = newName;
            _userManager.setName(user->id, newName);

            nlohmann::json response =
                {
                    { "type", "registred" },
                    { "name", newName }
                };

            ws->send(response.dump(), uWS::OpCode::TEXT);
            return;
        }

        if (std::regex_match(message, match, listCommand))
        {
            int limit = match[1].matched ? std::stoi(match[1]) : -1;
            nlohmann::json userList = nlohmann::json::array();

            auto users = _userManager.listUsers(limit);
            for (const auto& u : users) userList.push_back({{"id", u->id}, {"name", u->userName}});

            ws->send(userList.dump(), uWS::OpCode::TEXT);
            return;
        }

        // /pm <user_id> <message>
        if (std::regex_match(message, match, privateMsgCommand))
        {
            const std::string targetId = match[1];
            const std::string text = match[2];

            auto socket = _userManager.getSocket(targetId);
            if (!socket)
            {
                ws->send(nlohmann::json{{"type", "error"}, {"message", "User not found"}}.dump(), uWS::OpCode::TEXT);
            }
            else
            {
                socket->send(nlohmann::json{{"type", "private"}, {"from", user->userName}, {"text", text}}.dump(), uWS::OpCode::TEXT);
            }
            return;
        }

        // /join <topic>
        if (std::regex_match(message, match, joinCommand))
        {
            const std::string topic = match[1];
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

        // /topic <topic> <message>
        if (std::regex_match(message, match, topicCommand))
        {
            const std::string topic = match[1];
            const std::string text = match[2];

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

        const nlohmann::json error = 
        {
            { "type", "error" },
            { "message", "Unknow or malformed command" }
        };

        ws->send(error.dump(), uWS::OpCode::TEXT);
    }
}