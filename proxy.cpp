/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <thread>
#include <sys/socket.h>
#include <iostream>
#include <unordered_set>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write
#include "hostcontact.h"

using namespace std;

const int PORT = 8888;
size_t BUF_SIZE = 2048;
unordered_set<string> blacklist;

bool check_item_blocked(string url)
{
    return blacklist.find(url) != blacklist.end();
}

void add_to_blacklist(string url)
{
    if ( !check_item_blocked(url) )
    {
        blacklist.insert(url);
    }
}

void remove_from_blacklist(string url)
{
    if ( check_item_blocked(url) )
    {
        blacklist.erase(url);
    }
}

void print_blacklist()
{
    cout << "Blocked websites: \n";
    for (const string &url: blacklist)
        cout << "  " << url << "\n";
}

/*
*   Yeah, I seemingly inconsistently use string/char*
*   socket.h is a C library, so the char* is needed to be compatible with that
*   Replacing the other uses of string with char* doesn't feel right :<
*/

void req_handler(int sock)
{
	int read_size;
	char *client_req = new char[BUF_SIZE];

	// Receive a message from client
	if ( (read_size = recv(sock, client_req, BUF_SIZE, 0)) > 0 )
	{
		// placeholder, delete following line when html is actually retrieving
		// write( sock, client_req, strlen(client_req) );

        cout << "REQUEST RECEIVED:\n" << client_req;

        //IF NOT BLOCKED get webpage ELSE return error
        char *http_response = new char[BUF_SIZE];

        // grab url and check against blacklist
        if ( !check_item_blocked(client_req) )
        {
            if ( get_html( http_response, BUF_SIZE, client_req, sock ) != 0 )
            {
                // write error page into response
            }
        }
        else
        {
            // write block page into http_response 
        }
        
        delete[] http_response;
        close(sock);
	}
	
    delete[] client_req;

	if (read_size == 0)
	{
		cout << "Client disconnected" << endl;
	}
	else if (read_size == -1)
	{
		cerr << "recv failed\n";
	}
    cout << "bye bye" << endl;
}

void req_listener()
{
	int socket_desc , client_sock , c;
    int *new_sock;
	struct sockaddr_in server, client;

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		cerr << "Could not create socket\n";
        return;
	}
	cout << "Socket created\n";

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );
	
	if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		cerr << "bind failed. Error\n";
		return;
	}
	cout << "bind done\n";


	listen(socket_desc, 10);


	cout << "Waiting for incoming connections...\n";
	c = sizeof(struct sockaddr_in);

    while (true)
    {
        client_sock = accept( socket_desc, (struct sockaddr *)&client, (socklen_t*)&c );
        if (client_sock)
        {
            cout << "Connection accepted\n";
            thread request ( req_handler, client_sock );
            cout << "Handler assigned\n";
            request.detach();

            // look for next client
            // client_sock = accept( socket_desc, (struct sockaddr *)&client, (socklen_t*)&c );
        }
        else
            cout << "AAAAAAAGGGHHHHHH" << endl;
    }
    
    if (client_sock < 0)
    {
        cerr << "accept failed\n";
        return;
    }
}