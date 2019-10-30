#pragma once

#pragma warning(disable : 4996)
#pragma warning(disable : 4096)

#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define EOT (char)17
#define WM_SOCKET (WM_USER + 1)
#define BUFFERSIZE 128
#define RECVBUFSIZE 1000000
#define PORT 5150
#define PACKETSIZE 1024
#define NUMPACKETS 10
