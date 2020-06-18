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
    SOCKET sckt = socket(AF_INET, SOCK_DGRAM, 0);
    if (sckt == INVALID_SOCKET)
    {
        printf("Error create socket\n");
        exit(1);
    }
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(5150);
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

    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    //dest_addr.sin_port=htons(5150);
    dest_addr.sin_port = htons(5150);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char hello[70] = "client connected to server...";
    int a = sendto(sckt, hello, sizeof(hello), 0, (sockaddr*)&dest_addr, sizeof(dest_addr));

    // Прием сообщения с сервера
    sockaddr_in server_addr;
    int server_addr_size = sizeof(server_addr);

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
        int chk = sendto(sckt, packet, sizeof(packet), 0, (sockaddr*)&dest_addr, sizeof(dest_addr));
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        if (chk==-1)
            printf("error of sending");
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        printf("the time: %f microseconds\n", time_span.count());
        tm += time_span.count();
    }
    int chk = sendto(sckt, "", 0, 0, (sockaddr*)&dest_addr, sizeof(dest_addr));
    printf("average time: %f microseconds\n", tm / num_packets);
    delete[] data;
    // размыкание соединения
    shutdown(sckt, 2);
    //заканчивает использование ws2_32.dll
    WSACleanup();
}