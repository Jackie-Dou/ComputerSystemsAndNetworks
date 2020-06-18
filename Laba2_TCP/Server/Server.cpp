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
    SOCKET sckt = socket(AF_INET, SOCK_STREAM, 0);
    if (sckt == INVALID_SOCKET)
    {
        printf("socket error\n");
        system("pause");
        return -1;
    }
    //структура для инициализация сервера
    struct sockaddr_in sa;
    //используемый домен
    sa.sin_family = AF_INET;
    //порт для прослушивания
    sa.sin_port = htons(1200);
    //адрес привязки порта
    sa.sin_addr.S_un.S_addr = INADDR_ANY;
    //закрепляем порт за приложением. прослушивающий сокет, порт и сетевой интерфейс из структуры, размер
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
    char res[1000] = { 0 };
    while (true)//бесконечный цикл
    {
        char buf[100] = { 0 };
        int r = recv(ns, buf, sizeof(buf), 0);//прием файла от клиента. Функция возвращает число считанных байтов 
        if (r <= 0)//если нет данных
        {
            printf("end\n");
            break;//выход из цикла
        }
        strcat(res, buf);
        //передача ответа после приема 
        printf("receive: %s \n", buf);
        send(ns, "OK", 3 * sizeof(char), 0);
        i++;
    }
    shutdown(ns, 2);
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