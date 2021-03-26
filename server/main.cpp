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
    SOCKET sListen;         // �����׽���
    sockaddr_in serverAddr; // �����������ַ
    char buf[4096];          // ��������
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

        // ���������׽���(IPV4, TCP)
        sListen = socket(AF_INET, SOCK_STREAM, 0);
        if (sListen == -1)
        {
            cout << "set up Socket error!\n";
            assert(0);
        }

        // ��IP��˿�
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);
        serverAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
        if (bind(sListen, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            cout << "bind error!\n";
            closesocket(sListen);
            assert(0);
        }

        // ���� 5��ʾ�Ŷӵȴ������ֵ
        if (listen(sListen, 6) == SOCKET_ERROR)
        {
            cout << "listen error!\n";
            closesocket(sListen);
            assert(0);
        }
		cout << "           Server\n";
        cout << "****************************\n";
        cout << "waiting for being connected!\n";

        fd_set socketSet;    // ������׽��������пͻ����׽���
        FD_ZERO(&socketSet); // ���
        FD_SET(sListen, &socketSet);

        fd_set readSet;
        FD_ZERO(&readSet);

        fd_set writeSet;
        FD_ZERO(&readSet);

        map<int, sockaddr_in> addrs; // ʹ��map�洢socket��socket_addr�ļ�ֵ�� 
        

        while (1)
        {
            readSet = socketSet;
            writeSet = socketSet;
            
            Sleep(30);
            int nRetAll = select(0, &readSet, &writeSet, NULL, NULL); // ����׽��ֵĿɶ���д��
            

            if (nRetAll > 0)
            {
                // �����׽��ֿɶ�(����������)
                if (FD_ISSET(sListen, &readSet))
                {
                    if (socketSet.fd_count < FD_SETSIZE)
                    {
                        sockaddr_in addrRemote;
                        int addrLen = sizeof(addrRemote);
                        SOCKET sClient = accept(sListen, (sockaddr *)&addrRemote, &addrLen);
                        if (sClient != INVALID_SOCKET)
                        {
                            FD_SET(sClient, &socketSet); // ����socket����socketSet
                            // ȷ����socket��socketSet�е��±�
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

                // ��ѯsocketSet�е�ÿ���׽���
                for(int i = 0; i < socketSet.fd_count; i++)
                {
                    // �Ƿ�߿ɶ���
                    if(FD_ISSET(socketSet.fd_array[i], &readSet) )
                    {  
                        //����recv���������� flag = 0 ��������
                        memset(buf, 0 , 4096);
                        int sender = i;
                        int nRecv = recv(socketSet.fd_array[i], buf, 4096, 0);
                        if(nRecv > 0)
                        {
                            buf[nRecv] = 0;
                            cout << "user" << i << '(' << inet_ntoa(addrs[socketSet.fd_array[i]].sin_addr) << "):" << buf << endl;

                            // �㲥
                        for(int i = 0; i < socketSet.fd_count; i++)
                        {
                            if (socketSet.fd_array[i] == sListen || i == sender)
                            {
                                continue;
                            }
                            char writeBuf[4096];
                            strcpy(writeBuf, inet_ntoa(addrs[socketSet.fd_array[i]].sin_addr));
                            strcpy(writeBuf + strlen(writeBuf), "��");
                            strcpy(writeBuf + strlen(writeBuf), buf);
                            int nRet = send(socketSet.fd_array[i], writeBuf, strlen(writeBuf)+1, 0);
                            if(nRet <= 0)  
                            {
                                // ͨ��send����ж��������
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
