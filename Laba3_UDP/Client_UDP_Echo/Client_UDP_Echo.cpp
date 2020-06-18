#pragma comment(lib, "Ws2_32.lib")
#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <iostream>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    WSADATA wsd;
    if (WSAStartup(0X0101, &wsd))
    {
        printf("winsock is not initialized\n");
        WSACleanup();
    }
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
    dest_addr.sin_port = htons(7);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sockaddr_in server_addr;
    int server_addr_size = sizeof(server_addr);

    i = 0;
    char buf[100] = { 0 };
    char packet[11] = { 0 };
    int num_packets = 0;

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
        printf("packet: %s\n", packet);
        int chk = sendto(sckt, packet, sizeof(packet), 0, (sockaddr*)&dest_addr, sizeof(dest_addr));
        if (chk == -1)
            printf("error of sending");
        int y = recvfrom(sckt, buf, sizeof(buf), 0, (sockaddr*)&server_addr, &server_addr_size);
        printf("answer: %s\n", buf);
    }
    sendto(sckt, "", 0, 0, (sockaddr*)&dest_addr, sizeof(dest_addr));
    delete[] data;
    shutdown(sckt, 2);
    WSACleanup();
}