/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <thread>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write

using namespace std;

const int PORT = 8888;
const int BUF_SIZE = 1024;


