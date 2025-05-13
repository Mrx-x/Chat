#ifndef CHAT_SERVER
#define CHAT_SERVER

#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace Chat
{
    struct PerSocketData
    {
        std::string id;
        std::string userName;
    };

    class ChatServer
    {
        using ClientsMap = std::unordered_map<std::string, uWS::WebSocket<false, true, PerSocketData>*>;
        using TopicsMap = std::unordered_map<std::string, std::unordered_set<std::string>>;
    public:
        ChatServer() = default;
        void run(int port);

    private:
        void handleMessage(auto *ws, std::string_view message);

    private:
        ClientsMap _clients;
        TopicsMap _topics;
    };
}
#endif /*CHAT_SERVER*/