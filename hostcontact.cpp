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

const char* HTTP_OK = "HTTP/1.1 200 Connection established\r\n\r\n";
const char* HTTP_PORT = "80";
// const char* HTTPS_PORT = "443";
const int BUF_SIZE = 2048;
// some sort of cache

int make_http_request(const char* request, const char* hostname, int client_sock)
{
    int sock;
	struct sockaddr_in server;
	char* server_address;
    struct addrinfo hints = {};
    struct addrinfo *addrs;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		cout << "Could not create socket with host" << endl;
        return -1;
	}

    if (getaddrinfo(hostname, HTTP_PORT, &hints, &addrs) != 0)
    {
        cout << "Hostname seems invalid!" << endl;
        return -1;
    }

	// Connecting to host
	if ( connect( sock, addrs->ai_addr, sizeof(*addrs) ) < 0 )
	{
		cerr << "Connect to host failed. Error" << endl;
		return -1;
	}
	
	cout << "Connected to host :" << hostname << "\n";

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

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int addr_ret = getaddrinfo(hostname, port, &hints, &addrs);
    if ( addr_ret != 0 )
    {
        cerr << "Hostname and port not working! Error: " << addr_ret  << endl;
        return -1;
    }

    server_sock = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
	if (server_sock == -1)
	{
		cout << "Could not create socket with host" << endl;
        return -1;
	}

	// Connecting to host
	if ( connect( server_sock, addrs->ai_addr, sizeof(*addrs) ) < 0 )
	{
		cerr << "Connect to host failed. Error" << endl;
		return -1;
	}

    cout << "Connected to host: " << hostname << ":" << port << endl;

    send( client_sock, HTTP_OK, strlen(HTTP_OK), 0 );

    // send back & forth stuuff
    char *response_buf = new char[BUF_SIZE];
    fd_set read;
    int maxsock = max(client_sock, server_sock);

    int c_bytes;
    int s_bytes;
    do
    {
        FD_ZERO(&read);
        FD_SET(client_sock, &read);
        FD_SET(server_sock, &read);

        if (select(maxsock+1, &read, NULL, NULL, NULL) > 0)
        {
            if ( FD_ISSET(client_sock, &read) )
            {
                c_bytes = recv( client_sock, response_buf, BUF_SIZE, 0 );
                send( server_sock, response_buf, c_bytes, 0 );
            }
            if ( FD_ISSET(server_sock, &read) )
            {
                s_bytes = recv( server_sock, response_buf, BUF_SIZE, 0 );
                send( client_sock, response_buf, s_bytes, 0 );
            }
        }
    } while (c_bytes > 0 && s_bytes > 0);

    delete[] response_buf;
    close(server_sock);
}
