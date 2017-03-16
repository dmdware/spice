
#include "../platform.h"


#ifdef PLATFORM_WIN


// {B62C4E8D-62CC-404b-BBBF-BF3E3BBB1374}
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);

#define CXN_TEST_DATA_STRING              ("~!@#$%^&*()-_=+?<>1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
#define CXN_TRANSFER_DATA_LENGTH          (sizeof(CXN_TEST_DATA_STRING))


#define CXN_BDADDR_STR_LEN                17   // 6 two-digit hex values plus 5 colons
#define CXN_MAX_INQUIRY_RETRY             3
#define CXN_DELAY_NEXT_INQUIRY            15
#define CXN_SUCCESS                       0
#define CXN_ERROR                         1
#define CXN_DEFAULT_LISTEN_BACKLOG        4





//
// NameToBthAddr converts a bluetooth device name to a bluetooth address,
// if required by performing inquiry with remote name requests.
// This function demonstrates device inquiry, with optional LUP flags.
//
ULONG NameToBthAddr(_In_ const LPWSTR pszRemoteName, _Out_ PSOCKADDR_BTH pRemoteBtAddr)
{
    INT             iResult = CXN_SUCCESS;
    BOOL            bContinueLookup = FALSE, bRemoteDeviceFound = FALSE;
    ULONG           ulFlags = 0, ulPQSSize = sizeof(WSAQUERYSET);
    HANDLE          hLookup = NULL;
    PWSAQUERYSET    pWSAQuerySet = NULL;

    ZeroMemory(pRemoteBtAddr, sizeof(*pRemoteBtAddr));

    pWSAQuerySet = (PWSAQUERYSET) HeapAlloc(GetProcessHeap(),
                                            HEAP_ZERO_MEMORY,
                                            ulPQSSize);
    if ( NULL == pWSAQuerySet ) {
        iResult = STATUS_NO_MEMORY;
        wprintf(L"!ERROR! | Unable to allocate memory for WSAQUERYSET\n");
    }

    //
    // Search for the device with the correct name
    //
    if ( CXN_SUCCESS == iResult) {

        for ( INT iRetryCount = 0;
            !bRemoteDeviceFound && (iRetryCount < CXN_MAX_INQUIRY_RETRY);
            iRetryCount++ ) {
            //
            // WSALookupService is used for both service search and device inquiry
            // LUP_CONTAINERS is the flag which signals that we're doing a device inquiry.
            //
            ulFlags = LUP_CONTAINERS;

            //
            // Friendly device name (if available) will be returned in lpszServiceInstanceName
            //
            ulFlags |= LUP_RETURN_NAME;

            //
            // BTH_ADDR will be returned in lpcsaBuffer member of WSAQUERYSET
            //
            ulFlags |= LUP_RETURN_ADDR;

            if ( 0 == iRetryCount ) {
                wprintf(L"*INFO* | Inquiring device from cache...\n");
            } else {
                //
                // Flush the device cache for all inquiries, except for the first inquiry
                //
                // By setting LUP_FLUSHCACHE flag, we're asking the lookup service to do
                // a fresh lookup instead of pulling the information from device cache.
                //
                ulFlags |= LUP_FLUSHCACHE;

                //
                // Pause for some time before all the inquiries after the first inquiry
                //
                // Remote Name requests will arrive after device inquiry has
                // completed.  Without a window to receive IN_RANGE notifications,
                // we don't have a direct mechanism to determine when remote
                // name requests have completed.
                //
                wprintf(L"*INFO* | Unable to find device.  Waiting for %d seconds before re-inquiry...\n", CXN_DELAY_NEXT_INQUIRY);
                Sleep(CXN_DELAY_NEXT_INQUIRY * 1000);

                wprintf(L"*INFO* | Inquiring device ...\n");
            }

            //
            // Start the lookup service
            //
            iResult = CXN_SUCCESS;
            hLookup = 0;
            bContinueLookup = FALSE;
            ZeroMemory(pWSAQuerySet, ulPQSSize);
            pWSAQuerySet->dwNameSpace = NS_BTH;
            pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
            iResult = WSALookupServiceBegin(pWSAQuerySet, ulFlags, &hLookup);

            //
            // Even if we have an error, we want to continue until we
            // reach the CXN_MAX_INQUIRY_RETRY
            //
            if ( (NO_ERROR == iResult) && (NULL != hLookup) ) {
                bContinueLookup = TRUE;
            } else if ( 0 < iRetryCount ) {
                wprintf(L"=CRITICAL= | WSALookupServiceBegin() failed with error code %d, WSAGetLastError = %d\n", iResult, WSAGetLastError());
                break;
            }

            while ( bContinueLookup ) {
                //
                // Get information about next bluetooth device
                //
                // Note you may pass the same WSAQUERYSET from LookupBegin
                // as long as you don't need to modify any of the pointer
                // members of the structure, etc.
                //
                // ZeroMemory(pWSAQuerySet, ulPQSSize);
                // pWSAQuerySet->dwNameSpace = NS_BTH;
                // pWSAQuerySet->dwSize = sizeof(WSAQUERYSET);
                if ( NO_ERROR == WSALookupServiceNext(hLookup,
                                                      ulFlags,
                                                      &ulPQSSize,
                                                      pWSAQuerySet) ) {
                    
                    //
                    // Compare the name to see if this is the device we are looking for.
                    //
                    if ( ( pWSAQuerySet->lpszServiceInstanceName != NULL ) &&
                         ( CXN_SUCCESS == stricmp((const char*)pWSAQuerySet->lpszServiceInstanceName, (const char*)pszRemoteName) ) ) {
                        //
                        // Found a remote bluetooth device with matching name.
                        // Get the address of the device and exit the lookup.
                        //
                        CopyMemory(pRemoteBtAddr,
                                   (PSOCKADDR_BTH) pWSAQuerySet->lpcsaBuffer->RemoteAddr.lpSockaddr,
                                   sizeof(*pRemoteBtAddr));
                        bRemoteDeviceFound = TRUE;
                        bContinueLookup = FALSE;
                    }
                } else {
                    iResult = WSAGetLastError();
                    if ( WSA_E_NO_MORE == iResult ) { //No more data
                        //
                        // No more devices found.  Exit the lookup.
                        //
                        bContinueLookup = FALSE;
                    } else if ( WSAEFAULT == iResult ) {
                        //
                        // The buffer for QUERYSET was insufficient.
                        // In such case 3rd parameter "ulPQSSize" of function "WSALookupServiceNext()" receives
                        // the required size.  So we can use this parameter to reallocate memory for QUERYSET.
                        //
                        HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
                        pWSAQuerySet = (PWSAQUERYSET) HeapAlloc(GetProcessHeap(),
                                                                HEAP_ZERO_MEMORY,
                                                                ulPQSSize);
                        if ( NULL == pWSAQuerySet ) {
                            wprintf(L"!ERROR! | Unable to allocate memory for WSAQERYSET\n");
                            iResult = STATUS_NO_MEMORY;
                            bContinueLookup = FALSE;
                        }
                    } else {
                        wprintf(L"=CRITICAL= | WSALookupServiceNext() failed with error code %d\n", iResult);
                        bContinueLookup = FALSE;
                    }
                }
            }

            //
            // End the lookup service
            //
            WSALookupServiceEnd(hLookup);

            if ( STATUS_NO_MEMORY == iResult ) {
                break;
            }
        }
    }

    if ( NULL != pWSAQuerySet ) {
        HeapFree(GetProcessHeap(), 0, pWSAQuerySet);
        pWSAQuerySet = NULL;
    }

    if ( bRemoteDeviceFound ) {
        iResult = CXN_SUCCESS;
    } else {
        iResult = CXN_ERROR;
    }

    return iResult;
}






