/*----------------------------------------------------------------------------------------------------------------------
--	SOURCE FILE:	tcp.cpp - An application responsible for UDP Protocol operations. Operations for both Client and
--							  Server side are included in this source file
--
--	PROGRAM:		File Transfer/Protocol Analysis
--
--	FUNCTIONS:
--					void start_server(int port, HWND hwnd);
--					std::string send_packet(char *host, int port, int packet_size, int num_packet);
--					std::string receive_packet(int port, WPARAM wParam);
--					void end_connection();
--					
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
--	The UDP Protocol is responsible for creating a connection between two computers. One side is the Client and the
--	the other is the Server. The Client side is responsible for sending data over to the Server. The Server side is
--	responsible for receiving data from the Client. Data is not sent reliably, however it is sent much quicker.
----------------------------------------------------------------------------------------------------------------------*/

#include "udp.h"

// Global Connection Socket
SOCKET udp_sock;

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
--	Starts the UDP Server and listens for connections on the given port. This function is called by WndProc
--	asynchronously to receive and dispatch events from the socket.
----------------------------------------------------------------------------------------------------------------------*/
void UDP::start_server(int port, HWND hwnd)
{
	DWORD result;
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
	if ((udp_sock = socket(PF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		perror("socket() failed with error %d\n" + WSAGetLastError());
		WSACleanup();
		return;
	}

	// Initialize Address Structure
	internet_addr.sin_family = AF_INET;
	internet_addr.sin_addr.s_addr = INADDR_ANY;
	internet_addr.sin_port = htons(port);

	// Bind socket to address structure
	if (bind(udp_sock, (PSOCKADDR)&internet_addr, sizeof(internet_addr)) == SOCKET_ERROR)
	{
		perror("bind() failed with error %d\n" + WSAGetLastError());
		WSACleanup();
		return;
	}

	WSAAsyncSelect(udp_sock, hwnd, WM_SOCKET, FD_READ);
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
--	Sends a packet of data to the Server. First the Client makes a connection to the UDP server using the given host
--	and port number. After a connection has been successfully made, the client will send the data. This function is
--	called when the user clicks on the "Send Data" menu item.
----------------------------------------------------------------------------------------------------------------------*/
std::string UDP::send_packet(char * host, int port, int packet_size, int num_packet)
{
	SOCKET data_sock;
	INT result;
	DWORD sent_bytes;
	DWORD total_bytes = 0;
	struct	hostent	*hp;
	struct	sockaddr_in server;
	WSAOVERLAPPED overlapped;
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

	// Create Datagram Socket with Overlapped Structure
	if ((data_sock = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
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

	// Allocate Single Packet Buffer
	packet_buf = (char *)malloc(packet_size * sizeof(char));

	// Create and Send Packets
	for (int i = 0; i < num_packet; i++) {
		int k = 0;
		for (int j = 0; j < packet_size; j++) {
			// Insert characters from A-Z
			k = (j < 26) ? j : j % 26;
			packet_buf[j] = 'A' + k;
		}
		if (i == num_packet - 1)
			packet_buf[num_packet - 1] = EOT;

		data_buf.buf = packet_buf;
		data_buf.len = packet_size;

		if (WSASendTo(data_sock, &data_buf, 1, &sent_bytes, 0, (PSOCKADDR)&server, sizeof(server), &overlapped, NULL) == SOCKET_ERROR) {
			DWORD errorCode = WSAGetLastError();
			if (errorCode != ERROR_IO_PENDING) {
				WaitForMultipleObjects(1, &overlapped.hEvent, true, 1000);
			}
		}
		overlapped.hEvent = WSACreateEvent();
	}

	WSACleanup();

	// Append Data Information to print_output
	print_output += "[UDP CLIENT]";
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
	print_output += std::to_string(sent_bytes * num_packet);
	print_output += " Bytes";

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
--	Sends a packet of data to the Server. First the Client makes a connection to the UDP server using the given host
--	and port number. After a connection has been successfully made, the client will send the data. This function is
--	called when the user clicks on the "Send Data" menu item.
----------------------------------------------------------------------------------------------------------------------*/
void UDP::receive_packet(int port, WPARAM wParam, std::string &print_string)
{
	char packet_buf[RECVBUFSIZE];
	WSABUF data_buf;
	data_buf.len = RECVBUFSIZE;
	data_buf.buf = packet_buf;
	DWORD received_bytes;
	SOCKADDR source_addr;
	int source_addr_len = sizeof(SOCKADDR);
	long total_bytes = 0;
	int timeout = 0;
	int packets_recvd = 0;
	DWORD flags = 0;
	SYSTEMTIME sys_time;
	std::string print_output;

	// Start Timer
	GetSystemTime(&sys_time);
	WORD start_millis = (sys_time.wSecond * 1000) + sys_time.wMilliseconds;

	// Receive Data from Socket
	do
	{
		received_bytes = 0;
		if (WSARecvFrom(udp_sock, &data_buf, 1, &received_bytes, &flags, &source_addr, &source_addr_len, NULL, NULL) == SOCKET_ERROR) 
		{
			DWORD errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK) 
			{
				if (data_buf.buf[received_bytes - 1] == EOT)
					break;
				if (timeout < 10000) 
				{
					timeout++;
					continue;
				}
				else 
				{
					break;
				}
			}
		}
		else {
			timeout = 0;
			total_bytes += received_bytes;
			packets_recvd++;
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
	print_output += "[UDP SERVER]";
	print_output += "\nTotal Transfer Time: ";
	print_output += std::to_string(end_millis - start_millis);
	print_output += " ms";
	print_output += "\nTotal Data Transferred: ";
	print_output += std::to_string(total_bytes);
	print_output += " Bytes";
	print_output += "\nNumber of Packets Received: ";
	print_output += std::to_string(packets_recvd);

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
void UDP::end_connection()
{
	closesocket(udp_sock);
}
