#include "Server.hpp"

#include <iostream>
#include <regex>

namespace Chat
{
    static const std::string generalTopic = "general";

    ChatServer::ChatServer()
    : _userManager(), _topicManager(), _commandRouter(_userManager, _topicManager)
    {}

    void ChatServer::run(int port)
    {
        _topicManager.create(generalTopic);

        uWS::App().ws<PerSocketData>("/*", 
        {
            .open = [this](auto *ws) 
            { 
                auto id = _userManager.addConnection(ws);
                auto* user = ws->getUserData();
                
                user->id = id;
                user->userName = user->id;

                _topicManager.join(generalTopic, id);

                std::cout << "[+] Client connected: " << user->id << "\n";

                nlohmann::json response =
                {
                    { "type", "welcome" },
                    { "id", user->id }
                };

                ws->send(response.dump(), uWS::OpCode::TEXT);

                nlohmann::json joinedResponse = 
                {
                    { "type", "joined" },
                    { "topic", generalTopic }
                };

                ws->send(joinedResponse.dump(), uWS::OpCode::TEXT);
            },
            .message = [this](auto *ws, std::string_view msg, uWS::OpCode)
            { 
                _commandRouter.handle(ws, std::string(msg));
            },
            .close = [this](auto *ws, int, std::string_view)
            { 
                auto id = ws->getUserData()->id;
                _userManager.removeConnection(id);
                std::cout << "[-] " << id << " disconected\n";
                _commandRouter.broadscastDisconnectUser(id);
            }
        })
        .listen(port, [port](auto *token) 
        {
            std::cout << (token ? "[*] Listening " : "[!] Failed ") << port << '\n';
        })
        .run();
    }
}