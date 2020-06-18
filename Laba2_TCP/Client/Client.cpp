#pragma comment(lib, "Ws2_32.lib")
#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <iostream>
#include <sys/stat.h>
#include <time.h>
#include <chrono>

using namespace std::chrono;

int main(int argc, char* argv[])
{
    // инициализация библиотеки winsock2
    WSADATA wsd;
    //инициализирует библиотеку для приложения
    if (WSAStartup(0X0101, &wsd))
    {
        printf("winsock is not initialized\n");
        WSACleanup();
    }
    //инициализация сокета в случае сервера/клиента TCP(интернет-приложение, потоковый, TCP)
    SOCKET sckt = socket(AF_INET, SOCK_STREAM, 0);
    if (sckt == INVALID_SOCKET)
    {
        printf("Error create socket\n");
        exit(1);
    }
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1200);
    sa.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int i, j, n;
    printf("Enter length of data: ");
    scanf_s("%d", &n);
    int* data = new int[n];
    srand(14);
    for (i = 0; i < n; i++)
    {
        data[i] = rand() % 26 + 97;
    }
    printf("wait for server...\n");
    while (true)
    {
        int connect_res = connect(sckt, (SOCKADDR*)&sa, sizeof(sa));
        if (connect_res == 0)
            break;
        Sleep(250);
    }
    printf("connected\n");
    i = 0;
    char buf[100] = { 0 };
    char packet[11] = { 0 };
    int num_packets = 0;

    double tm = 0;


    while (i < n)
    {
        memset(buf, 0, strlen(buf));
        memset(packet, 0, 11);
        for (j = 0; j < 10; j++)
        {
            if (i < n)
            {
                packet[j] = char(data[i]);
                i++;
            }
        }
        num_packets++;
        printf("packet: %s       |       ", packet);
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        int chk = send(sckt, packet, strlen(packet), 0);
        int y = recv(sckt, buf, sizeof(buf), 0);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        printf("the time: %f microseconds\n", time_span.count());
        tm += time_span.count();

        if (y == -1)
        {
            printf("no answer\n");
        }
        else
            printf("  answer: %s\n", buf);
    }
    printf("average time: %f microseconds\n", tm / num_packets);
    delete[] data;
    // размыкание соединения
    shutdown(sckt, 2);
    //заканчивает использование ws2_32.dll
    WSACleanup();
}