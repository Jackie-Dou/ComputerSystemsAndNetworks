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
    SOCKET sckt = socket(AF_INET, SOCK_DGRAM, 0);
    if (sckt == INVALID_SOCKET)
    {
        printf("socket error\n");
        WSACleanup();
        system("pause");
        return -1;
    }
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(7);

    if (bind(sckt, (sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        printf("bind error\n");
        closesocket(sckt);
        system("pause");
        return -1;
    }
    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    char buf[100] = { 0 };
    while (true)
    {
        int r = recvfrom(sckt, buf, sizeof(buf), 0, (sockaddr*)&client_addr, &client_addr_size);
        if (r == SOCKET_ERROR)
            printf("packet is not received");
        if (r <= 0)
        {
            printf("end\n");
            break;
        }
        printf("receive: %s \n", buf);
        int chk = sendto(sckt, buf, sizeof(buf), 0, (sockaddr*)&client_addr, sizeof(client_addr));
        if (chk == -1)
            printf("error of sending");
        printf("send:    %s \n", buf);
    }
    shutdown(sckt, 2);
    WSACleanup();
}