#ifndef CHAT_SERVER
#define CHAT_SERVER

#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "UserManager.hpp"
#include "TopicManager.hpp"
#include "CommandRouter.hpp"

namespace Chat
{
    class ChatServer
    {
    public:
        ChatServer();
        void run(int port);

    private:
        UserManager _userManager;
        TopicManager _topicManager;
        CommandRouter _commandRouter;
    };
}
#endif /*CHAT_SERVER*/