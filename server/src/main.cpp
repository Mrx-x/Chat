#include "Server.hpp"

int main()
{
    Chat::ChatServer server;
    server.run(9002);
}