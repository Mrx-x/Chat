#include "Server.hpp"

#include <iostream>
#include <regex>

namespace Chat
{
    void ChatServer::run(int port)
    {
        uWS::App().ws<PerSocketData>("/*", {.open = [this](auto *ws)
                                            { 
                auto* user = ws->getUserData();
                const std::size_t countUsers = _clients.size();
                
                user->id = "User_" + std::to_string(countUsers + 1);
                user->userName = user->id;

                _clients[user->id] = ws;
                
                std::cout << "[+] Client connected: " << user->id << "\n";

                nlohmann::json response =
                {
                    { "command", "Welcome" },
                    { "id", user->id }
                };

                ws->send(response.dump(), uWS::OpCode::TEXT); },
                                            .message = [this](auto *ws, std::string_view msg, uWS::OpCode)
                                            { handleMessage(ws, msg); },
                                            .close = [this](auto *ws, int, std::string_view)
                                            { std::cout << "Client disconnected.\n"; }})
            .listen(port, [port](auto *token)
                    {
                if (token) std::cout << "Server is listening on port" << port << std::endl; })
            .run();
    }

    void ChatServer::handleMessage(auto *ws, std::string_view msg)
    {
        auto *user = ws->getUserData();
        std::string message(msg);

        std::smatch match;
        static const std::regex registerCommand(R"(^/register\s+(\w+)$)");
        static const std::regex listCommand(R"(^/list(?:\s+(\d+))?$)");
        static const std::regex privateMsgCommand(R"(^/pm\s+(\w+)\s+(.+)$)");
        static const std::regex joinCommand(R"(^join\s+(\w+)$)");
        static const std::regex topicCommand(R"(^topic\s+(\w+)\s+(.+)$)");

        if (std::regex_match(message, match, registerCommand))
        {
            std::string newName = match[1];
            user->userName = newName;

            nlohmann::json response =
                {
                    {"type", "registred"},
                    {"name", newName}
                };

            ws->send(response.dump(), uWS::OpCode::TEXT);
            return;
        }

        if (std::regex_match(message, match, listCommand))
        {
            int limit = match[1].matched ? std::stoi(match[1]) : -1;
            nlohmann::json userList = nlohmann::json::array();

            int count = 0;
            for (const auto &[id, socket] : _clients)
            {
                const auto *userData = socket->getUserData();
                userList.push_back({{"id", userData->id}, {"name", userData->userName}});

                if (limit > 0 && ++count >= limit)
                    break;
            }

            ws->send(userList.dump(), uWS::OpCode::TEXT);
            return;
        }

        // /pm <user_id> <message>
        if (std::regex_match(message, match, privateMsgCommand))
        {
            const std::string targetId = match[1];
            const std::string text = match[2];

            if (auto it = _clients.find(targetId); it != _clients.end())
            {
                nlohmann::json privateMsg =
                    {
                        {"type", "private"},
                        {"from", user->userName},
                        {"message", text}
                    };

                it->second->send(privateMsg.dump(), uWS::OpCode::TEXT);
            }
            else
            {
                nlohmann::json err =
                    {
                        {"type", "error"},
                        {"message", "User not found"}
                    };
                ws->send(err.dump(), uWS::OpCode::TEXT);
            }
            return;
        }

        // /join <topic>
        if (std::regex_match(message, match, joinCommand))
        {
            const std::string topic = match[1];
            _topics[topic].insert(user->id);

            nlohmann::json response = 
            {
                { "type", "joined" }, 
                { "topic", "topic" }
            };

            ws->send(response.dump(), uWS::OpCode::TEXT);
        }

        // /topic <topic> <message>
        if (std::regex_match(message, match, topicCommand))
        {
            const std::string topic = match[1];
            const std::string text = match[2];

            if (_topics.contains(topic))
            {
                const nlohmann::json topicMsg =
                {
                    { "type", "topic" },
                    { "topic", topic },
                    { "from", user->userName },
                    { "message", text }
                };

                for (const auto& memberId : _topics[topic])
                {
                    if (memberId == user->id) continue;

                    if (auto it = _clients.find(memberId); it != _clients.end())
                    {
                        it->second->send(topicMsg.dump(), uWS::OpCode::TEXT);
                    }
                }
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