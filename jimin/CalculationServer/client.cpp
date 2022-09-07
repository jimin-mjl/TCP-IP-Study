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

// argc : Ŀ�ǵ� �������� ���� ������ ��
// argv : ���� ��� (argv[0]�� ������ Ŀ�ǵ�� �ް� ��)
int main(int argc, char* argv[])
{
    if (argc != 3)  // ���� ����� �޾Ҵ��� Ȯ��
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
    operationInfo[0] = opSize + '0';  // �ǿ������� ������ 1����Ʈ ���� ���·� �����Ѵ�. 

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
    WSADATA WSAData; // windows socket ������ ���� socket�� ���� ���� ���� ��� �ִ� ����ü

    int result = WSAStartup(MAKEWORD(2, 2), &WSAData);  // WS2_32 ���̺귯�� �ʱ�ȭ
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
    SOCKADDR_IN addr = {};  // SOCKADDR_IN ����ü �ʱ�ȭ (sin_zero = 0���� �ʱ�ȭ)

    addr.sin_family = AF_INET;  // IPv4 ü��
    addr.sin_addr.s_addr = inet_addr(ip);  // ���ڿ��� �򿣵�� ����� ������ ��ȯ�ؼ� ����    
    // InetPton(AF_INET, ip, &addr.sin_addr.s_addr);
    addr.sin_port = htons(atoi(port)); // port ��ȣ ���ڿ��� ������ ��ȯ�ؼ� ���� 

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
