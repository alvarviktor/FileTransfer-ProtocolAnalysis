#pragma once

#include "transport.h"

class TCP
{
	public:
		TCP() {};
		~TCP() {};
		void start_server(int port, HWND hwnd);
		void accept_connection(WPARAM wParam, HWND hwnd);
		std::string send_packet(char *host, int port, int packet_size, int num_packet);
		void receive_packet(int port, WPARAM wParam, std::string &print_string);
		void end_connection();
};