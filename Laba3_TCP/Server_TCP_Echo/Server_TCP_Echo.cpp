#pragma comment(lib, "Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <iostream>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    int i = 0;
    WSADATA wsd;
    WSAStartup(0X0101, &wsd);
    SOCKET sckt = socket(AF_INET, SOCK_STREAM, 0);
    if (sckt == INVALID_SOCKET)
    {
        printf("socket error\n");
        system("pause");
        return -1;
    }
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(7);
    sa.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(sckt, (SOCKADDR*)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        printf("bind error\n");
        system("pause");
        return -1;
    }
    if (listen(sckt, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen error\n");
        system("pause");
        return -1;
    }
    printf("wait for client...\n");
    SOCKET ns;
    SOCKADDR_IN nsa;
    int sizeof_nsa = sizeof(nsa);
    ns = accept(sckt, (SOCKADDR*)&nsa, &sizeof_nsa);
    if (ns == INVALID_SOCKET)
    {
        printf("accept error\n");
        system("pause");
        return -1;
    }
    printf("connected\n");
    while (true)
    {
        char buf[100] = { 0 };
        int r = recv(ns, buf, sizeof(buf), 0);
        if (r == SOCKET_ERROR)
            printf("packet is not received");
        if (r <= 0)
        {
            printf("end\n");
            break;
        }
        printf("receive: %s \n", buf);
        int chk = send(ns, buf, sizeof(buf), 0);
        if (chk == -1)
            printf("error of sending");
        printf("send:    %s \n", buf);
    }
    shutdown(ns, 2);
    shutdown(sckt, 2);
    WSACleanup();
}