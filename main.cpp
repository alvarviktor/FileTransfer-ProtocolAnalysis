/*----------------------------------------------------------------------------------------------------------------------
--	SOURCE FILE:	main.cpp - An application that is the starting point of the entire program reponsible for
--							   creating the window, menu items, and managing window messages
--
--	PROGRAM:		File Transfer/Protocol Analysis
--
--	FUNCTIONS:		
--					int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--					LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--					void open_help()
--					BOOL CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
--					void init_mode(HWND &hwnd, std::string title, UINT mode_id)
--					void init_dialog(HWND &hwnd)
--
--	DATE:			January 23, 2019
--
--	REVISIONS:	    February 5, 2019 [Refactored Assignment#1 to Assignment#2]
--					February 5, 2019 [Change comment headers and notes]
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	NOTES:			
--	This program will be the main driver of the entire program containing the entry point. The program creates a window,
--	menu items, and dialog boxes. The main function of this program is to generate TCP and UDP datagrams/packets and
--	transfer them using the TCP/IP protocol suite between two windows computers. The program also allows the user to
--	select from being the Client or Server application through a menu item and will notify the programs intent on the
--	application window. Furthermore, this program will utilize the lookup functions created from the previous program
--	to connect to the Server. Lastly, the user will specify the amount of data to send or send data from a file/save
--	data to a file and make a comparison between the TCP and UDP protocols which will be documented in the Technial 
--	Document that came with the application.
--
--	Inside the WndProc function there are multiple switch statements that trigger the programs main functionality
--	which is allowing the user to select their desired function. Lastly, the program was built using a layered OSI
--	model approach that divide a layers and their functions into seperate source files.
----------------------------------------------------------------------------------------------------------------------*/

#pragma comment(lib, "Ws2_32.lib")


#include <windowsx.h>
#include "resource.h"
#include "transport.h"
#include "tcp.h"
#include "udp.h"

// Enum Definition
enum Protocol { TCP_PROTOCOL, UDP_PROTOCOL };

// Function Prototypes
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
void open_help();
void init_mode(HWND &hwnd, std::string title, UINT mode_id);
void init_dialog(HWND &hwnd);
void get_control_contents(HWND &hwnd, int dlg_item, LPSTR str_buf, int size);

// Global Variables
Protocol protocol;
TCP tcp_connection;
UDP udp_connection;
static std::string print_string;
static std::string CLASS_NAME("File Transfer/Protocol Analysis");

// Initialize Default Values
int port = PORT;
int packetsize = PACKETSIZE;
int numpackets = NUMPACKETS;

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		WinMain
--
--	DATE:			January 23, 2019
--
--	REVISIONS:	    February 5, 2019 [Changed Window Class Name]
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
--							HINSTANCE hInst: A handle to the current instance of the application.		
--							HINSTANCE hprevInstance: A handle to the previous instance of the application.
--							LPSTR lspszCmdParam: The command line for the application, excluding the program name.
--							int nCmdShow: Controls how the window is to be shown
--
--	RETURNS:		int.
--
--	NOTES:
--	The entry point for a graphical windows application. Messages are recieved from user actions when interacting with
--  the window and are then sent for processing to the WndProc function.
--
--	WinMain is also responsible for creating the window and setting the properties of the window such as background,
--	cursor, menus, etc.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	HWND hwnd;
	MSG Msg;
	WNDCLASSEX Wcl;
	LPCSTR class_name = CLASS_NAME.c_str();

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION);				// large icon 
	Wcl.hIconSm = NULL;											// use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);					// cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	// white background
	Wcl.lpszClassName = class_name;

	Wcl.lpszMenuName = TEXT("MYMENU");
	Wcl.cbClsExtra = 0;											// no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(class_name, class_name, WS_OVERLAPPEDWINDOW, 10, 10,
		800, 600, NULL, NULL, hInst, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	EnableMenuItem(GetMenu(hwnd), IDM_SEND_DATA, MF_DISABLED);
	EnableMenuItem(GetMenu(hwnd), IDM_START_SERVER, MF_DISABLED);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		WndProc
