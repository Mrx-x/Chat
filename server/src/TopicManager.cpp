#include "TopicManager.hpp"

namespace Chat
{
    void TopicManager::create(const std::string& topic)
    {
        _topics.emplace(topic, std::unordered_set<std::string>{});
    }

    void TopicManager::join(const std::string& topic, const std::string& userId)
    {
        if (auto it = _topics.find(topic); it != _topics.end())
        {
            it->second.insert(userId);
        }
    }

    std::vector<std::string> TopicManager::members(const std::string& topic) const
    {
        if (auto it = _topics.find(topic); it != _topics.end())
        {
            return { it->second.begin(), it->second.end() };
        }
        return {};
    }
}