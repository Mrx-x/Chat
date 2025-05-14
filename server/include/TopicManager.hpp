#ifndef CHAT_TOPICMANAGER_HPP
#define CHAT_TOPICMANAGER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Chat
{
    class TopicManager
    {
    public:
        TopicManager() = default;

    public:
        void create(const std::string& topic);
        void join(const std::string& topic, const std::string& userId);
        std::vector<std::string> members(const std::string& topic) const;

    private:
        std::unordered_map<std::string, std::unordered_set<std::string>> _topics;
    };
}

#endif /*CHAT_TOPICMANAGER_HPP*/