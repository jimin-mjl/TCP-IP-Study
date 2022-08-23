#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

size_t constexpr DEFAULT_BUF_LEN = 1024;

void HandleError(const char* msg);
void InitializeWinSock();
SOCKET CreateSocket();
SOCKADDR_IN SetSocketAddr(const char* ip, const char* port);
void ConnectSocket(SOCKET* sock, SOCKADDR_IN* addr);
int SendMsg(SOCKET* sock, const char* msg);
int RecvMsg(SOCKET* sock, char* buffer);

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

    char recvBuf[DEFAULT_BUF_LEN];
    int sendSize = SendMsg(&sock, "Hello World!");
    int recvSize = RecvMsg(&sock, recvBuf);

    if (sendSize == recvSize)
    {
        std::cout << "Echoed!" << std::endl;
    }
    else
    {
        std::cout << "There is an error..." << std::endl;
    }
}

void HandleError(const char* fname)
{
    std::cout << "Error on " << fname << std::endl;
    WSACleanup();
    exit(1);
}

void InitializeWinSock()
{
    WSADATA WSAData; // windows socket 구현을 위해 socket의 버전 정보 등을 담고 있는 구조체

    int result = WSAStartup(MAKEWORD(2, 2), &WSAData);  // WS2_32 라이브러리 초기화
    if (result != 0)
    {
        std::cout << "Error on Initializing WS2_32.dll" << std::endl;
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
    SOCKADDR_IN addr = SOCKADDR_IN();  // SOCKADDR_IN 구조체 초기화

    addr.sin_family = AF_INET;  // IPv4 체계
    addr.sin_addr.s_addr = inet_addr(ip);  // 문자열을 IPv4 주소로 변환해서 전달    
    addr.sin_port = htons(atoi(port)); // port 번호 문자열을 정수로 변환해서 전달 

    return addr;
}

void ConnectSocket(SOCKET* sock, SOCKADDR_IN* addr)
{
    int result = connect(*sock, addr, sizeof(*addr));

    if (result == SOCKET_ERROR)
    {
        closesocket(&sock);
        HandleError("connect()");
    }
}

int SendMsg(SOCKET* sock, const char* msg)
{
    int result = send(*sock, msg, sizeof(msg), 0);
    
    if (result == SOCKET_ERROR)
    {
        closesocket(*sock);
        HandleError("send()");
    }

    return result;
}

int RecvMsg(SOCKET* sock, char* buffer)
{
    int result = recv(*sock, buffer, DEFAULT_BUF_LEN, 0);

    if (result == SOCKET_ERROR)
    {
        closesocket(*sock);
        HandleError("recv()");
    }

    return result;
}
