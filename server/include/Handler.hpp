#ifndef CHAT_HANDLER_HPP
#define CHAT_HANDLER_HPP

#include <uwebsockets/App.h>
#include <string>

namespace Chat
{
    struct PerSocketData;

    class IHandler
    {
    public:
        using uWebSocket = uWS::WebSocket<false,true,PerSocketData>;
    public:
        virtual ~IHandler() = default;
        virtual void handle(uWebSocket* ws, const std::string& message) = 0;
    };
}

#endif /*CHAT_HANDLER_HPP*/