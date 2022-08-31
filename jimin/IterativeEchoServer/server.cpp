#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")  // 의존 라이브러리에 "Ws2_32.lib"를 포함시키라고 링커한테 알려준다.

u_short constexpr PORT = 27015;
size_t constexpr DEFAULT_BUF_LEN = 1024;

enum class eClientStatus
{
    Normal,
    Closed,
    Error,
};

void HandleError(const char* msg);
void InitializeWinSock();
SOCKET CreateSocket();
SOCKADDR_IN SetServSocketAddr();
void BindSocket(SOCKET* sock, SOCKADDR_IN* addr);
void ListenSocket(SOCKET* sock);
SOCKET AcceptClient(SOCKET* sock);
eClientStatus HandleClient(SOCKET* client, char* buffer);
void ShutdownSocket(SOCKET* client, int option);

int main()
{
    InitializeWinSock();  // WS2_32 라이브러리 초기화

    SOCKET servSock = CreateSocket();  //  서버 소켓 생성 
    SOCKADDR_IN servAddr = SetServSocketAddr();  // 서버 소켓에 할당할 소켓 정보 구조체 생성
    BindSocket(&servSock, &servAddr);  // 소켓을 바인딩 상태로 만든다
    ListenSocket(&servSock);  // 소켓을 리스닝 상태로 만들고 클라이언트를 기다린다 

    // 다섯 번의 클라이언트 요청을 받은 뒤 종료한다. 
    for (int i = 0; i < 5; i++)
    {
        // 클라이언트가 연결 요청을 보내면 수락하고 클라이언트와 연결된 소켓을 생성한다. 
        SOCKET clientSock = AcceptClient(&servSock);

        char recvBuf[DEFAULT_BUF_LEN];  // 수신한 메세지를 넣어놓을 버퍼
        bool clientClosed = false;

        while (clientClosed == false)  // 클라이언트가 연결을 끊을 때까지 메세지를 수신한다 
        {
            eClientStatus status = HandleClient(&clientSock, recvBuf);

            switch (status)
            {
            case eClientStatus::Closed:
                std::cout << "-----Client Closed----" << std::endl;
                clientClosed = true;
                break;
            case eClientStatus::Error:
                std::cout << "Error on recv()" << std::endl;
                clientClosed = true;
                break;
            default:  // eClientStatus::Normal
                break;
            }
        }

        ShutdownSocket(&clientSock, SD_SEND);  // 연결 종료 (수신은 가능하도록 열어놓는다)
        closesocket(clientSock);  // 클라이언트 소켓 종료
    }

    closesocket(servSock);  // 서버 소켓 종료
    WSACleanup();  // WS2_32 라이브러리 해제 
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

SOCKADDR_IN SetServSocketAddr()
{
    SOCKADDR_IN addr = SOCKADDR_IN();  // SOCKADDR_IN 구조체 초기화

    addr.sin_family = AF_INET;  // IPv4 체계
    // 리틀 엔디안 방식의 서버 IP 주소 정수값 -> 빅 엔디안 방식의 정수값  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    return addr;
}

void BindSocket(SOCKET* sock, SOCKADDR_IN* addr)
{
    int result = bind(*sock, (sockaddr*)addr, sizeof(*addr));
    if (result == SOCKET_ERROR)
    {
        HandleError("bind()");
    }
}

void ListenSocket(SOCKET* sock)
{
    int result = listen(*sock, SOMAXCONN);  // 소켓과 최대 연결 대기열 크기 전달
    if (result == SOCKET_ERROR)
    {
        HandleError("listen()");
    }
}

SOCKET AcceptClient(SOCKET* sock)
{
    SOCKADDR_IN clientAddr = SOCKADDR_IN();  // 클라이언트 소켓 정보를 담을 구조체 생성 
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSock = accept(*sock, (sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSock == INVALID_SOCKET)
    {
        HandleError("accept()");
    }

    std::cout << "----Client Entered----" << std::endl;

    return clientSock;
}

eClientStatus HandleClient(SOCKET* client, char* buffer)
{
    // 클라이언트로부터 메세지 수신하면 버퍼에 넣어놓는다 
    int recvResult = recv(*client, buffer, DEFAULT_BUF_LEN, 0);

    if (recvResult > 0)  // 정상적으로 수신한 경우
    {
        // 클라이언트로부터 받은 메세지를 그대로 돌려준다 
        int sendResult = send(*client, buffer, recvResult, 0);
        if (sendResult == SOCKET_ERROR)
        {
            closesocket(*client);
            HandleError("send()");
        }

        buffer[recvResult] = 0;  // 문자열 끝은 항상 NULL값으로 채워준다.
        std::cout << "From client: " << buffer << std::endl;

        return eClientStatus::Normal;
    }
    else if (recvResult == 0)  // 클라이언트가 연결을 종료한 경우 
    {
        return eClientStatus::Closed;
    }
    else  // 에러가 난 경우 
    {
        return eClientStatus::Error;
    }
}

void ShutdownSocket(SOCKET* client, int option)
{
    int shutDownResult = shutdown(*client, option);

    if (shutDownResult == SOCKET_ERROR)
    {
        closesocket(*client);
        HandleError("shutdown()");
    }
}
