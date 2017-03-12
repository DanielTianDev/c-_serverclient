//SERVER PROGRAM
//http://www.rohitab.com/discuss/topic/26991-cc-how-to-code-a-multi-client-server-in-c-using-threads/
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 6666



DWORD WINAPI receive_cmds(LPVOID lpParam) {
	DWORD thread_id = GetCurrentThreadId();
	printf("Thread created (id - %d)\n", thread_id);

	//set our socket to the socket passed in as a paramter

	SOCKET current_client = (SOCKET)lpParam;

	//buffer to hold received data
	char receiveBuf[DEFAULT_BUFLEN];
	//buffer to hold our sent data
	char sendBuf[DEFAULT_BUFLEN];
	//flag for error checking
	int iResult, iSendResult;

	//recv loop
	while (true) {
		iResult = recv(current_client, receiveBuf, sizeof(receiveBuf), 0); //recv cmds

		Sleep(10);

		if (iResult == 0) {
			//MessageBox(0, "error", "error; iResult is 0", MB_OK);
			printf("Connection (id - %d) closing...\n", thread_id);
			closesocket(current_client);
			ExitThread(0);
		}

		if (iResult > 0) {
			receiveBuf[iResult] = '\0';
			printf("Received Message (bytes: %d, id - %d): %s\n", iResult, thread_id, receiveBuf);

			if (strcmp(receiveBuf, "bye")== 0) { // close the socket associted with this client and end this thread
				printf("Connection (id - %d) closing...\n", thread_id);
				closesocket(current_client);
				//WSACleanup();
				ExitThread(0);
			}
			
			strcpy_s(sendBuf, "Message received");
			Sleep(5);
			iSendResult = send(current_client, sendBuf, sizeof(sendBuf), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("Send to client (id - %d) failed with error: %d\n", thread_id, WSAGetLastError());
				closesocket(current_client);
				ExitThread(0);
			}

		}
		else {
			printf("recv (id - %d) failed with error: %d. Note if the error is 10054, it means the connection was forcibly closed by the user\n", thread_id, WSAGetLastError());
			closesocket(current_client);
			ExitThread(0);
		}

		//clear buffers 
		strcpy_s(receiveBuf, "");
		strcpy_s(sendBuf, "");

	}

}


int __cdecl main(void)
{
	// our masterSocket(socket that listens for connections)
	SOCKET masterSocket = INVALID_SOCKET;
	// for our thread
	DWORD thread;
	WSADATA wsaData;
	sockaddr_in server;

	int iResult, iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	printf("Starting up multi-threaded UDP server by yours truly\n");

	// fill in winsock struct ... 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_PORT); // listen on telnet port 6666

	
	masterSocket = socket(AF_INET, SOCK_STREAM, 0); //create our socket

	if (masterSocket == INVALID_SOCKET) {
		printf("create socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	
	// bind our socket to a port(port 6666) 
	iResult = bind(masterSocket, (sockaddr*)&server, sizeof(server));

	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(masterSocket);
		WSACleanup();
		return 1;
	}

	// listen for a connection  
	iResult = listen(masterSocket, SOMAXCONN_HINT(3)); // If set to SOMAXCONN_HINT(N) (where N is a number), the backlog value will be N, adjusted to be within the range (200, 65535).
	if (iResult == SOCKET_ERROR) 
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(masterSocket);
		WSACleanup();
		return 1;
	}

	// socket that we snedzrecv data on
	SOCKET client;

	sockaddr_in from;

	int fromlen = sizeof(from);

	while (true) {

		//accept connections
		client = accept(masterSocket, (struct sockaddr*)&from, &fromlen);
		printf("Client connected\n");
		//create our recv_cmds thread and parse client socket as a parameter
		CreateThread(NULL, 0, receive_cmds, (LPVOID)client, 0, &thread);
	}

	// cleanup (shutdown winsock)
	closesocket(masterSocket);
	WSACleanup();
	system("pause"); return 0;
}



/*



void receive(int iResult, SOCKET ClientSocket, char * recvbuf, int recvbuflen) {
// Receive until the peer shuts down the connection
int iSendResult;
do {
iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
recvbuf[iResult] = '\0';
printf("value: %s\n\n", recvbuf);
if (iResult > 0) {
printf("Bytes received: %d\n", iResult);
//fwrite(buffer, buffer_length, 1, stdout);

// Echo the buffer back to the sender
iSendResult = send(ClientSocket, recvbuf, iResult, 0);
if (iSendResult == SOCKET_ERROR) {
printf("send failed with error: %d\n", WSAGetLastError());
closesocket(ClientSocket);
WSACleanup();
return;
}
printf("Bytes sent: %d\n", iSendResult);
}
else if (iResult == 0)
printf("Connection closing...\n");
else {
printf("recv failed with error: %d\n", WSAGetLastError());
closesocket(ClientSocket);
WSACleanup();
return;
}

} while (iResult > 0); //iResult > 0
}


int __cdecl main(void)
{
WSADATA wsaData;
int iResult;

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

struct addrinfo *result = NULL;
struct addrinfo hints;

int iSendResult;
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

// Initialize Winsock
iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
if (iResult != 0) {
printf("WSAStartup failed with error: %d\n", iResult);
return 1;
}

printf("Server Listening for client...\n");

ZeroMemory(&hints, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
hints.ai_flags = AI_PASSIVE;

// Resolve the server address and port
iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
if (iResult != 0) {
printf("getaddrinfo failed with error: %d\n", iResult);
WSACleanup();
return 1;
}


// Create a SOCKET for connecting to server
ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
if (ListenSocket == INVALID_SOCKET) {
printf("socket failed with error: %ld\n", WSAGetLastError());
freeaddrinfo(result);
WSACleanup();
return 1;
}

// Setup the TCP listening socket
iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
if (iResult == SOCKET_ERROR) {
printf("bind failed with error: %d\n", WSAGetLastError());
freeaddrinfo(result);
closesocket(ListenSocket);
WSACleanup();
return 1;
}

freeaddrinfo(result);

iResult = listen(ListenSocket, SOMAXCONN);
if (iResult == SOCKET_ERROR) {
printf("listen failed with error: %d\n", WSAGetLastError());
closesocket(ListenSocket);
WSACleanup();
return 1;
}

// Accept a client socket
ClientSocket = accept(ListenSocket, NULL, NULL);
printf("Accepted client socket (from listen socket)\n");
if (ClientSocket == INVALID_SOCKET) {
printf("accept failed with error: %d\n", WSAGetLastError());
closesocket(ListenSocket);
WSACleanup();
return 1;
}

// No longer need server socket
closesocket(ListenSocket);

std::thread first(receive, iResult, ClientSocket, recvbuf, recvbuflen);

first.join();





// shutdown the connection since we're done
iResult = shutdown(ClientSocket, SD_SEND);
if (iResult == SOCKET_ERROR) {
printf("shutdown failed with error: %d\n", WSAGetLastError());
closesocket(ClientSocket);
WSACleanup();
return 1;
}

// cleanup
closesocket(ClientSocket);
WSACleanup();

system("pause"); return 0;
}



*/