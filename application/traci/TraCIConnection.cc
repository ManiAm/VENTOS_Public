
#define WANT_WINSOCK2
#include <platdep/sockets.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <cenvir.h>
#include <cexception.h>
#include <algorithm>
#include <functional>

#include "TraCIConnection.h"
#include "TraCIConstants.h"
#include "TraCICommands.h"

namespace VENTOS {

pid_t TraCIConnection::pid = -1;

SOCKET socket(void* ptr)
{
    ASSERT(ptr);
    return *static_cast<SOCKET*>(ptr);
}


TraCIConnection::TraCIConnection(void* ptr) : socketPtr(ptr)
{
    ASSERT(socketPtr);
}


TraCIConnection::~TraCIConnection()
{
    if (socketPtr)
    {
        closesocket(socket(socketPtr));
        delete static_cast<SOCKET*>(socketPtr);
    }

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#else
    // send SIGINT
    if (pid)
        kill(pid, 15);
#endif
}


int TraCIConnection::startServer(std::string SUMOexe, std::string SUMOconfig, std::string switches, int seed)
{
    int port = TraCIConnection::getFreeEphemeralPort();

    // auto-set seed, if requested
    if (seed == -1)
    {
        const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
        seed = atoi(seed_s);
    }

    std::cout << std::endl;
    printf(">>> Starting SUMO TraCI server on port %d with seed %d ... \n", port, seed);
    std::cout.flush();

    // assemble commandLine
    std::ostringstream commandLine;
    commandLine << SUMOexe
            << " --remote-port " << port
            << " --seed " << seed
            << " --configuration-file " << SUMOconfig
            << switches;

    TraCIConnection::TraCILauncher(commandLine.str());

    return port;
}


// get a random Ephemeral port from the system
int TraCIConnection::getFreeEphemeralPort()
{
    if (initsocketlibonce() != 0)
        throw cRuntimeError("Could not init socketlib");

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        throw cRuntimeError("Failed to create socket: %s", strerror(errno));

    struct sockaddr_in serv_addr;
    struct sockaddr* serv_addr_p = (struct sockaddr*)&serv_addr;
    memset(serv_addr_p, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = 0;   // get a random Ephemeral port

    if (::bind(sock, serv_addr_p, sizeof(serv_addr)) < 0)
        throw cRuntimeError("Failed to bind socket: %s", strerror(errno));

    socklen_t len = sizeof(serv_addr);
    if (getsockname(sock, serv_addr_p, &len) < 0)
        throw cRuntimeError("Failed to get hostname: %s", strerror(errno));

    int port = ntohs(serv_addr.sin_port);

    closesocket(sock);

    return port;
}


void TraCIConnection::TraCILauncher(std::string commandLine)
{
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFOA);

    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    char cmdline[32768];
    strncpy(cmdline, commandLine.c_str(), sizeof(cmdline));
    bool bSuccess = CreateProcess(0, cmdline, 0, 0, 1, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, 0, 0, &si, &pi);
    if (!bSuccess)
    {
        std::string msg = "undefined error";

        DWORD errorMessageID = ::GetLastError();
        if(errorMessageID != 0)
        {

            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            std::string message(messageBuffer, size);
            LocalFree(messageBuffer);
            msg = message;
        }

        msg = std::string() + "Error launching TraCI server (\"" + commandLine + "\"): " + msg + ". Make sure you have set $PATH correctly.";

        throw cRuntimeError(msg.c_str());
    }
    else
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

#else

    // create a child process
    // forking creates an exact copy of the parent process at the time of forking.
    pid = fork();

    // fork failed
    if(pid < 0)
        throw cRuntimeError("fork() failed!");

    // in child process
    if(pid == 0)
    {
        // make the child process ignore the SIGINT signal
        signal(SIGINT, SIG_IGN);

        // run SUMO server inside this child process
        // if execution is successful then child will be blocked at this line
        int r = system(commandLine.c_str());

        if (r == -1)
            throw cRuntimeError("Running \"%s\" failed during system()", commandLine.c_str());

        if (WEXITSTATUS(r) != 0)
            throw cRuntimeError("Error launching TraCI server (\"%s\"): exited with code %d.", commandLine.c_str(), WEXITSTATUS(r));

        exit(1);
    }
    else
    {
        printf("  Parent PID %d \n", getpid());
        printf("  Child  PID %d \n", pid);
    }

#endif
}


TraCIConnection* TraCIConnection::connect(const char* host, int port)
{
    std::cout << std::endl;
    printf(">>> Connecting to TraCI server on port %d ... \n", port);
    std::cout.flush();

    if (initsocketlibonce() != 0)
        throw cRuntimeError("Could not init socketlib");

    in_addr addr;
    struct hostent* host_ent;
    struct in_addr saddr;

    saddr.s_addr = inet_addr(host);
    if (saddr.s_addr != static_cast<unsigned int>(-1))
        addr = saddr;
    else if ((host_ent = gethostbyname(host)))
        addr = *((struct in_addr*) host_ent->h_addr_list[0]);
    else
        throw cRuntimeError("Invalid TraCI server address: %s", host);

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*)&address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = addr.s_addr;

