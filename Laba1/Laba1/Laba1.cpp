#ifndef UNICODE
#define UNICODE
#endif
#pragma comment(lib, "mpr.lib")
#pragma comment(lib, "Iphlpapi.lib")

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <Iphlpapi.h>
#include <winnetwk.h>

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr);
void DisplayStruct(int i, LPNETRESOURCE lpnrLocal);
char* getMAC();

int main() {
    // lpNetResource - указатель на структуру NETRESOURCE. 
        //Если этот параметр равен нулю, то перечисление начнется с самой верхней ступени иерархии сетевых ресурсов. 
        //Нуль ставится для того, чтобы получить самый первый ресурс. 
        //После этого в качестве этого параметра передается указатель на уже найденный ресурс. 
         //Тогда перечисление начнется с найденного и продолжится дальше, пока не найдутся все ресурсы.
    LPNETRESOURCE lpnr = NULL;
       if (EnumerateFunc(lpnr) == FALSE) {
            printf("Call to EnumerateFunc failed\n");
            return 1;
        }

    char* pMac = getMAC();
    system("pause");
    free(pMac);
    return 0;
}

BOOL WINAPI EnumerateFunc(LPNETRESOURCE lpnr) {
    DWORD dwResult, dwResultEnum;  //32-битное беззнаковое целое
    HANDLE hEnum; //дескриптор
    DWORD cbBuffer = 16384;     // 16K is a good size
    DWORD cEntries = -1;        // enumerate all possible entries
    LPNETRESOURCE lpnrLocal;    // pointer to enumerated structures
    DWORD i;
    //
    // Call the WNetOpenEnum function to begin the enumeration.
    //The WNetOpenEnum function starts an enumeration of network resources or existing connections. You can continue the enumeration by calling the WNetEnumResource function.
    dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
        RESOURCETYPE_ANY,   // all resources
        0,  // enumerate all resources
        lpnr,       // NULL first time the function is called
        &hEnum);    // handle to the resource

    if (dwResult != NO_ERROR) {
        printf("WnetOpenEnum failed with error %d\n", dwResult);
        return FALSE;
    }

    //
    // Call the GlobalAlloc function to allocate resources.
    //Allocates the specified number of bytes from the heap. - выделение блока глобальной памяти 
    lpnrLocal = (LPNETRESOURCE)GlobalAlloc(GPTR, cbBuffer); //тип выделяемой памяти и размер - перемещаемый, иниц.нулями
    if (lpnrLocal == NULL) {
        printf("WnetOpenEnum failed with error %d\n", dwResult);
        //NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetOpenEnum");
        return FALSE;
    }

    do {
        //
        // Initialize the buffer.
        //
        ZeroMemory(lpnrLocal, cbBuffer); //обнуление памяти - с какого места сколько байт
        //
        // Call the WNetEnumResource function to continue
        //  the enumeration.
        //
        dwResultEnum = WNetEnumResource(hEnum,  // resource handle
            &cEntries,      // defined locally as -1
            lpnrLocal,      // LPNETRESOURCE
            &cbBuffer);     // buffer size
//
// If the call succeeds, loop through the structures.
//
        if (dwResultEnum == NO_ERROR) {
            for (i = 0; i < cEntries; i++) {
                // Call an application-defined function to
                //  display the contents of the NETRESOURCE structures.
                // Displays data from the specified structure or memory address as a list
                DisplayStruct(i, &lpnrLocal[i]);

                // If the NETRESOURCE structure represents a container resource, 
                //  call the EnumerateFunc function recursively.
                //все ресурсы контейнеры     Флаг dwUsage используется только при перечислении всех ресурсов
                if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
                    & RESOURCEUSAGE_CONTAINER))
                    //if(!EnumerateFunc(hwnd, hdc, &lpnrLocal[i]))
                    if (!EnumerateFunc(&lpnrLocal[i]))
                        printf("EnumerateFunc returned FALSE\n");
                //TextOut(hdc, 10, 10, "EnumerateFunc returned FALSE.", 29);
            }
        }
        // Process errors.
        //
        else if (dwResultEnum != ERROR_NO_MORE_ITEMS) {
            printf("WNetEnumResource failed with error %d\n", dwResultEnum);
            //NetErrorHandler(hwnd, dwResultEnum, (LPSTR)"WNetEnumResource");
            break;
        }
    } while (dwResultEnum != ERROR_NO_MORE_ITEMS);
    //
    // Call the GlobalFree function to free the memory.
    //
    GlobalFree((HGLOBAL)lpnrLocal);
    //
    // Call WNetCloseEnum to end the enumeration.
    //
    dwResult = WNetCloseEnum(hEnum);

    if (dwResult != NO_ERROR) {
        //
        // Process errors.
        //
        printf("WNetCloseEnum failed with error %d\n", dwResult);
        //NetErrorHandler(hwnd, dwResult, (LPSTR)"WNetCloseEnum");
        return FALSE;
    }

    return TRUE;
}

