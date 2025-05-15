#include "UserManager.hpp"

namespace Chat
{
    std::string UserManager::addConnection(uWebSocket* ws)
    {
        std::string id = "user_" + std::to_string(_sockets.size() + 1);
        _sockets[id] = ws;
        return id;
    }

    void UserManager::removeConnection(const std::string& id)
    {
        _sockets.erase(id);
    }

    void UserManager::setName(const std::string& id, const std::string& name)
    {
        if (auto it = _sockets.find(id); it != _sockets.end())
        {
            it->second->getUserData()->userName = name;
        }
    }

    std::vector<PerSocketData*> UserManager::listUsers(std::size_t limit) const
    {
        std::vector<PerSocketData*> result;
        result.reserve(_sockets.size());

        std::size_t count = 0;
        for (const auto& [id, socket] : _sockets)
        {
            result.push_back(socket->getUserData());
            if (limit && ++count >= limit) break;
        }
        return result;
    }

    UserManager::uWebSocket* UserManager::getSocket(const std::string& id) const
    {
        if (auto it = _sockets.find(id); it != _sockets.end())
            return it->second;
        else
            return nullptr;
    }

    std::vector<UserManager::uWebSocket*> UserManager::getAllSockets() const
    {
        std::vector<uWebSocket*> result;
        result.reserve(_sockets.size());
        for (const auto& [id, socket] : _sockets) result.push_back(socket);
        return result;
    }

    PerSocketData* UserManager::getUser(const std::string& id) const
    {
        if (auto it = _sockets.find(id); it != _sockets.end())
            return it->second->getUserData();
        else 
            return nullptr;
    }
}