//
// RunClientMode runs the application in client mode.  It opens a socket, connects it to a
// remote socket, transfer some data over the connection and closes the connection.
//
ULONG RunClientMode(_In_ SOCKADDR_BTH RemoteAddr, _In_ int iMaxCxnCycles)
{
    ULONG           ulRetCode = CXN_SUCCESS;
    int             iCxnCount = 0;
    char         *pszData = NULL;
    SOCKET          LocalSocket = INVALID_SOCKET;
    SOCKADDR_BTH    SockAddrBthServer = RemoteAddr;
    HRESULT         res;

    pszData = (char *) HeapAlloc(GetProcessHeap(),
                                 HEAP_ZERO_MEMORY,
                                 CXN_TRANSFER_DATA_LENGTH);
    if ( NULL == pszData ) {
        ulRetCode = STATUS_NO_MEMORY;
        wprintf(L"=CRITICAL= | HeapAlloc failed | out of memory, gle = [%d] \n", GetLastError());
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        //
        // Setting address family to AF_BTH indicates winsock2 to use Bluetooth sockets
        // Port should be set to 0 if ServiceClassId is spesified.
        //
        SockAddrBthServer.addressFamily = AF_BTH;
        SockAddrBthServer.serviceClassId = g_guidServiceClass;
        SockAddrBthServer.port = 0;

        //
        // Create a static data-string, which will be transferred to the remote
        // Bluetooth device
        //
      //  res = StringCbCopyNA(pszData, CXN_TRANSFER_DATA_LENGTH, CXN_TEST_DATA_STRING, CXN_TRANSFER_DATA_LENGTH);
		res=0;
		strncpy(pszData, CXN_TEST_DATA_STRING, min(CXN_TRANSFER_DATA_LENGTH,CXN_TRANSFER_DATA_LENGTH));
        if ( FAILED(res) ) {
            printf("=CRITICAL= | Creating a static data string failed\n");
            ulRetCode = CXN_ERROR;
        }

    }

    if ( CXN_SUCCESS == ulRetCode ) {
    
        pszData[(CXN_TRANSFER_DATA_LENGTH/sizeof(wchar_t)) - 1] = 0;

        //
        // Run the connection/data-transfer for user specified number of cycles
        //
        for ( iCxnCount = 0;
            (0 == ulRetCode) && (iCxnCount < iMaxCxnCycles || iMaxCxnCycles == 0);
            iCxnCount++ ) {

            wprintf(L"\n");

            //
            // Open a bluetooth socket using RFCOMM protocol
            //
            LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
            if ( INVALID_SOCKET == LocalSocket ) {
                wprintf(L"=CRITICAL= | socket() call failed. WSAGetLastError = [%d]\n", WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            //
            // Connect the socket (pSocket) to a given remote socket represented by address (pServerAddr)
            //
            if ( SOCKET_ERROR == connect(LocalSocket,
                                         (struct sockaddr *) &SockAddrBthServer,
                                         sizeof(SOCKADDR_BTH)) ) {
                wprintf(L"=CRITICAL= | connect() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            //
            // send() call indicates winsock2 to send the given data
            // of a specified length over a given connection.
            //
            wprintf(L"*INFO* | Sending following data string:\n%s\n", pszData);
            if ( SOCKET_ERROR == send(LocalSocket,
                                      (char *)pszData,
                                      (int)CXN_TRANSFER_DATA_LENGTH,
                                      0) ) {
                wprintf(L"=CRITICAL= | send() call failed w/socket = [0x%I64X], szData = [%p], dataLen = [%I64u]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, pszData, (ULONG64)CXN_TRANSFER_DATA_LENGTH, WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            //
            // Close the socket
            //
            if ( SOCKET_ERROR == closesocket(LocalSocket) ) {
                wprintf(L"=CRITICAL= | closesocket() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }

            LocalSocket = INVALID_SOCKET;

        }
    }

    if ( INVALID_SOCKET != LocalSocket ) {
        closesocket(LocalSocket);
        LocalSocket = INVALID_SOCKET;
    }

    if ( NULL != pszData ) {
        HeapFree(GetProcessHeap(), 0, pszData);
        pszData = NULL;
    }

    return(ulRetCode);
}

//
// RunServerMode runs the application in server mode.  It opens a socket, connects it to a
// remote socket, transfer some data over the connection and closes the connection.
//


#define CXN_INSTANCE_STRING L"Sample Bluetooth Server"
ULONG RunServerMode(_In_ int iMaxCxnCycles, FILE* fp)
{
    ULONG           ulRetCode = CXN_SUCCESS;
    int             iAddrLen = sizeof(SOCKADDR_BTH);
    int             iCxnCount = 0;
    UINT            iLengthReceived = 0;
    UINT            uiTotalLengthReceived;
    size_t          cbInstanceNameSize = 0;
    char *          pszDataBuffer = NULL;
    char *          pszDataBufferIndex = NULL;
    char *       pszInstanceName = NULL;
    char         szThisComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD           dwLenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    SOCKET          LocalSocket = INVALID_SOCKET;
    SOCKET          ClientSocket = INVALID_SOCKET;
    WSAQUERYSET     wsaQuerySet = {0};
    SOCKADDR_BTH    SockAddrBthLocal = {0};
    LPCSADDR_INFO   lpCSAddrInfo = NULL;
    HRESULT         res;

    //
    // This fixed-size allocation can be on the stack assuming the
    // total doesn't cause a stack overflow (depends on your compiler settings)
    // However, they are shown here as dynamic to allow for easier expansion
    //
    lpCSAddrInfo = (LPCSADDR_INFO) HeapAlloc( GetProcessHeap(),
                                              HEAP_ZERO_MEMORY,
                                              sizeof(CSADDR_INFO) );
    if ( NULL == lpCSAddrInfo ) {
        wprintf(L"!ERROR! | Unable to allocate memory for CSADDR_INFO\n");
        ulRetCode = CXN_ERROR;
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        if ( !GetComputerName(szThisComputerName, &dwLenComputerName) ) {
            fprintf(fp, "=CRITICAL= | GetComputerName() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    //
    // Open a bluetooth socket using RFCOMM protocol
    //
    if ( CXN_SUCCESS == ulRetCode ) {
        LocalSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
        if ( INVALID_SOCKET == LocalSocket ) {
            fprintf(fp, "=CRITICAL= | socket() call failed. WSAGetLastError = [%d]\n", WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        //
        // Setting address family to AF_BTH indicates winsock2 to use Bluetooth port
        //
        SockAddrBthLocal.addressFamily = AF_BTH;
        SockAddrBthLocal.port = BT_PORT_ANY;

        //
        // bind() associates a local address and port combination
        // with the socket just created. This is most useful when
        // the application is a server that has a well-known port
        // that clients know about in advance.
        //
        if ( SOCKET_ERROR == bind(LocalSocket,
                                  (struct sockaddr *) &SockAddrBthLocal,
                                  sizeof(SOCKADDR_BTH) ) ) {
            fprintf(fp, "=CRITICAL= | bind() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        ulRetCode = getsockname(LocalSocket,
                                (struct sockaddr *)&SockAddrBthLocal,
                                &iAddrLen);
        if ( SOCKET_ERROR == ulRetCode ) {
            fprintf(fp, "=CRITICAL= | getsockname() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        //
        // CSADDR_INFO
        //
        lpCSAddrInfo[0].LocalAddr.iSockaddrLength = sizeof( SOCKADDR_BTH );
        lpCSAddrInfo[0].LocalAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
        lpCSAddrInfo[0].RemoteAddr.iSockaddrLength = sizeof( SOCKADDR_BTH );
        lpCSAddrInfo[0].RemoteAddr.lpSockaddr = (LPSOCKADDR)&SockAddrBthLocal;
        lpCSAddrInfo[0].iSocketType = SOCK_STREAM;
        lpCSAddrInfo[0].iProtocol = BTHPROTO_RFCOMM;

        //
        // If we got an address, go ahead and advertise it.
        //
        ZeroMemory(&wsaQuerySet, sizeof(WSAQUERYSET));
        wsaQuerySet.dwSize = sizeof(WSAQUERYSET);
        wsaQuerySet.lpServiceClassId = (LPGUID) &g_guidServiceClass;

        //
        // Adding a byte to the size to account for the space in the
        // format string in the swprintf call. This will have to change if converted
        // to UNICODE
        //
       // res = StringCchLength(szThisComputerName, sizeof(szThisComputerName), &cbInstanceNameSize);
        res = cbInstanceNameSize = strlen((const char*)szThisComputerName);
        if( FAILED(res) ) {
            fprintf(fp, "-FATAL- | ComputerName specified is too large\n");
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        cbInstanceNameSize += sizeof(CXN_INSTANCE_STRING) + 1;
        pszInstanceName = (char*)HeapAlloc(GetProcessHeap(),
                                           HEAP_ZERO_MEMORY,
                                           cbInstanceNameSize);
        if ( NULL == pszInstanceName ) {
           fprintf(fp, "-FATAL- | HeapAlloc failed | out of memory | gle = [%d] \n", GetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {
        //StringCbPrintf(pszInstanceName, cbInstanceNameSize, L"%s %s", szThisComputerName, CXN_INSTANCE_STRING);
        sprintf(pszInstanceName, "%s %s", szThisComputerName, CXN_INSTANCE_STRING);
        wsaQuerySet.lpszServiceInstanceName = pszInstanceName;
        wsaQuerySet.lpszComment = "Example Service instance registered in the directory service through RnR";
        wsaQuerySet.dwNameSpace = NS_BTH;
        wsaQuerySet.dwNumberOfCsAddrs = 1;      // Must be 1.
        wsaQuerySet.lpcsaBuffer = lpCSAddrInfo; // Req'd.

        //
        // As long as we use a blocking accept(), we will have a race
        // between advertising the service and actually being ready to
        // accept connections.  If we use non-blocking accept, advertise
        // the service after accept has been called.
        //
        if ( SOCKET_ERROR == WSASetService(&wsaQuerySet, RNRSERVICE_REGISTER, 0) ) {
            fprintf(fp, "=CRITICAL= | WSASetService() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    //
    // listen() call indicates winsock2 to listen on a given socket for any incoming connection.
    //
    if ( CXN_SUCCESS == ulRetCode ) {
        if ( SOCKET_ERROR == listen(LocalSocket, CXN_DEFAULT_LISTEN_BACKLOG) ) {
            fprintf(fp, "=CRITICAL= | listen() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
            ulRetCode = CXN_ERROR;
        }
    }

    if ( CXN_SUCCESS == ulRetCode ) {

        for ( iCxnCount = 0;
            (CXN_SUCCESS == ulRetCode) && ( (iCxnCount < iMaxCxnCycles) || (iMaxCxnCycles == 0) );
            iCxnCount++ ) {

            fprintf(fp, "\n");

            //
            // accept() call indicates winsock2 to wait for any
            // incoming connection request from a remote socket.
            // If there are already some connection requests on the queue,
            // then accept() extracts the first request and creates a new socket and
            // returns the handle to this newly created socket. This newly created
            // socket represents the actual connection that connects the two sockets.
            //
            ClientSocket = accept(LocalSocket, NULL, NULL);
            if ( INVALID_SOCKET == ClientSocket ) {
                wprintf(L"=CRITICAL= | accept() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
                ulRetCode = CXN_ERROR;
                break; // Break out of the for loop
            }

            //
            // Read data from the incoming stream
            //
            BOOL bContinue = TRUE;
            pszDataBuffer = (char *)HeapAlloc(GetProcessHeap(),
                                              HEAP_ZERO_MEMORY,
                                              CXN_TRANSFER_DATA_LENGTH);
            if ( NULL == pszDataBuffer ) {
                fprintf(fp, "-FATAL- | HeapAlloc failed | out of memory | gle = [%d] \n", GetLastError());
                ulRetCode = CXN_ERROR;
                break;
            }
            pszDataBufferIndex = pszDataBuffer;
            uiTotalLengthReceived = 0;
            while ( bContinue && (uiTotalLengthReceived < CXN_TRANSFER_DATA_LENGTH) ) {
                //
                // recv() call indicates winsock2 to receive data
                // of an expected length over a given connection.
                // recv() may not be able to get the entire length
                // of data at once.  In such case the return value,
                // which specifies the number of bytes received,
                // can be used to calculate how much more data is
                // pending and accordingly recv() can be called again.
                //
                iLengthReceived = recv(ClientSocket,
                                       (char *)pszDataBufferIndex,
                                       (CXN_TRANSFER_DATA_LENGTH - uiTotalLengthReceived),
                                       0);

                switch ( iLengthReceived ) {
                case 0: // socket connection has been closed gracefully
                    bContinue = FALSE;
                    break;

                case SOCKET_ERROR:
                    wprintf(L"=CRITICAL= | recv() call failed. WSAGetLastError=[%d]\n", WSAGetLastError());
                    bContinue = FALSE;
                    ulRetCode = CXN_ERROR;
                    break;

                default:

                    //
                    // Make sure we have enough room
                    //
                    if ( iLengthReceived > (CXN_TRANSFER_DATA_LENGTH - uiTotalLengthReceived)) {
                        wprintf(L"=CRITICAL= | received too much data\n");
                        bContinue = FALSE;
                        ulRetCode = CXN_ERROR;
                        break;
                    }

                    pszDataBufferIndex += iLengthReceived;
                    uiTotalLengthReceived += iLengthReceived;
                    break;
                }
            }

            if ( CXN_SUCCESS == ulRetCode ) {

                if ( CXN_TRANSFER_DATA_LENGTH != uiTotalLengthReceived ) {
                    wprintf(L"+WARNING+ | Data transfer aborted mid-stream. Expected Length = [%I64u], Actual Length = [%d]\n", (ULONG64)CXN_TRANSFER_DATA_LENGTH, uiTotalLengthReceived);
                }
                wprintf(L"*INFO* | Received following data string from remote device:\n%s\n", (wchar_t *)pszDataBuffer);

                //
                // Close the connection
                //
                if ( SOCKET_ERROR == closesocket(ClientSocket) ) {
                    wprintf(L"=CRITICAL= | closesocket() call failed w/socket = [0x%I64X]. WSAGetLastError=[%d]\n", (ULONG64)LocalSocket, WSAGetLastError());
                    ulRetCode = CXN_ERROR;
                } else {
                    //
                    // Make the connection invalid regardless
                    //
                    ClientSocket = INVALID_SOCKET;
                }
            }
        }
    }

    if ( INVALID_SOCKET != ClientSocket ) {
        closesocket(ClientSocket);
        ClientSocket = INVALID_SOCKET;
    }

    if ( INVALID_SOCKET != LocalSocket ) {
        closesocket(LocalSocket);
        LocalSocket = INVALID_SOCKET;
    }

    if ( NULL != lpCSAddrInfo ) {
        HeapFree(GetProcessHeap(), 0, lpCSAddrInfo);
        lpCSAddrInfo = NULL;
    }
    if ( NULL != pszInstanceName ) {
        HeapFree(GetProcessHeap(), 0, pszInstanceName);
        pszInstanceName = NULL;
    }

    if ( NULL != pszDataBuffer ) {
        HeapFree(GetProcessHeap(), 0, pszDataBuffer);
        pszDataBuffer = NULL;
    }

    return(ulRetCode);
}



#if 0
//
// ShowCmdLineSyntaxHelp displays the command line usage
//
void ShowCmdLineHelp(void)
{
    wprintf(
          L"\n  Bluetooth Connection Sample application for demonstrating connection and data transfer."
          L"\n"
          L"\n"
          L"\n  BTHCxn.exe  [-n<RemoteName> | -a<RemoteAddress>] "
          L"\n                  [-c<ConnectionCycles>]"
          L"\n"
          L"\n"
          L"\n  Switches applicable for Client mode:"
          L"\n    -n<RemoteName>        Specifies name of remote BlueTooth-Device."
          L"\n"
          L"\n    -a<RemoteAddress>     Specifies address of remote BlueTooth-Device."
          L"\n                          The address is in form XX:XX:XX:XX:XX:XX"
          L"\n                          where XX is a hexidecimal byte"
          L"\n"
          L"\n                          One of the above two switches is required for client."
          L"\n"
          L"\n"
          L"\n  Switches applicable for both Client and Server mode:"
          L"\n    -c<ConnectionCycles>  Specifies number of connection cycles."
          L"\n                          Default value for this parameter is 1.  Specify 0 to "
          L"\n                          run infinite number of connection cycles."
          L"\n"
          L"\n"
          L"\n"
          L"\n  Command Line Examples:"
          L"\n    \"BTHCxn.exe -c0\""
          L"\n    Runs the BTHCxn server for infinite connection cycles."
          L"\n    The application reports minimal information onto the cmd window."
          L"\n"
          L"\n    \"BTHCxn.exe -nServerDevice -c50\""
          L"\n    Runs the BTHCxn client connecting to remote device (having name "
          L"\n    \"ServerDevice\" for 50 connection cycles."
          L"\n    The application reports minimal information onto the cmd window."
          L"\n"
          );
}

#endif
#endif	//plat win

#ifdef PLATFORM_WIN
int32_t WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32_t nCmdShow)
#else
int main(int argc, char* argv[])
#endif
{
#if 0	//listing
	inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };

	printf("asdasd\r\n");

    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
	printf("as111dasd\r\n");
        exit(1);
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
	printf("asdas222d\r\n");
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");
	printf("asdas22122d\r\n");

    for (i = 0; i < num_rsp; i++) {
	printf("a3333sdasd\r\n");
        ba2str(&(ii+i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), 
            name, 0) < 0)
        strcpy(name, "[unknown]");
        printf("%s  %s\n", addr, name);
    }

	printf("asda444sd\r\n");
    free( ii );
    close( sock );
    return 0;
#endif

//rf s
#if 0

    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;
    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);
    memset(buf, 0, sizeof(buf));

    // read data from the client
    bytes_read = read(client, buf, sizeof(buf));
    if( bytes_read > 0 ) {
        printf("received [%s]\n", buf);
    }

    // close connection
    close(client);
    close(s);
    return 0;
#endif

//refcomm c
#if 0

    struct sockaddr_rc addr = { 0 };
    int s, status;
    char dest[18] = "D8:BB:2C:73:79:85";

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // send a message
    if( status == 0 ) {
        status = write(s, "hello!", 6);
    }

    if( status < 0 ) perror("uh oh");

    close(s);
    return 0;
#endif

#if 0
int dynamic_bind_rc(int sock, struct sockaddr_rc *sockaddr, uint8_t *port)
{
    int err;
    for( *port = 1; *port <= 31; *port++ ) {
        sockaddr->rc_channel = *port;
        err = bind(sock, (struct sockaddr *)sockaddr, sizeof(sockaddr));
        if( ! err || errno == EINVAL ) break;
    }
    if( port == 31 ) {
        err = -1;
        errno = EINVAL;
    }
    return err;
}
#endif

//l2cap s
#if 0
struct sockaddr_l2 loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int s, client, bytes_read;
    socklen_t opt = sizeof(rem_addr);

    // allocate socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // bind socket to port 0x1001 of the first available 
    // bluetooth adapter
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_bdaddr = *BDADDR_ANY;
    loc_addr.l2_psm = htobs(0x1001);

    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(s, 1);

    // accept one connection
    client = accept(s, (struct sockaddr *)&rem_addr, &opt);

    ba2str( &rem_addr.l2_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);

    memset(buf, 0, sizeof(buf));

    // read data from the client
    bytes_read = read(client, buf, sizeof(buf));
    if( bytes_read > 0 ) {
        printf("received [%s]\n", buf);
    }

    // close connection
    close(client);
    close(s);
#endif

//l2capp c
#if 0
struct sockaddr_l2 addr = { 0 };
    int s, status;
    char *message = "hello!";
    char dest[18] = "01:23:45:67:89:AB";

    if(argc < 2)
    {
        fprintf(stderr, "usage: %s <bt_addr>\n", argv[0]);
        exit(2);
    }

    strncpy(dest, argv[1], 18);

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // set the connection parameters (who to connect to)
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_psm = htobs(0x1001);
    str2ba( dest, &addr.l2_bdaddr );

    // connect to server
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    // send a message
    if( status == 0 ) {
        status = write(s, "hello!", 6);
    }

    if( status < 0 ) perror("uh oh");

    close(s);
#endif

#if 0
/* Occasionally, an application may need to adjust the maximum transmission unit (MTU) for an L2CAP connection and set it to something other than the default of 672 bytes. In BlueZ, this is done with the getsockopt and setsockopt functions. */

struct l2cap_options {
    uint16_t    omtu;
    uint16_t    imtu;
    uint16_t    flush_to;
    uint8_t     mode;
};

int set_l2cap_mtu( int sock, uint16_t mtu ) {
	struct l2cap_options opts;
    int optlen = sizeof(opts), err;
    err = getsockopt( s, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen );
    if( ! err ) {
        opts.omtu = opts.imtu = mtu;
        err = setsockopt( s, SOL_L2CAP, L2CAP_OPTIONS, &opts, optlen );
    }
    return err;
};


#endif


#if 0
int set_flush_timeout(bdaddr_t *ba, int timeout)
{
    int err = 0, dd;
    struct hci_conn_info_req *cr = 0;
    struct hci_request rq = { 0 };

    struct {
        uint16_t handle;
        uint16_t flush_timeout;
    } cmd_param;

    struct {
        uint8_t  status;
        uint16_t handle;
    } cmd_response;

    // find the connection handle to the specified bluetooth device
    cr = (struct hci_conn_info_req*) malloc(
            sizeof(struct hci_conn_info_req) + 
            sizeof(struct hci_conn_info));
    bacpy( &cr->bdaddr, ba );
    cr->type = ACL_LINK;
    dd = hci_open_dev( hci_get_route( &cr->bdaddr ) );
    if( dd < 0 ) {
        err = dd;
        goto cleanup;
    }
    err = ioctl(dd, HCIGETCONNINFO, (unsigned long) cr );
    if( err ) goto cleanup;

    // build a command packet to send to the bluetooth microcontroller
    cmd_param.handle = cr->conn_info->handle;
    cmd_param.flush_timeout = htobs(timeout);
    rq.ogf = OGF_HOST_CTL;
    rq.ocf = 0x28;
    rq.cparam = &cmd_param;
    rq.clen = sizeof(cmd_param);
    rq.rparam = &cmd_response;
    rq.rlen = sizeof(cmd_response);
    rq.event = EVT_CMD_COMPLETE;

    // send the command and wait for the response
    err = hci_send_req( dd, &rq, 0 );
    if( err ) goto cleanup;

    if( cmd_response.status ) {
        err = -1;
        errno = bt_error(cmd_response.status);
    }

cleanup:
    free(cr);
    if( dd >= 0) close(dd);
    return err;
}
#endif
#if 1
	FILE* fp = fopen("log.txt", "w");
	//ot,ob
	//msw recmspnv
	fprintf(fp, "asdasd\r\n");
	//fprintf(stdout, "asdasd");

	unsigned long ulRetCode = 0;

	//rtings

	//reps circdb

    WSADATA     WSAData = {0};
    //
    // Ask for Winsock version 2.2.
    //
    if ( CXN_SUCCESS == ulRetCode) {
        ulRetCode = WSAStartup(MAKEWORD(2, 2), &WSAData);
        if (CXN_SUCCESS != ulRetCode) {
            wprintf(L"-FATAL- | Unable to initialize Winsock version 2.2\n");
        }
    }

    if ( CXN_SUCCESS == ulRetCode) {
/*
        //
        // Note, this app "prefers" the name if provided, but it is app-specific
        // Other applications may provide more generic treatment.
        //
        if ( L'\0' != g_szRemoteName[0] ) {
            //
            // Get address from the name of the remote device and run the application
            // in client mode
            //
            ulRetCode = NameToBthAddr(g_szRemoteName, &RemoteBthAddr);
            if ( CXN_SUCCESS != ulRetCode ) {
                wprintf(L"-FATAL- | Unable to get address of the remote radio having name %s\n", g_szRemoteName);    
            }

            if ( CXN_SUCCESS == ulRetCode) {
                ulRetCode = RunClientMode(RemoteBthAddr, g_ulMaxCxnCycles);
            }
            
        } else if ( L'\0' != g_szRemoteAddr[0] ) {

            //
            // Get address from formated address-string of the remote device and
            // run the application in client mode
            //
            int iAddrLen = sizeof(RemoteBthAddr);
            ulRetCode = WSAStringToAddressW(g_szRemoteAddr,
                                               AF_BTH,
                                               NULL,
                                               (LPSOCKADDR)&RemoteBthAddr,
                                               &iAddrLen);
            if ( CXN_SUCCESS != ulRetCode ) {
                wprintf(L"-FATAL- | Unable to get address of the remote radio having formated address-string %s\n", g_szRemoteAddr);
            }

            if ( CXN_SUCCESS == ulRetCode ) {
                ulRetCode = RunClientMode(RemoteBthAddr, g_ulMaxCxnCycles);
            }

        } else*/ {
            //
            // No remote name/address specified.  Run the application in server mode
            //
            ulRetCode = RunServerMode(//g_ulMaxCxnCycles
				10000, fp);
        }
    }

	//RunServerMode(10000);

	fclose(fp);

    return(int)ulRetCode;

#endif
}
