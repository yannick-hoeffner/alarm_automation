// MSDN recommends to use windows.h and winsock2.h with WIN32_LEAN_AND_MEAN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

const char* UDP_IP = "0.0.0.0";
const int UDP_PORT = 64000;

const char* ALARM_MESSAGE = "alarm received";
const char* ALARM_CLEAR_MESSAGE = "alarm cleared";

/**
 * Gets the current time in seconds using mononical clock.
 * @return The current time in seconds.
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

/**
 * Starts a new process with the given application name.
 * @param lpApplicationName See https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw
 * for more information about the name parameter.
 * @return A PROCESS_INFORMATION structure containing information about the newly created process.
 */
void startProcess(LPCTSTR lpApplicationName, PROCESS_INFORMATION* pi) {
    // additional information not set
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    // struct for receiving process information
    ZeroMemory(pi, sizeof(*pi));

    if (!CreateProcess( lpApplicationName,   // the path
        NULL,           // No Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        pi             // Pointer to PROCESS_INFORMATION structure
    )) {
        std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
    }
}

int main()
{
    SOCKET sock = initSocket();
    if (sock == INVALID_SOCKET){
        return 1;
    }
    std::cout << "Listening for UDP packets on port " << UDP_PORT << "..." << std::endl;

    // TODO: Handle Ctrl+C to exit gracefully
    SetConsoleCtrlHandler([](DWORD) -> BOOL {
        std::cout << "\nExiting..." << std::endl;
        return FALSE; // Let default handler terminate
    }, TRUE);

    // vars for recvfrom
    char buf[1024];
    sockaddr_in senderAddr{};
    int senderAddrSize = sizeof(senderAddr);

    // vars for process management
    double lastAlarmTime = 0.0;
    PROCESS_INFORMATION pi{};
    bool processRunning = false;

    while (true) {
        if (processRunning){
            // check whether the process is still running
            DWORD exitCode;
            if (GetExitCodeProcess(pi.hProcess, &exitCode)){
                if (exitCode != STILL_ACTIVE){
                    std::cout << "Alarm process has exited with code: " << exitCode << std::endl;
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    processRunning = false;
                }
            }
        }
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
        if (strcmp(buf, ALARM_MESSAGE) == 0) {
            // =================================
            // handle alarm
            // =================================
            double currentTime = getTime();
            std::cout << "Alarm received!" << lastAlarmTime << " -> " << currentTime << std::endl;

            if (!processRunning){
                // The ESP sends multiple alarm messages per second,
                // to avoid informing about the same alarm multiple times, 
                // we only start the process if at least 60 seconds have passed since the last alarm.
                if (currentTime - lastAlarmTime >= 10.0) { // TODO: Change back to 60.0 for production
                    startProcess(TEXT("alarm.exe"), &pi);
                    processRunning = true;
                }
            }

            lastAlarmTime = currentTime;
        }else if(strcmp(buf, ALARM_CLEAR_MESSAGE) == 0){
            // =================================
            // handle alarm clear
            // =================================
            std::cout << "Alarm cleared!" << std::endl;
            if (processRunning){
                TerminateProcess(pi.hProcess, 0);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                processRunning = false;
                std::cout << "Alarm process terminated." << std::endl;
            }
            // alarm cleared is an immediate indicator that every
            // following alarm message is a new alarm, so we reset lastAlarmTime
            lastAlarmTime = 0.0;
        }
    }


    closesocket(sock);
    WSACleanup();
    std::cout << "Socket closed and Winsock cleaned up." << std::endl;
    return 0;
}