#include <iostream>
#include <cstdlib>
#include <map>
#include <WinSock2.h>
#include <windows.h>
#include <cassert>

using namespace std;

class Server
{
private:
    SOCKET sListen;         // 监听套接字
    sockaddr_in serverAddr; // 服务器网络地址
    char buf[4096];          // 接受数据
public:
    Server()
    {
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        if (WSAStartup(sockVersion, &wsaData) != 0)
        {
            cout << "initial WSA error.\n";
            assert(0);
        }

        // 创建监听套接字(IPV4, TCP)
        sListen = socket(AF_INET, SOCK_STREAM, 0);
        if (sListen == -1)
        {
            cout << "set up Socket error!\n";
            assert(0);
        }

        // 绑定IP与端口
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);
        serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
        if (bind(sListen, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            cout << "bind error!\n";
            closesocket(sListen);
            assert(0);
        }

        // 监听 5表示排队等待的最大值
        if (listen(sListen, 6) == SOCKET_ERROR)
        {
            cout << "listen error!\n";
            closesocket(sListen);
            assert(0);
        }
		cout << "           Server\n";
        cout << "****************************\n";
        cout << "waiting for being connected!\n";

        fd_set socketSet;    // 存监听套接字与所有客户端套接字
        FD_ZERO(&socketSet); // 清空
        FD_SET(sListen, &socketSet);

        fd_set readSet;
        FD_ZERO(&readSet);

        fd_set writeSet;
        FD_ZERO(&readSet);

        map<int, sockaddr_in> addrs; // 使用map存储socket与socket_addr的键值对 
        

        while (1)
        {
            readSet = socketSet;
            writeSet = socketSet;
            
            Sleep(30);
            int nRetAll = select(0, &readSet, &writeSet, NULL, NULL); // 检查套接字的可读可写性
            

            if (nRetAll > 0)
            {
                // 监听套接字可读(有连接请求)
                if (FD_ISSET(sListen, &readSet))
                {
                    if (socketSet.fd_count < FD_SETSIZE)
                    {
                        sockaddr_in addrRemote;
                        int addrLen = sizeof(addrRemote);
                        SOCKET sClient = accept(sListen, (sockaddr *)&addrRemote, &addrLen);
                        if (sClient != INVALID_SOCKET)
                        {
                            FD_SET(sClient, &socketSet); // 将该socket加入socketSet
                            // 确定该socket在socketSet中的下标
                            for(int i = 0; i < socketSet.fd_count; i++)
                            {
                                if (FD_ISSET(socketSet.fd_array[i], &readSet))
                                {
                                    addrs[sClient] = addrRemote;
                                    break;
                                }
                            }
                            cout << "connect to " << inet_ntoa(addrRemote.sin_addr) << endl;
                        }
                    }
                    else
                    {
                        cout << "the num of connection is out of range!\n";
                        continue;
                    }
                }

                // 轮询socketSet中的每个套接字
                for(int i = 0; i < socketSet.fd_count; i++)
                {
                    // 是否具可读性
                    if(FD_ISSET(socketSet.fd_array[i], &readSet) )
                    {  
                        //调用recv，接收数据 flag = 0 阻塞接受
                        memset(buf, 0 , 4096);
                        int sender = i;
                        int nRecv = recv(socketSet.fd_array[i], buf, 4096, 0);
                        if(nRecv > 0)
                        {
                            buf[nRecv] = 0;
                            cout << "user" << i << '(' << inet_ntoa(addrs[socketSet.fd_array[i]].sin_addr) << "):" << buf << endl;

                            // 广播
                        for(int i = 0; i < socketSet.fd_count; i++)
                        {
                            if (socketSet.fd_array[i] == sListen || i == sender)
                            {
                                continue;
                            }
                            char writeBuf[4096];
                            strcpy(writeBuf, inet_ntoa(addrs[socketSet.fd_array[i]].sin_addr));
                            strcpy(writeBuf + strlen(writeBuf), "：");
                            strcpy(writeBuf + strlen(writeBuf), buf);
                            int nRet = send(socketSet.fd_array[i], writeBuf, strlen(writeBuf)+1, 0);
                            if(nRet <= 0)  
                            {
                                // 通过send结果判断连接情况
                                if(GetLastError() == WSAEWOULDBLOCK)  
                                {  
                                    //do nothing  
                                }  
                                else  
                                {  
                                    cout << inet_ntoa(addrs[socketSet.fd_array[i]].sin_addr) << " has disconnected!\n";
                                    closesocket(socketSet.fd_array[i]);   
                                    FD_CLR(socketSet.fd_array[i],&socketSet);  
                                }   
                            }
                        }
                        }
                    } 
                }
            }
        }
    }

    ~Server()
    {
        closesocket(sListen);
        WSACleanup();
    }
};

int main()
{
    Server server;

    return 0;
}