    SOCKET* socketPtr = new SOCKET();
    if (*socketPtr < 0) throw cRuntimeError("Could not create socket to connect to TraCI server");

    for (int tries = 1; tries <= 10; ++tries)
    {
        *socketPtr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(*socketPtr, address_p, sizeof(address)) >= 0)
            break;

        closesocket(socket(socketPtr));

        std::stringstream ss;
        ss << "Could not connect to TraCI server; error message: " << sock_errno() << ": " << strerror(sock_errno());
        std::string msg = ss.str();

        int sleepDuration = tries*.25 + 1;

        if (tries >= 10)
            throw cRuntimeError(msg.c_str());
        else if (tries == 3)
            std::cout << msg << " -- Will retry in " << sleepDuration << " second(s). \n" << std::flush;

        sleep(sleepDuration);
    }

    {
        int x = 1;
        ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));
    }

    return new TraCIConnection(socketPtr);
}


TraCIBuffer TraCIConnection::query(uint8_t commandGroupId, const TraCIBuffer& buf)
{
    sendMessage(makeTraCICommand(commandGroupId, buf));

    TraCIBuffer obuf(receiveMessage());
    uint8_t cmdLength; obuf >> cmdLength;
    uint8_t commandResp; obuf >> commandResp;
    ASSERT(commandResp == commandGroupId);
    uint8_t result; obuf >> result;
    std::string description; obuf >> description;

    if (result == RTYPE_NOTIMPLEMENTED)
        throw cRuntimeError("TraCI server reported command 0x%2x not implemented (\"%s\"). Might need newer version.",
                commandGroupId,
                description.c_str());

    if (result == RTYPE_ERR)
        throw cRuntimeError("TraCI server reported throw cRuntimeError executing command 0x%2x (\"%s\").",
                commandGroupId,
                description.c_str());

    ASSERT(result == RTYPE_OK);

    return obuf;
}


TraCIBuffer TraCIConnection::queryOptional(uint8_t commandGroupId, const TraCIBuffer& buf, bool& success, std::string* errorMsg)
{
    sendMessage(makeTraCICommand(commandGroupId, buf));

    TraCIBuffer obuf(receiveMessage());
    uint8_t cmdLength; obuf >> cmdLength;
    uint8_t commandResp; obuf >> commandResp;
    ASSERT(commandResp == commandGroupId);
    uint8_t result; obuf >> result;
    std::string description; obuf >> description;
    success = (result == RTYPE_OK);
    if (errorMsg)
        *errorMsg = description;

    return obuf;
}