void DisplayStruct(int i, LPNETRESOURCE lpnrLocal) {
    printf("NETRESOURCE[%d] Scope: ", i);
    switch (lpnrLocal->dwScope) {
    case (RESOURCE_CONNECTED):
        printf("connected\n");
        break;
    case (RESOURCE_GLOBALNET):
        printf("all resources\n");
        break;
    case (RESOURCE_REMEMBERED):
        printf("remembered\n");
        break;
    default:
        printf("unknown scope %d\n", lpnrLocal->dwScope);
        break;
    }

    printf("NETRESOURCE[%d] Type: ", i);
    switch (lpnrLocal->dwType) {
    case (RESOURCETYPE_ANY):
        printf("any\n");
        break;
    case (RESOURCETYPE_DISK):
        printf("disk\n");
        break;
    case (RESOURCETYPE_PRINT):
        printf("print\n");
        break;
    default:
        printf("unknown type %d\n", lpnrLocal->dwType);
        break;
    }

    printf("NETRESOURCE[%d] DisplayType: ", i);
    switch (lpnrLocal->dwDisplayType) {
    case (RESOURCEDISPLAYTYPE_GENERIC):
        printf("generic\n");
        break;
    case (RESOURCEDISPLAYTYPE_DOMAIN):
        printf("domain\n");
        break;
    case (RESOURCEDISPLAYTYPE_SERVER):
        printf("server\n");
        break;
    case (RESOURCEDISPLAYTYPE_SHARE):
        printf("share\n");
        break;
    case (RESOURCEDISPLAYTYPE_FILE):
        printf("file\n");
        break;
    case (RESOURCEDISPLAYTYPE_GROUP):
        printf("group\n");
        break;
    case (RESOURCEDISPLAYTYPE_NETWORK):
        printf("network\n");
        break;
    default:
        printf("unknown display type %d\n", lpnrLocal->dwDisplayType);
        break;
    }

    printf("NETRESOURCE[%d] Usage: 0x%x = ", i, lpnrLocal->dwUsage);
    if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONNECTABLE)
        printf("connectable ");
    if (lpnrLocal->dwUsage & RESOURCEUSAGE_CONTAINER)
        printf("container ");
    printf("\n");

    printf("NETRESOURCE[%d] Localname: %S\n", i, lpnrLocal->lpLocalName);
    printf("NETRESOURCE[%d] Remotename: %S\n", i, lpnrLocal->lpRemoteName);
    printf("NETRESOURCE[%d] Comment: %S\n", i, lpnrLocal->lpComment);
    printf("NETRESOURCE[%d] Provider: %S\n", i, lpnrLocal->lpProvider);
    printf("\n");
}

char* getMAC() {
    //A pointer to a buffer that receives a linked list of IP_ADAPTER_INFO structures.
    //The IP_ADAPTER_INFO structure contains information about a particular network adapter on the local computer.
    PIP_ADAPTER_INFO AdapterInfo;
    DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
    char* mac_addr = (char*)malloc(18);

    AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (AdapterInfo == NULL)
    {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        free(mac_addr);
        return NULL; // it is safe to call free(NULL)
    }

    // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
    //The GetAdaptersInfo function retrieves adapter information for the local computer.
    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(AdapterInfo);
        AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
        if (AdapterInfo == NULL)
        {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            free(mac_addr);
            return NULL;
        }
    }

    if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
    {
        // Contains pointer to current adapter info
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        do
        {
            // technically should look at pAdapterInfo->AddressLength
            //   and not assume it is 6.
            sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
                pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
            printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
            // print them all, return the last one.
            // return mac_addr;

            printf("\n");
            pAdapterInfo = pAdapterInfo->Next;
        } while (pAdapterInfo);
    }
    free(AdapterInfo);
    return mac_addr; // caller must free.
}
