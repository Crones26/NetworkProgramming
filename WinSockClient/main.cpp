#define _CRT_SECURE_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<iphlpapi.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
using namespace std;
using std::cin;
using std::cout;
using std::endl;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT			"27015"
#define DEFAULT_BUFFER_LENGTH	1500
#define SZ_SORRY	"Sorry, but all is busy"

BOOL g_connected = TRUE;


VOID Send(SOCKET connect_socket, addrinfo* result);
VOID Recieve(SOCKET connect_socket);

//#define ALL_IN_ONE
void main()
{
	setlocale(LC_ALL, "");
	WSAData wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//2) Выполняем разрешение имен:
	addrinfo* result = NULL;
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddreinfo() failed with code: " << iResult << endl;
		WSACleanup();
		return;
	}

	//3) Создаем сокет для подключения к серверу:
	SOCKET connect_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		cout << "Socket creation failed with code: " << WSAGetLastError() << endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//4) Подключаемся к серверу:
	iResult = connect(connect_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to connect to Server" << endl;
		closesocket(connect_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//5) Получение и отправка данных:
	//int recvbuflen = DEFAULT_BUFFER_LENGTH;
#ifdef ALL_IN_ONE
	CHAR send_buffer[DEFAULT_BUFFER_LENGTH] = "Hello Server, I am Client";
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH]{};

	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send data failed with " << WSAGetLastError() << endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		cout << iResult << " Bytes sent" << endl;

		//6) Receive data:

		iResult = recv(connect_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)cout << "Bytes received: " << iResult << ", Message: " << recvbuffer << endl;
		else if (iResult == 0)cout << "Connection closed" << endl;
		else cout << "Recieve failed with code: " << WSAGetLastError() << endl;
		/////////////////////////////////////////////
		if (strcmp(recvbuffer, SZ_SORRY) == 0)break;
		/////////////////////////////////////////////
		ZeroMemory(send_buffer, sizeof(send_buffer));
		ZeroMemory(recvbuffer, sizeof(recvbuffer));

		cout << "Введите сообщение: ";
		SetConsoleCP(1251);
		cin.getline(send_buffer, DEFAULT_BUFFER_LENGTH);
		SetConsoleCP(866);
		//for (int i = 0; send_buffer[i]; i++)send_buffer[i] = tolower(send_buffer[i]);
	} while (iResult > 0 && strcmp(send_buffer, "exit"));
#endif // ALL_IN_ONE
	DWORD dwRecvThreadID = 0;
	HANDLE recvHandle = CreateThread
	(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Recieve,
		(LPVOID)connect_socket,
		0,
		&dwRecvThreadID
	);

	Send(connect_socket, result);
	g_connected = FALSE;
	//TODO:Sync Threads


	//7) Disconnect:
	iResult = shutdown(connect_socket, SD_SEND);
	closesocket(connect_socket);
	freeaddrinfo(result);
	WSACleanup();

	system("PAUSE");
}

VOID Send(SOCKET connect_socket, addrinfo* result)
{
	INT iResult = 0;
	cout << "Your nickname please: ";
	CHAR sz_nickname[32]{};
	cin.getline(sz_nickname, 32);
	CHAR send_buffer[DEFAULT_BUFFER_LENGTH] = "Hello Server, I am ";
	strcat(send_buffer, sz_nickname);
	do
	{
		iResult = send(connect_socket, send_buffer, strlen(send_buffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			cout << "Send data failed with " << WSAGetLastError() << endl;
			closesocket(connect_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		cout << iResult << " Bytes sent" << endl;

		ZeroMemory(send_buffer, sizeof(send_buffer));
		cout << "Введите сообщение: ";
		SetConsoleCP(1251);
		cin.getline(send_buffer, DEFAULT_BUFFER_LENGTH);
		SetConsoleCP(866);
		//for (int i = 0; send_buffer[i]; i++)send_buffer[i] = tolower(send_buffer[i]);
	} while (iResult > 0 && strcmp(send_buffer, "exit"));
}
VOID Recieve(SOCKET connect_socket)
{
	INT iResult = 0;
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH]{};
	do
	{
		//6) Receive data:
		iResult = recv(connect_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)cout << "Bytes received: " << iResult << ", Message: " << recvbuffer << endl;
		else if (iResult == 0)cout << "Connection closed" << endl;
		else cout << "Recieve failed with code: " << WSAGetLastError() << endl;
		
		if (strcmp(recvbuffer, SZ_SORRY) == 0)break;

		ZeroMemory(recvbuffer, sizeof(recvbuffer));
	} while (g_connected);
}