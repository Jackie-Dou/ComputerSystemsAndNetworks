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
    // инициализация библиотеки winsock2
    WSADATA wsd;
    //инициализирует библиотеку ws2_32.dll для приложения. версия windows socket,указатель на структуру
    WSAStartup(0X0101, &wsd);
    //инициализация сокета в случае сервера/клиента TCP. Интернет, тип сокета TCP, протокол
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
    sa.sin_port = htons(5150);

    if (bind(sckt, (sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        printf("bind error\n");
        closesocket(sckt);
        system("pause");
        return -1;
    }

    sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);

    // Connect on
    char buf[100] = { 0 };
    int bsize = recvfrom(sckt, buf, sizeof(buf), 0, (sockaddr*)&client_addr, &client_addr_size);
    if (bsize == SOCKET_ERROR)
        printf("packet is not received");
    else
        printf("%s\n", buf);

    char res[1000] = { 0 };
    while (true)//бесконечный цикл
    {
        bsize = recvfrom(sckt, buf, sizeof(buf), 0, (sockaddr*)&client_addr, &client_addr_size);
        if (bsize<=0)
        {
            printf("end\n");
            break;//выход из цикла
        }
        strcat(res, buf); 
        printf("receive: %s \n", buf);
    }
    shutdown(sckt, 2);
    int n = strlen(res);
    int* data = new int[n];
    srand(14);
    for (i = 0; i < n; i++)
    {
        data[i] = rand() % 26 + 97;
    }
    char ref[1000] = { 0 };
    char a[1] = { 0 };
    for (i = 0; i < n; i++)
    {
        a[0] = char(data[i]);
        strncat(ref, a, 1);
    }
    if (strcmp(res, res) != 0)
        printf("data corruption\n");
    else
        printf("no data corruption\n");
    WSACleanup();

}