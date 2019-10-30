/*----------------------------------------------------------------------------------------------------------------------
--	SOURCE FILE:	tcp.cpp - An application responsible for TCP Protocol operations. Operations for both Client and
--							  Server side are included in this source file
--
--	PROGRAM:		File Transfer/Protocol Analysis
--
--	FUNCTIONS:	
--					start_server(int port, HWND hwnd)
--					void accept_connection(WPARAM wParam, HWND hwnd)
--					std::string send_packet(char *host, int port, int packet_size, int num_packet)
--					void receive_packet(int port, WPARAM wParam)
--					void end_connection()
--				
--	DATE:			February 6, 2019
--
--	REVISIONS:		(Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	NOTES:
--	The TCP Protocol is responsible for creating a connection between two computers. One side is the Client and the
--	the other is the Server. The Client side is responsible for sending data over to the Server. The Server side is
--	responsible for receiving data from the Client. Data is sent reliably and as a stream of bytes.
----------------------------------------------------------------------------------------------------------------------*/

#include "tcp.h"

// Global Connection Socket
SOCKET tcp_sock;

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		start_server
--
--	DATE:			February 6, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		start_server(int port, HWND hwnd)
--						int port: The Port the server will be listening on
--						HWND hwnd: Window Handle
--
--	RETURNS:		void.
--
--	NOTES:
--	Starts the TCP Server and listens for connections on the given port. This function is called by WndProc
--	asynchronously to receive and dispatch events from the socket.
----------------------------------------------------------------------------------------------------------------------*/
void TCP::start_server(int port, HWND hwnd)
{
	DWORD result;
	SOCKET listen_socket;
	SOCKADDR_IN internet_addr;
	WSADATA wsaData;

	// Open up a Winsock Session
	if ((result = WSAStartup(0x0202, &wsaData)) != 0)
	{
		perror("WSAStartup failed with error %d\n" + result);
		WSACleanup();
		return;
	}

	// Create Socket
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		perror("socket() failed with error %d\n" + WSAGetLastError());
		WSACleanup();
		return;
	}

	// Initialize Address Structure
	internet_addr.sin_family = AF_INET;
	internet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	internet_addr.sin_port = htons(port);

	WSAAsyncSelect(listen_socket, hwnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);

	// Bind socket to address structure
	if (bind(listen_socket, (PSOCKADDR)&internet_addr, sizeof(internet_addr)) == SOCKET_ERROR)
	{
		perror("bind() failed with error %d\n" + WSAGetLastError());
		WSACleanup();
		return;
	}

	// Listen for connections
	if (listen(listen_socket, 5))
	{
		perror("listen() failed with error %d\n" + WSAGetLastError());
		WSACleanup();
		return;
	}
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		accept_connection
--
--	DATE:			February 6, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		accept_connection(WPARAM wParam)
--						WPARAM wParam: Parameter passed by WndProc (The socket)
--						HWND hwnd: Window Handle
--
--	RETURNS:		void.
--
--	NOTES:
--	Accepts a connection from a client in TCP. This function is called by WndProc when the WM_SOCKET's FD_ACCEPT event
--	is triggered. Once the FD_ACCEPT event is triggered a new SOCKET is created where the communication of data will
--	be taking place.
----------------------------------------------------------------------------------------------------------------------*/
void TCP::accept_connection(WPARAM wParam, HWND hwnd)
{
	// Close Old Socket Connections before Accepting
	closesocket(tcp_sock);

	if ((tcp_sock = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
	{
		printf("accept() failed with error %d\n", WSAGetLastError());
		return;
	}

	WSAAsyncSelect(tcp_sock, hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		send_packet
--
--	DATE:			February 6, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		send_packet(char *host, int port, int packet_size, int num_packet)
--						char *host: Host IP
--						int port: The Port the server is listening on
--						int packet_size: Size of a packet in Bytes
--						int num_packet: Number of packets to send
--
--	RETURNS:		std::string - output string.
--
--	NOTES:
--	Sends a packet of data to the Server. First the Client makes a connection to the TCP server using the given host
--	and port number. After a connection has been successfully made, the client will send the data. This function is
--	called when the user clicks on the "Send Data" menu item.
----------------------------------------------------------------------------------------------------------------------*/
std::string TCP::send_packet(char *host, int port, int packet_size, int num_packet)
{
	INT result;
	DWORD sent_bytes;
	DWORD total_bytes = 0;
	struct	hostent	*hp;
	struct	sockaddr_in server;
	WSAOVERLAPPED overlapped;
	SOCKET connection;
	WSABUF data_buf;
	char *packet_buf;
	WSADATA wsaData;
	std::string print_output;

	// Open up a Winsock Session
	if ((result = WSAStartup(0x0202, &wsaData)) != 0)
	{
		perror("WSAStartup failed with error %d\n" + result);
		WSACleanup();
		return "Error WSAStartup()";
	}

	// Create Stream Socket with Overlapped Structure
	if ((connection = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		perror("Cannot create socket");
		WSACleanup();
		return "Error WSASocket";
	}

	// Initialize Address Structure
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// Resolve Host
	if ((hp = gethostbyname(host)) == NULL) {
		perror("Unknown server address");
		WSACleanup();
		return "Error gethostbyname()";
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

	// Connecting to the server
	if (connect(connection, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Can't connect to server");
		WSACleanup();
		return "Error connect()";
	}

	// Allocate Single Packet Buffer
	packet_buf = (char *)malloc(packet_size * sizeof(char));

	// Create WSA Event for Asynchronous I/O
	if ((overlapped.hEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
		perror("WSACreateEvent failed");
		WSACleanup();
		return "Error WSACreateEvent()";
	}

	// Create and Send Packets
	for (int i = 0; i < num_packet; i++) {
		int k = 0;
		for (int j = 0; j < packet_size; j++) {
			// Insert characters from A-Z
			k = (j < 26) ? j : j % 26;
			packet_buf[j] = 'A' + k;
		}

		data_buf.buf = packet_buf;
		data_buf.len = packet_size;

		// Send and wait for event
		WSASend(connection, &data_buf, 1, &sent_bytes, 0, &overlapped, NULL);
		total_bytes += sent_bytes;
		WaitForSingleObject(&overlapped.hEvent, 1000);

		// Create New Event
		overlapped.hEvent = WSACreateEvent();
	}

	// Append Data Information to print_output
	print_output += "[TCP CLIENT]";
	print_output += "\nHost: ";
	print_output += host;
	print_output += "\nPort: ";
	print_output += std::to_string(port);
	print_output += "\nPacket Size: ";
	print_output += std::to_string(packet_size);
	print_output += " Bytes";
	print_output += "\nNumber of Packets: ";
	print_output += std::to_string(num_packet);
	print_output += "\nTotal Data Transferred: ";
	print_output += std::to_string(total_bytes);
	print_output += " Bytes";

	SleepEx(5, TRUE);
	closesocket(connection);
	WSACleanup();

	return print_output;
}


/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		receive_packet
--
--	DATE:			February 6, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		receive_packet(int port, WPARAM wParam)
--						int port: The Port the server is listening on
--						WPARAM wParam: Parameter passed by WndProc (The socket)
--
--	RETURNS:		std::string - output string.
--
--	NOTES:
--	Sends a packet of data to the Server. First the Client makes a connection to the TCP server using the given host
--	and port number. After a connection has been successfully made, the client will send the data. This function is
--	called when the user clicks on the "Send Data" menu item.
----------------------------------------------------------------------------------------------------------------------*/
void TCP::receive_packet(int port, WPARAM wParam, std::string &print_string)
{
	char packet_buf[RECVBUFSIZE];
	WSABUF data_buf;
	data_buf.len = RECVBUFSIZE;
	data_buf.buf = packet_buf;
	DWORD received_bytes = 0;
	DWORD flags = 0;
	DWORD total_bytes = 0;
	int timeout = 0;
	SYSTEMTIME sys_time;
	std::string print_output;

	// Start Timer
	GetSystemTime(&sys_time);
	WORD start_millis = (sys_time.wSecond * 1000) + sys_time.wMilliseconds;

	// Receive Data from Socket
	do 
	{
		received_bytes = 0;
		if (WSARecv(tcp_sock, &data_buf, 1, &received_bytes, &flags, NULL, NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) 
			{
				break;
			}
		}
		else 
		{
			OutputDebugString("Received Bytes: ");
			OutputDebugString(std::to_string(received_bytes).c_str());
			OutputDebugString("\n");
			OutputDebugString("Total Bytes: ");
			OutputDebugString(std::to_string(total_bytes).c_str());
			OutputDebugString("\n");
			if (received_bytes == 0) {
				// Timeout after 5 Loops
				if (timeout < 5) {
					timeout++;
					continue;
				}
				OutputDebugString("TIMEOUT");
				break;
			}
			timeout = 0;
			total_bytes += received_bytes;
			memset(data_buf.buf, 0, RECVBUFSIZE);
		}
	} while (true);

	if (total_bytes == 0)
	{
		return;
	}

	// Stop Timer
	GetSystemTime(&sys_time);
	DWORD end_millis = (sys_time.wSecond * 1000) + sys_time.wMilliseconds;

	// Append Received Data Statistics to print_output
	print_output += "[TCP SERVER]";
	print_output += "\nTotal Transfer Time: ";
	print_output += std::to_string(end_millis - start_millis);
	print_output += " ms";
	print_output += "\nTotal Data Transferred: ";
	print_output += std::to_string(total_bytes);
	print_output += " Bytes";

	// Wait for server to finish
	SleepEx(100, FALSE);

	// Close connection
	closesocket(tcp_sock);

	print_string = print_output;
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		end_connection
--
--	DATE:			February 6, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		end_connection()
--
--	RETURNS:		void.
--
--	NOTES:
--  Ends the connection socket created from the Client or Server. This function ensures that the connection socket
--	created when making a connection or creating a connection, is closed to ensure that a new connection is made
--	if a user decides to start the server again or send more data.
----------------------------------------------------------------------------------------------------------------------*/
void TCP::end_connection()
{
	closesocket(tcp_sock);
}