std::string TraCIConnection::receiveMessage()
{
    if (!socketPtr)
        throw cRuntimeError("Not connected to TraCI server");

    uint32_t msgLength;

    {
        char buf2[sizeof(uint32_t)];
        uint32_t bytesRead = 0;
        while (bytesRead < sizeof(uint32_t))
        {
            int receivedBytes = ::recv(socket(socketPtr), reinterpret_cast<char*>(&buf2) + bytesRead, sizeof(uint32_t) - bytesRead, 0);
            if (receivedBytes > 0)
                bytesRead += receivedBytes;
            else if (receivedBytes == 0)
            {
                printf("ERROR in receiveMessage: Connection to TraCI server closed unexpectedly.\n");
                std::cout.flush();
                terminateSimulation();
            }
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                printf("ERROR in receiveMessage: Connection to TraCI server lost: %d: %s\n", sock_errno(), strerror(sock_errno()));
                std::cout.flush();
                terminateSimulation();
            }
        }

        TraCIBuffer(std::string(buf2, sizeof(uint32_t))) >> msgLength;
    }

    uint32_t bufLength = msgLength - sizeof(msgLength);
    char buf[bufLength];

    {
        uint32_t bytesRead = 0;
        while (bytesRead < bufLength)
        {
            int receivedBytes = ::recv(socket(socketPtr), reinterpret_cast<char*>(&buf) + bytesRead, bufLength - bytesRead, 0);
            if (receivedBytes > 0)
                bytesRead += receivedBytes;
            else if (receivedBytes == 0)
            {
                printf("ERROR in receiveMessage: Connection to TraCI server closed unexpectedly.\n");
                std::cout.flush();
                terminateSimulation();
            }
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                printf("ERROR in receiveMessage: Connection to TraCI server lost: %d: %s\n", sock_errno(), strerror(sock_errno()));
                std::cout.flush();
                terminateSimulation();
            }
        }
    }

    return std::string(buf, bufLength);
}


void TraCIConnection::sendMessage(std::string buf)
{
    if (!socketPtr)
        throw cRuntimeError("Not connected to TraCI server");

    {
        uint32_t msgLength = sizeof(uint32_t) + buf.length();
        TraCIBuffer buf2 = TraCIBuffer();
        buf2 << msgLength;
        uint32_t bytesWritten = 0;
        while (bytesWritten < sizeof(uint32_t))
        {
            size_t sentBytes = ::send(socket(socketPtr), buf2.str().c_str() + bytesWritten, sizeof(uint32_t) - bytesWritten, 0);
            if (sentBytes > 0)
                bytesWritten += sentBytes;
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                printf("ERROR in sendMessage: Connection to TraCI server lost: %d: %s\n", sock_errno(), strerror(sock_errno()));
                std::cout.flush();
                terminateSimulation();
            }
        }
    }

    {
        uint32_t bytesWritten = 0;
        while (bytesWritten < buf.length())
        {
            size_t sentBytes = ::send(socket(socketPtr), buf.c_str() + bytesWritten, buf.length() - bytesWritten, 0);
            if (sentBytes > 0)
                bytesWritten += sentBytes;
            else
            {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;

                printf("ERROR in sendMessage: Connection to TraCI server lost: %d: %s\n", sock_errno(), strerror(sock_errno()));
                std::cout.flush();
                terminateSimulation();
            }
        }
    }
}


std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf)
{
    if (sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length() > 0xFF)
    {
        uint32_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + buf.str().length();
        return (TraCIBuffer() << static_cast<uint8_t>(0) << len << commandId).str() + buf.str();
    }

    uint8_t len = sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length();
    return (TraCIBuffer() << len << commandId).str() + buf.str();
}


void TraCIConnection::terminateSimulation()
{
    // get a pointer to TraCI module
    cModule *module = simulation.getSystemModule()->getSubmodule("TraCI");
    ASSERT(module);
    TraCI_Commands *TraCI = static_cast<TraCI_Commands *>(module);
    ASSERT(TraCI);

    // set TraCIclosed flag that is used in the finish()
    TraCI->par("TraCIclosed") = true;

    // end the simulation
    TraCI->endSimulation();
}

}
