#ifndef CHAT_USERMANAGER_HPP
#define CHAT_USERMANAGER_HPP

#include <uwebsockets/App.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace Chat
{
    struct PerSocketData
    {
        std::string id;
        std::string userName;
    };

    class UserManager
    {
        using uWebSocket = uWS::WebSocket<false,true,PerSocketData>;
    public:
        UserManager() = default;

    public:
        std::string addConnection(uWebSocket* ws);
        void removeConnection(const std::string& id);
        void setName(const std::string& id, const std::string& name);
        std::vector<PerSocketData*> listUsers(std::size_t limit) const;
        uWebSocket* getSocket(const std::string& id) const;
        std::vector<uWebSocket*> getAllSockets() const;
        PerSocketData* getUser(const std::string& id) const;

    private:
        std::unordered_map<std::string, uWebSocket*> _sockets;
        std::size_t _counter = 0;
    };
}

#endif /*CHAT_USERMANAGER_HPP*/