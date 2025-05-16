#ifndef CHAT_COMMANDROUTER_HPP
#define CHAT_COMMANDROUTER_HPP

#include "Handler.hpp"

namespace Chat
{
    class UserManager;
    class TopicManager;

    class CommandRouter : public IHandler
    {
    public:
        CommandRouter(UserManager& userManager, TopicManager& topicManager);

    public:
        void handle(uWebSocket* ws, const std::string& message) override;
        void broadscastUsersList(uWebSocket* ws, int limit = 0);
        void broadscastDisconnectUser(const std::string& id);
    private:
        void registerUser();
        void listUsers();
        void privateMessage();
        void joinTopic();
        void topicMessage();

    private:
        UserManager& _userManager;
        TopicManager& _topicManager;
    };
}

#endif /*CHAT_COMMANDROUTER_HPP*/