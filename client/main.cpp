#include <iostream>
#include <cassert>
#include <WinSock2.h>
#include <string>
#include <thread>

using namespace std;

class Client
{
private:
	string name;
    SOCKET clientSocket;    // 客户端套接字
    sockaddr_in serverAddr; // 服务器网络地址
    char readBuf[4096];          // 数据
    char writeBuf[4096];
    struct timeval timeout;
public:
    Client()
    {
        WORD sockVersion = MAKEWORD(2,2);
        WSADATA wsaData;
        if(WSAStartup(sockVersion, &wsaData)!=0)
        {
            cout << "initial WSA error!\n";
            assert(0);
        }

        // 创建客户端套接字(IPV4, TCP)
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == -1)
        {
            cout << "invalid socket!\n";
            getchar();
            assert(0);
        }

        // 服务器网络地址信息
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);
        serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

        // 连接 
        if (connect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            cout << "unable to connect to the server!\n";
            getchar();
            closesocket(clientSocket);
            assert(0);
        }
        
        cout << "             Client\n";
        cout << "***********************************\n";
        cout << "$ connect to the server successfully!\n";
        cout << "$ welcome to the chatroom!!!\n\n\n" ;

        // new thread send massage
        thread *sendMsgThread = new thread(&Client::sendMsg, this);
        
		while (1)
		{
			int rec = recv(clientSocket, readBuf, 255, 0);
	        if (rec > 0)
	        {
	            readBuf[rec] = 0;
	            cout << readBuf << endl;
	        }
		}
		delete sendMsgThread;
    }
    
    ~Client()
    {
        closesocket(clientSocket);
	    WSACleanup();
    }
    
    void sendMsg()
    {
    	while(1)
    	{
    		memset(writeBuf, 0, 4096);
    		cin.getline(writeBuf, 4096);
    		send(clientSocket, writeBuf, strlen(writeBuf)+1, 0);
		}
	}
};

int main()
{
    Client client;
    
    getchar();

    return 0;
}