--
--	DATE:			January 23, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--							HWND hwnd: A handle to the window
--							UINT Message: The system provided message
--							WPARAM wParam: Addition message information
--							LPARAM lParam: Addition message information
--
--	RETURNS:		LRESULT.
--
--	NOTES:
--	The main processing point of handling recieved messages. Messages recieved in WinMain are sent to WndProc for
--  proccessing, however if there are unproccessed messages they will be handled by the default DefWindowProc function.
--
--	Two of the most important messages that are handled in this function are the WM_COMMAND and WM_PAINT messages.
--	The WM_COMMAND message is responsible for handling user interaction with the window's menu items. When a user clicks
--	on one of the menu items, a message is triggered and handled accordingly. Secondly, the WM_PAINT message is
--	responsible for drawing to the application window. In the case of this program, it's main responsibility is to
--	draw text to the screen. When a new string is to be drawn the WM_PAINT message will call the draw functions.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	// Initialize Drawing Variables
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rec;
	UINT text_format = DT_LEFT | DT_EXTERNALLEADING | DT_WORDBREAK;
	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_HELP:
			open_help();
			break;
		case IDM_EXIT:
			// Terminate Program
			PostQuitMessage(0);
			break;
		case IDM_TCP_CLIENT:
			protocol = TCP_PROTOCOL;
			init_mode(hwnd, ": TCP CLIENT MODE", IDM_TCP_CLIENT);
			break;
		case IDM_TCP_SERVER:
			protocol = TCP_PROTOCOL;
			init_mode(hwnd, ": TCP SERVER MODE", IDM_TCP_SERVER);
			break;
		case IDM_UDP_CLIENT:
			protocol = UDP_PROTOCOL;
			init_mode(hwnd, ": UDP CLIENT MODE", IDM_UDP_CLIENT);
			break;
		case IDM_UDP_SERVER:
			protocol = UDP_PROTOCOL;
			init_mode(hwnd, ": UDP SERVER MODE", IDM_UDP_SERVER);
			break;
		case IDM_SEND_DATA:
			DialogBox(NULL, MAKEINTRESOURCE(SEND_DATA_DIALOG), hwnd, DialogProc);
			break;
		case IDM_START_SERVER:
			DialogBox(NULL, MAKEINTRESOURCE(START_SERVER_DIALOG), hwnd, DialogProc);
			break;
		}
		break;
	case WM_PAINT:
		// Draw string to main application window
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rec);
		DrawText(hdc, print_string.c_str(), strlen(print_string.c_str()), &rec, text_format);
		EndPaint(hwnd, &ps);
		ReleaseDC(hwnd, hdc);
		break;
	case WM_SOCKET:
		// Check for Errors
		if (WSAGETSELECTERROR(lParam))
		{
			perror("Socket failed with error " + WSAGETSELECTERROR(lParam));
		}
		else
		{
			switch (WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				tcp_connection.accept_connection(wParam, hwnd);
				break;
			case FD_READ:
				switch (protocol)
				{
				case TCP_PROTOCOL:
					tcp_connection.receive_packet(port, wParam, print_string);
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
					break;
				case UDP_PROTOCOL:
					udp_connection.receive_packet(port, wParam, print_string);
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
					break;
				}
				break;
			}
		}
		return 0;
	case WM_DESTROY:				
		// Terminate program
		tcp_connection.end_connection();
		udp_connection.end_connection();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		open_help
