/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <thread>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include <netdb.h>
#include <vector>

using namespace std;

const char* HTTP_OK = "HTTP/1.1 200 OK";
const int CLIENT_NODE = 0;
const char* HTTP_PORT = "80";
const char* HTTPS_PORT = "443";
const int BUF_SIZE = 1024;
// some sort of cache

int make_http_request(const char* request, const char* hostname, int client_sock)
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
	
    if (getaddrinfo(hostname, HTTP_PORT, &hints, &addrs) != 0)
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

int get_html(const char* request, const char* hostname, int client_sock)
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

int http_tunnel(const char* hostname, int client_sock, const char* port)
{
    int server_sock;
	struct sockaddr_in server;
	char* server_address;
    struct addrinfo hints = {};
    struct addrinfo *addrs;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sock == -1)
	{
		cout << "Could not create socket with host" << endl;
        return -1;
	}
	cout << "Socket created with host\n";

    if (getaddrinfo(hostname, port, &hints, &addrs) != 0)
    {
        cerr << "AAAAAAGGGGGGGGHHHHHHHHHH" << endl;
        return -1;
    }

	// Connecting to host
	if ( connect( server_sock, addrs->ai_addr, sizeof(*addrs) ) < 0 )
	{
		cerr << "Connect to host failed. Error" << endl;
		return -1;
	}

	cout << "Connected to host\n";

    send( client_sock, HTTP_OK, strlen(HTTP_OK), 0 );

    // send back & forth stuuff
    char *response_buf = new char[BUF_SIZE];
    fd_set read;
    FD_ZERO(&read);

    int bytes;
    do
    {
        FD_SET(client_sock, &read);
        FD_SET(server_sock, &read);

        if (select(2, &read, NULL, NULL, NULL) > 0)
        {
            if ( FD_ISSET(client_sock, &read) )
            {
                bytes = recv( server_sock, response_buf, BUF_SIZE, 0 );
                send( server_sock, response_buf, bytes, 0 );
            }
            if ( FD_ISSET(server_sock, &read) )
            {
                bytes = recv( server_sock, response_buf, BUF_SIZE, 0 );
                send( client_sock, response_buf, bytes, 0 );
            }
        }
    } while (bytes > 0);

    close(server_sock);
}
