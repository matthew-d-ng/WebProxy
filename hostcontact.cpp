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

using namespace std;

const int PORT = 80;
const char* PORT_STR = "80";
const int BUF_SIZE = 1024;
// some sort of cache

void hostname_from_req(char* request, char* name_buf)
{
    string str(request);

    int i = str.find("Host:");
    int j = str.find("\n", i);
    int length = j - (i + 6);
    string name = str.substr(i+6, length);

    strcpy(name_buf, name.c_str());
    cout << name_buf;

    // TODO: fix cursed code
}

int make_http_request(char* response_buf, size_t buf_size, char* request)
{
    int sock;
	struct sockaddr_in server;
    char* hostname = new char[1024];
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
	
    hostname_from_req( request, hostname );
    cout << "host: " << hostname << endl;

    if (getaddrinfo("www.google.com", PORT_STR, &hints, &addrs) != 0)
    {
        cout << "AAAAAAGGGGGGGGHHHHHHHHHH" << endl;
        return -1;
    }
    delete[] hostname;

	// Connecting to host
	if ( connect( sock, addrs->ai_addr, sizeof(server) ) < 0 )
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

    // try {
    // start timeout

    // Receive a reply from the server
    if ( recv(sock, response_buf, buf_size, 0) );
    {
        
    }
    // stop timer
    // } except (timer exception) { return -1; }

	close(sock);
    return 0;
    
}

int get_html(char* return_buf, size_t buf_size, char* request)
{
    // IF in cache
    // THEN return cached stuff
    // ELSE get it from the real host
    
    // NO CACHE:
    if ( make_http_request(return_buf, buf_size, request) != 0 )
    {
        return -1;
    }

    return 0;
}
