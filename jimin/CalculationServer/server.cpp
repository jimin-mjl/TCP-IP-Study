#include <iostream>
#include <queue>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")  // ���� ���̺귯���� "Ws2_32.lib"�� ���Խ�Ű��� ��Ŀ���� �˷��ش�.

u_short constexpr PORT = 27015;
size_t constexpr DEFAULT_BUF_SIZE = 1024;

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
int HandleClient(SOCKET* client, char* buffer);
int Calculate(int* nums, int size, char sign);
void ShutdownSocket(SOCKET* client, int option);

int main()
{
    InitializeWinSock();  // WS2_32 ���̺귯�� �ʱ�ȭ

    SOCKET servSock = CreateSocket();  //  ���� ���� ���� 
    SOCKADDR_IN servAddr = SetServSocketAddr();  // ���� ���Ͽ� �Ҵ��� ���� ���� ����ü ����
    BindSocket(&servSock, &servAddr);  // ������ ���ε� ���·� �����
    ListenSocket(&servSock);  // ������ ������ ���·� ����� Ŭ���̾�Ʈ�� ��ٸ��� 

    char buffer[DEFAULT_BUF_SIZE] = {};  // �Է� ���� ����

    // �ټ� ���� Ŭ���̾�Ʈ ��û�� ���� �� �����Ѵ�. 
    for (int i = 0; i < 5; i++)
    {
        // Ŭ���̾�Ʈ�� ���� ��û�� ������ �����ϰ� Ŭ���̾�Ʈ�� ����� ������ �����Ѵ�. 
        SOCKET clientSock = AcceptClient(&servSock);

        int result = HandleClient(&clientSock, buffer);

        if (result == 0)  // Ŭ���̾�Ʈ�� ������ ������ ���
        {
            std::cout << "-----Client Closed----" << std::endl;
        }
        else if (result < 0)  // ����
        {
            std::cout << "Error on recv()" << std::endl;
        }
        else
        {
            std::cout << "Result send to client." << std::endl;
        }

        ShutdownSocket(&clientSock, SD_SEND);  // ���� ���� (������ �����ϵ��� ������´�)
        closesocket(clientSock);  // Ŭ���̾�Ʈ ���� ����
    }

    closesocket(servSock);  // ���� ���� ����
    WSACleanup();  // WS2_32 ���̺귯�� ���� 
}

void HandleError(const char* fname)
{
    std::cout << "Error on " << fname << std::endl;
    WSACleanup();
    exit(1);
}

void InitializeWinSock()
{
    WSADATA WSAData; // windows socket ������ ���� socket�� ���� ���� ���� ��� �ִ� ����ü

    int result = WSAStartup(MAKEWORD(2, 2), &WSAData);  // WS2_32 ���̺귯�� �ʱ�ȭ
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
    SOCKADDR_IN addr = SOCKADDR_IN();  // SOCKADDR_IN ����ü �ʱ�ȭ

    addr.sin_family = AF_INET;  // IPv4 ü��
    // ��Ʋ ����� ����� ���� IP �ּ� ������ -> �� ����� ����� ������  
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
    int result = listen(*sock, SOMAXCONN);  // ���ϰ� �ִ� ���� ��⿭ ũ�� ����
    if (result == SOCKET_ERROR)
    {
        HandleError("listen()");
    }
}

SOCKET AcceptClient(SOCKET* sock)
{
    SOCKADDR_IN clientAddr = SOCKADDR_IN();  // Ŭ���̾�Ʈ ���� ������ ���� ����ü ���� 
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSock = accept(*sock, (sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSock == INVALID_SOCKET)
    {
        HandleError("accept()");
    }

    std::cout << "----Client Entered----" << std::endl;

    return clientSock;
}

int HandleClient(SOCKET* client, char* buffer)
{
    int opSize = 0;  // ���� �ǿ������� ����
    int result = recv(*client, reinterpret_cast<char*>(&opSize), sizeof(char), 0);
    if (result <= 0)
    {
        return result;
    }

    opSize -= '0';  // char�� ���� �ǿ������� ������ int�� ��ȯ�Ѵ�.
    int totalLen = opSize * sizeof(int) + 1;  // 1����Ʈ¥�� �����ڸ� ������ ���� �������� �� ũ��
    int recvLen = 0;
    while (recvLen < totalLen)  // ��ü �����͸� �� ������ ������ �Է� ���۸� �о���δ�.
    {
        result = recv(*client, &(buffer[recvLen]), totalLen, 0);
        recvLen += result;
    }

    char opSign = buffer[totalLen - 1];  // �� �������� �����ڸ� 1����Ʈ ������ �޴´�.
    int opResult = Calculate(reinterpret_cast<int*>(buffer), opSize, opSign);

    // Ŭ���̾�Ʈ���� ����� �����Ѵ�.  
    int sendResult = send(*client, reinterpret_cast<const char*>(&opResult), sizeof(opResult), 0);
    if (sendResult == SOCKET_ERROR)
    {
        closesocket(*client);
        HandleError("send()");
    }

    return sendResult;
}

int Calculate(int* nums, int size, char sign)
{
    int operationResult = nums[0];

    switch (sign)
    {
    case '+':
        for (int i = 1; i < size; i++)
        {
            operationResult += nums[i];
        }
        break;
    case '-':
        for (int i = 1; i < size; i++)
        {
            operationResult -= nums[i];
        }
        break;
    case '*':
        for (int i = 1; i < size; i++)
        {
            operationResult *= nums[i];
        }
        break;
    }

    return operationResult;
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
