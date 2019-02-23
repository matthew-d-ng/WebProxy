/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <thread>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "proxy.h"

using namespace std;

const char* HTTP_OK = "HTTP/1.1 200 Connection established\r\n\r\n";
const char* HTTP_PORT = "80";
// const char* HTTPS_PORT = "443";
const int BUF_SIZE = 2048;
// some sort of cache

/*
*   For sending a large amount of data, that could potentially overload our buffer
*/
void send_large_data(int socket, const char* data)
{
    int data_len = strlen(data);
    int current = 0;
    while ( current < data_len )
    {
        int bytes_left = data_len - current;
        int chunk_size = bytes_left > BUF_SIZE ? BUF_SIZE : bytes_left;
        send(socket, &data[current], chunk_size, 0);
        current+=chunk_size;
    }
}

int connect_to_server(const char* hostname, const char* port) 
{
    int sock;
	char* server_address;

    struct addrinfo hints = {};
    struct addrinfo *addrs;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		cerr << "Could not create socket with host" << endl;
        return -1;
	}

    // Getting IP address
    int addr_ret =  getaddrinfo(hostname, port, &hints, &addrs);
    if (addr_ret != 0 )
    {
        cerr << "Hostname or port seems invalid! Error: " << addr_ret << endl;
        return -1;
    }

	// Connecting to host
	if ( connect( sock, addrs->ai_addr, sizeof(*addrs) ) < 0 )
	{
		cerr << "Connect to host failed. Error" << endl;
		return -1;
	}

	cout << "Connected to host: " << hostname << ":" << port << "\n";
    return sock;
}

int make_http_request(const char* request, const char* hostname, int client_sock)
{
    int sock = connect_to_server(hostname, HTTP_PORT);

    if (sock == -1)
    {
        return -1;
    }

    send_large_data(sock, request);

    char *response_buf = new char[BUF_SIZE];
    string response = "";

    int bytes = recv( sock, response_buf, BUF_SIZE, 0 );
    //response.append(response_buf, bytes);
    while ( bytes > 0 )
    {
        send( client_sock, response_buf, bytes, 0 );
        //cout << "\n\nBYTES: " << bytes << "\n";
        //cout << response_buf;
        bytes = recv( sock, response_buf, BUF_SIZE, 0 );
        if (bytes == -1)
            cerr << "Something has broken badly oh god oh fuck";
        //response.append(response_buf, bytes);
    }
    if (bytes == 0)
        cout << "Graceful exit!\n";
    // send_large_data(client_sock, response.c_str());

    delete[] response_buf;
    cout << "Goodbye " << hostname << endl;
	close(sock);
    return 0;
}

int http_tunnel(const char* hostname, int client_sock, const char* port)
{
    int server_sock = connect_to_server(hostname, port);

    if (server_sock == -1)
    {
        return -1;
    }

    send( client_sock, HTTP_OK, strlen(HTTP_OK), 0 );

    // send stuff back & forth
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

        if ( select(maxsock+1, &read, NULL, NULL, NULL) > 0 )
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
    cout << "Goodbye " << hostname << ":" << port << endl;
    close(server_sock);
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
