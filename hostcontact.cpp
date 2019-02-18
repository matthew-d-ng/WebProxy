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
// some sort of cache

int get_html(char* return_buf, size_t buf_size, char* request)
{
    // IF in cache
    // THEN return cached stuff
    // ELSE get it from the real host
    
    // NO CACHE:
    if ( make_http_request(return_buf, buf_size, request) == 0 )
    {
        return 0;
    }
    else
    {
        return -1;
    }
    
}

int make_http_request(char* response_buf, size_t buf_size, char* request)
{
    int sock;
	struct sockaddr_in server;
	char* server_address;
    int server_port;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		cout << "Could not create socket with host" << endl;
        return -1;
	}
	cout << "Socket created with host\n";
	
	server.sin_addr.s_addr = inet_addr(server_address);
	server.sin_family = AF_INET;
	server.sin_port = htons(server_port);

	// Connecting to host
	if ( connect( sock, (struct sockaddr *)&server, sizeof(server) ) < 0 )
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
    
    // start timeout

    // try {
    // Receive a reply from the server
    if ( recv(sock, response_buf, buf_size , 0) < 0 )
    {
        cerr << "recv failed" << endl;
        return -1;
    }

    // stop timer
    // } except (timer exception) { return -1; }

	close(sock);
    return 0;
}
