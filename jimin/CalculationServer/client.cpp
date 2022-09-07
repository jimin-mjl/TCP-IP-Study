#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

size_t constexpr DEFAULT_BUF_LEN = 1024;

void HandleError(const char* msg);
void InitializeWinSock();
SOCKET CreateSocket();
SOCKADDR_IN SetSocketAddr(const char* ip, const char* port);
void ConnectSocket(SOCKET* sock, SOCKADDR_IN* addr);
int SendMsg(SOCKET* sock, const char* msg, int size);
int RecvMsg(SOCKET* sock, int* buffer);

// argc : 커맨드 라인으로 받을 인자의 수
// argv : 인자 목록 (argv[0]은 실행한 커맨드로 받게 됨)
int main(int argc, char* argv[])
{
    if (argc != 3)  // 인자 제대로 받았는지 확인
    {
        std::cout << "Invalid arguments passed" << std::endl;
        return 0;
    }

    InitializeWinSock();

    SOCKET sock = CreateSocket();
    SOCKADDR_IN addr = SetSocketAddr(argv[1], argv[2]);
    ConnectSocket(&sock, &addr);

    int opSize;
    cout << "Operand count: ";
    cin >> opSize;

    char operationInfo[DEFAULT_BUF_LEN] = {};
    operationInfo[0] = opSize + '0';  // 피연산자의 개수를 1바이트 정수 형태로 저장한다. 

    for (int i = 0; i < opSize; i++)
    {
        int num;
        cout << "Operand " << i + 1 << ": ";
        cin >> num;
        operationInfo[i * sizeof(int) + 1] = num;
    }

    char opSign;
    cout << "Operator: ";
    cin >> opSign;
    operationInfo[opSize * sizeof(int) + 1] = opSign;

    int totalSize = opSize * sizeof(int) + 2;
    SendMsg(&sock, operationInfo, totalSize);

    int result = 0;
    RecvMsg(&sock, &result);

    cout << "Operation result: " << result << endl;

    closesocket(sock);
    WSACleanup();
}

void HandleError(const char* fname)
{
    cout << "Error on " << fname << endl;
    WSACleanup();
    exit(1);
}

void InitializeWinSock()
{
    WSADATA WSAData; // windows socket 구현을 위해 socket의 버전 정보 등을 담고 있는 구조체

    int result = WSAStartup(MAKEWORD(2, 2), &WSAData);  // WS2_32 라이브러리 초기화
    if (result != 0)
    {
        cout << "Error on Initializing WS2_32.dll" << endl;
        exit(1);
    }
}

SOCKET CreateSocket()
{
    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        HandleError("socket()");
    }
    return sock;
}

SOCKADDR_IN SetSocketAddr(const char* ip, const char* port)
{
    SOCKADDR_IN addr = {};  // SOCKADDR_IN 구조체 초기화 (sin_zero = 0으로 초기화)

    addr.sin_family = AF_INET;  // IPv4 체계
    addr.sin_addr.s_addr = inet_addr(ip);  // 문자열을 빅엔디안 방식의 정수로 변환해서 전달    
    // InetPton(AF_INET, ip, &addr.sin_addr.s_addr);
    addr.sin_port = htons(atoi(port)); // port 번호 문자열을 정수로 변환해서 전달 

    return addr;
}

void ConnectSocket(SOCKET* sock, SOCKADDR_IN* addr)
{
    int result = connect(*sock, (sockaddr*)addr, sizeof(*addr));

    if (result == SOCKET_ERROR)
    {
        closesocket(*sock);
        HandleError("connect()");
    }
}

int SendMsg(SOCKET* sock, const char* msg, int size)
{
    int result = send(*sock, msg, size, 0);

    if (result == SOCKET_ERROR)
    {
        closesocket(*sock);
        HandleError("send()");
    }

    return result;
}

int RecvMsg(SOCKET* sock, int* buffer)
{
    int result = recv(*sock, reinterpret_cast<char*>(buffer), sizeof(*buffer), 0);

    if (result == SOCKET_ERROR)
    {
        closesocket(*sock);
        HandleError("recv()");
    }

    return result;
}
