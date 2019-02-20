/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <thread>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <netdb.h>
#include <vector>

using namespace std;

const int PORT = 80;
const char* PORT_STR = "80";
const int BUF_SIZE = 1024;
// some sort of cache

int make_http_request(char* request, const char* hostname, int client_sock)
{
    int sock;
	struct sockaddr_in server;
	char* server_address;
    struct addrinfo hints = {};
    struct addrinfo *addrs;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		cout << "Could not create socket with host" << endl;
        return -1;
	}
	cout << "Socket created with host\n";
	
    if (getaddrinfo(hostname, PORT_STR, &hints, &addrs) != 0)
    {
        cout << "AAAAAAGGGGGGGGHHHHHHHHHH" << endl;
        return -1;
    }

	// Connecting to host
	if ( connect( sock, addrs->ai_addr, sizeof(*addrs) ) < 0 )
	{
		cerr << "Connect to host failed. Error" << endl;
		return -1;
	}
	
	cout << "Connected to host\n";

    // send request
    if ( send(sock, request, strlen(request), 0) < 0 )
    {
        cerr << "Send failed" << endl;
        return -1;
    }

    char *response_buf = new char[BUF_SIZE];

    // Receive a reply from the server
    int bytes = recv( sock, response_buf, BUF_SIZE, 0 );
    while ( bytes > 0 )
    {
        send( client_sock, response_buf, bytes, 0 );
        bytes = recv( sock, response_buf, BUF_SIZE, 0 );
    }

    delete[] response_buf;
	close(sock);
    return 0;
}

int get_html(char* request, const char* hostname, int client_sock)
{
    // IF in cache
    // THEN return cached stuff
    // ELSE get it from the real host
    cout << "Host: " << hostname << endl;
    // NO CACHE:
    if ( make_http_request(request, hostname, client_sock) != 0 )
    {
        return -1;
    }

    return 0;
}

int constant_connection(const char* hostname, int client_sock)
{
    
}
