#include "Server.hpp"
#include <iostream>
#include <string.h>

const unsigned int       MAX_PORT_VALUE = 65535; // max for short int
const unsigned short int DEFAULT_PORT   = 9002;
const unsigned int       ERROR_PORT     = 0;

static unsigned int parsePort(int argc, char *argv[]) 
{
    static int         paramStrCount = 6;
    static const char *portParamName = "--port";
    if (argc <= 1) {
        return DEFAULT_PORT;
    }
    int paramIdx = 0;
    
    // finds first occurance
    while (paramIdx < argc) {
    const char *cmd = argv[paramIdx];
        if (strncmp(portParamName, cmd, paramStrCount) == 0) {
          break;
        }
        ++paramIdx;
    }
    if (paramIdx == argc) {
        return DEFAULT_PORT;
    }
    // last param,thus no port next to it
    if (paramIdx == argc - 1) {
        std::cerr << "Found " << portParamName << " at last parameter!Add port value after it.";
        return ERROR_PORT;
    }
    unsigned int port = atoi(argv[paramIdx + 1]);
    if (!port) {
        std::cerr << "Cannot convert value " << argv[paramIdx + 1] << " to usable port value!";
        return ERROR_PORT;
    }
    int overflowCheck = MAX_PORT_VALUE - port;
    if ((overflowCheck) < 0) {
        std::cerr << "Too big port value (" << argv[paramIdx + 1] << ")!";
        return ERROR_PORT;
    }
    
    return port;
}

int main(int argc,char *argv[])
{   
    short int port=parsePort(argc,argv);
    if(port==ERROR_PORT){
        return 1;
    }
    Chat::ChatServer server;
    server.run(9002);
    return 1;
}
