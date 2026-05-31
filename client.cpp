// MSDN recommends to use windows.h and winsock2.h with WIN32_LEAN_AND_MEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

const char* UDP_IP = "0.0.0.0";
const int UDP_PORT = 64000;

/**
 * Gets the current time in seconds.
 * @return The current time in seconds.
 * TODO: refactor
 */
double getTime()
{
    return static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()) / 1000.0;
}

/**
 * Prints the result of a received UDP packet.
 * @param senderIP The IP address of the sender.
 * @param senderPort The port of the sender.
 * @param buf The received data.
 */
void printResult(char* senderIP, int senderPort, char* buf)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::cout << "["
              << st.wYear << "-"
              << std::setfill('0') << std::setw(2) << st.wMonth << "-"
              << std::setfill('0') << std::setw(2) << st.wDay << " "
              << std::setfill('0') << std::setw(2) << st.wHour << ":"
              << std::setfill('0') << std::setw(2) << st.wMinute << ":"
              << std::setfill('0') << std::setw(2) << st.wSecond << "."
              << std::setfill('0') << std::setw(3) << st.wMilliseconds
              << "] ";
    std::cout << "Received from " << senderIP << ":" << senderPort
                  << " -> " << buf << std::endl;
}

/**
 * Initializes Winsock, creates a UDP socket, sets a receive timeout,
 * and binds the socket to UDP_PORT on all local interfaces.
 *
 * @return A valid SOCKET on success, or INVALID_SOCKET on failure.
 */
SOCKET initSocket(){
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return INVALID_SOCKET;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    DWORD timeout = 500;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    sockaddr_in bindAddr{};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(UDP_PORT);
    bindAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    return sock;
}

int main()
{
    SOCKET sock = initSocket();
    if (sock == INVALID_SOCKET){
        return 1;
    }
    std::cout << "Listening for UDP packets on port " << UDP_PORT << "..." << std::endl;

    SetConsoleCtrlHandler([](DWORD) -> BOOL {
        std::cout << "\nExiting..." << std::endl;
        return FALSE; // Let default handler terminate
    }, TRUE);

    char buf[1024];
    sockaddr_in senderAddr{};
    int senderAddrSize = sizeof(senderAddr);

    double lastReceivedTime = 0.0;
    lastReceivedTime = getTime();

    while (true) {
        int recvLen = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                               (sockaddr*)&senderAddr, &senderAddrSize);

        if (recvLen == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) {
                continue;
            }
            std::cerr << "recvfrom failed: " << err << std::endl;
            break;
        }

        buf[recvLen] = '\0';
        // trim recv string
        while (recvLen > 0 && (buf[recvLen - 1] == '\n' || buf[recvLen - 1] == '\r' || buf[recvLen - 1] == ' ')) {
            buf[--recvLen] = '\0';
        }

        // Get sender IP
        char senderIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &senderAddr.sin_addr, senderIP, sizeof(senderIP));
        int senderPort = ntohs(senderAddr.sin_port);

        printResult(senderIP, senderPort, buf);

        lastReceivedTime = getTime();
    }


    closesocket(sock);
    WSACleanup();
    return 0;
}