--
--	DATE:			January 23, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		open_help()
--
--	RETURNS:		void.
--
--	NOTES:
--	This function opens a MessageBox Help Dialog when the menu item is clicked. The MessageBox will display the four
--	functions the Windows Socket Lookup program provides for the user.
----------------------------------------------------------------------------------------------------------------------*/
void open_help()
{
	// MessageBox Strings
	std::string help_caption("Help");
	std::string help_text("The Application contains four of the following functions:\n\n");
	help_text += "1) Starting a TCP Server and wait for incoming data\n";
	help_text += "2) Send Data to a TCP Server as a TCP Client\n";
	help_text += "3) Starting a UDP Server and wait for incoming data\n";
	help_text += "4) Send Data to a UDP Server as a UDP Client\n\n";
	help_text += "Click on the \"Mode\" menu item to select a function";

	if (!MessageBox(NULL, help_text.c_str(), help_caption.c_str(), MB_OK))
	{
		PostQuitMessage(0);
	}
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		DialogProc
--
--	DATE:			January 23, 2019
--
--	REVISIONS:	    February 5, 2019 [Added new Dialog Boxes]
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
--						HWND hwnd: A handle to the dialog box
--						UINT Message: The message
--						WPARAM wParam: Additional message-specific information
--						LPARAM lParam: Additional message-specific information
--
--	RETURNS:		BOOL.
--
--	NOTES:
--	The DialogProc funcion is similar to the WndProc function. However, instead of managing the messages on the main
--  application window, the DialogProc handles messages that are being sent from the DialogBox. The two DialogBox were
--	created in the resource editor and each DialogBox contains controls with IDs. The control IDs are mainly used to
--	handle user interaction within a given DialogBox. Each DialogBox handles messages seperately within its own window.
----------------------------------------------------------------------------------------------------------------------*/
BOOL CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Initialize Buffers
	char host_buf[BUFFERSIZE];
	char port_buf[BUFFERSIZE];
	char packetsize_buf[BUFFERSIZE];
	char numpacket_buf[BUFFERSIZE];

	HWND packetsize_combobox = GetDlgItem(hwnd, PACKET_SIZE_COMBOBOX);
	HWND numpackets_combobox = GetDlgItem(hwnd, NUM_PACKET_COMBOBOX);

	switch (message)
	{
	case WM_INITDIALOG:
		init_dialog(hwnd);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SEND:
			// Get Options From Dialog
			get_control_contents(hwnd, HOST_EDIT_BOX, host_buf, BUFFERSIZE);
			get_control_contents(hwnd, PORT_EDIT_BOX, port_buf, BUFFERSIZE);
			ComboBox_GetLBText(packetsize_combobox, ComboBox_GetCurSel(packetsize_combobox), packetsize_buf);
			ComboBox_GetLBText(numpackets_combobox, ComboBox_GetCurSel(numpackets_combobox), numpacket_buf);

			port = atoi(port_buf);
			packetsize = atoi(packetsize_buf);
			numpackets = atoi(numpacket_buf);

			// Determine Protocol
			switch (protocol)
			{
			case TCP_PROTOCOL:
				print_string = tcp_connection.send_packet(host_buf, port, packetsize, numpackets);
				RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

				// Flush Buffers
				memset(host_buf, 0, BUFFERSIZE);
				memset(port_buf, 0, BUFFERSIZE);
				memset(packetsize_buf, 0, BUFFERSIZE);
				memset(numpacket_buf, 0, BUFFERSIZE);
				break;
			case UDP_PROTOCOL:
				print_string = udp_connection.send_packet(host_buf, port, packetsize, numpackets);
				RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

				// Flush Buffers
				memset(host_buf, 0, BUFFERSIZE);
				memset(port_buf, 0, BUFFERSIZE);
				memset(packetsize_buf, 0, BUFFERSIZE);
				memset(numpacket_buf, 0, BUFFERSIZE);
				break;
			}
			EndDialog(hwnd, 0);
			break;
		case ID_START:
			// Close Sockets Before New Server
			tcp_connection.end_connection();
			udp_connection.end_connection();

			// Get Port Number and Start Server
			get_control_contents(hwnd, PORT_EDIT_BOX, port_buf, BUFFERSIZE);
			port = atoi(port_buf);

			// Determine Protocol
			switch (protocol)
			{
			case TCP_PROTOCOL:
				tcp_connection.start_server(port, GetParent(hwnd));
				print_string = "TCP SERVER: Waiting for Connection on Port ";
				print_string += std::to_string(port);
				RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

				// Flush Buffers
				memset(host_buf, 0, BUFFERSIZE);
				memset(port_buf, 0, BUFFERSIZE);
				memset(packetsize_buf, 0, BUFFERSIZE);
				memset(numpacket_buf, 0, BUFFERSIZE);
				break;
			case UDP_PROTOCOL:
				udp_connection.start_server(port, GetParent(hwnd));
				print_string = "UDP SERVER: Waiting for Connection on Port ";
				print_string += std::to_string(port);
				RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

				// Flush Buffers
				memset(host_buf, 0, BUFFERSIZE);
				memset(port_buf, 0, BUFFERSIZE);
				memset(packetsize_buf, 0, BUFFERSIZE);
				memset(numpacket_buf, 0, BUFFERSIZE);
				break;
			}
			EndDialog(hwnd, 0);
			break;
		case ID_CANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return FALSE;
	default:
		return FALSE;
	}
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		init_mode
--
--	DATE:			February 5, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		void init_mode(HWND &hwnd, std::string title, UINT mode_id)
--						HWND &hwnd: Window Handle
--						std::string title: Window Title String
--						UINT mode_id: Menuitem ID
--
--	RETURNS:		void.
--
--	NOTES:
--	Initializes and sets the current mode of the application. There are two main modes with two specific protocols.
--	The two modes are Client and Server. The two protocols are TCP and UDP. Each mode has different funcitonality and
--	this function lets the user know which mode the application is running in.
----------------------------------------------------------------------------------------------------------------------*/
void init_mode(HWND &hwnd, std::string title, UINT mode_id)
{
	// Set Window Title
	std::string window_title(CLASS_NAME);
	window_title += title;
	SetWindowText(hwnd, window_title.c_str());

	// Enable & Disable Menu Items
	EnableMenuItem(GetMenu(hwnd), IDM_TCP_CLIENT, MF_ENABLED);
	EnableMenuItem(GetMenu(hwnd), IDM_TCP_SERVER, MF_ENABLED);
	EnableMenuItem(GetMenu(hwnd), IDM_UDP_CLIENT, MF_ENABLED);
	EnableMenuItem(GetMenu(hwnd), IDM_UDP_SERVER, MF_ENABLED);
	EnableMenuItem(GetMenu(hwnd), mode_id, MF_DISABLED);

	if (mode_id == IDM_TCP_CLIENT || mode_id == IDM_UDP_CLIENT)
	{
		// Disable Server Operations
		EnableMenuItem(GetMenu(hwnd), IDM_SEND_DATA, MF_ENABLED);
		EnableMenuItem(GetMenu(hwnd), IDM_START_SERVER, MF_ENABLED);
		EnableMenuItem(GetMenu(hwnd), IDM_START_SERVER, MF_DISABLED);
	}
	else
	{
		// Disable Client Operations
		EnableMenuItem(GetMenu(hwnd), IDM_SEND_DATA, MF_ENABLED);
		EnableMenuItem(GetMenu(hwnd), IDM_START_SERVER, MF_ENABLED);
		EnableMenuItem(GetMenu(hwnd), IDM_SEND_DATA, MF_DISABLED);
	}
}

/*----------------------------------------------------------------------------------------------------------------------
--	FUNCTION:		init_dialog
--
--	DATE:			February 5, 2019
--
--	REVISIONS:	    (Date and Description)
--
--	DESIGNER:		Viktor Alvar
--
--	PROGRAMMER:		Viktor Alvar
--
--	INTERFACE:		void init_send_dialog(HWND &hwnd)
--						HWND &hwnd: Window Handle
--
--	RETURNS:		void.
--
--	NOTES:
--	Initializes all the control widgets inside the dialog boxes created. There are two dialog boxes, SEND_DATA and 
--	START_SERVER. This function initializes each dialog box's control widgets with default values
----------------------------------------------------------------------------------------------------------------------*/
void init_dialog(HWND &hwnd)
{	
	// Get ComboBoxes
	HWND packetsize_combobox = GetDlgItem(hwnd, PACKET_SIZE_COMBOBOX);
	HWND numpackets_combobox = GetDlgItem(hwnd, NUM_PACKET_COMBOBOX);

	// Add Options to Packet Size ComboBox
	ComboBox_AddString(packetsize_combobox, "1024");
	ComboBox_AddString(packetsize_combobox, "4096");
	ComboBox_AddString(packetsize_combobox, "20000");
	ComboBox_AddString(packetsize_combobox, "60000");
	ComboBox_SetCurSel(packetsize_combobox, 0);

	// Add Options to Number of Packets ComboBox
	ComboBox_AddString(numpackets_combobox, "10");
	ComboBox_AddString(numpackets_combobox, "100");
	ComboBox_SetCurSel(numpackets_combobox, 0);

	// Set Default Host and Port# Values
	SetWindowText(GetDlgItem(hwnd, HOST_EDIT_BOX), "localhost");
	SetWindowText(GetDlgItem(hwnd, PORT_EDIT_BOX), "5150");
}

void get_control_contents(HWND &hwnd, int dlg_item, LPSTR str_buf, int size)
{
	// Error Text
	std::string error_text("GetDlgItemText() Failed");
	std::string error_caption("Error");

	if (!GetDlgItemText(hwnd, dlg_item, str_buf, size))
	{
		EndDialog(hwnd, 0);
		MessageBox(NULL, error_text.c_str(), error_caption.c_str(), MB_ICONERROR | MB_OK);
		return;
	}
}