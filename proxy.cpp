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

using namespace std;

const int PORT = 8888;
const int BUF_SIZE = 1024;
unordered_set<string> blacklist;

void add_to_blacklist(string url)
{
    if ( blacklist.find(url) == blacklist.end() )
    {
        blacklist.insert(url);
    }
}

void remove_from_blacklist(string url)
{
    if ( blacklist.find(url) != blacklist.end() )
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

void req_handler(int sock)
{
    //Get the socket descriptor
	int read_size;
	char *client_req = new char[BUF_SIZE];

	// Receive a message from client
	if( (read_size = recv(sock, client_req, BUF_SIZE, 0)) > 0 )
	{
		//Send the message back to client
		write( sock, client_req, strlen(client_req) );
        cout << "REQUEST RECEIVED:\n" << client_req;
        // IF actual client
        // THEN "get" webpage
        // ELSE add to cache and notify waiting clients
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

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		cerr << "Could not create socket\n";
        return;
	}
	cout << "Socket created\n";

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		//print the error message
		cerr << "bind failed. Error\n";
		return;
	}
	cout << "bind done\n";

	//Listen
	listen(socket_desc, 10);

	//Accept any incoming connection
	cout << "Waiting for incoming connections...\n";
	c = sizeof(struct sockaddr_in);

    client_sock = accept( socket_desc, (struct sockaddr *)&client, (socklen_t*)&c );
    while(client_sock )
    {
        cout << "Connection accepted\n";

        thread request ( req_handler, client_sock );
        cout << "Handler assigned\n";
        request.detach();
        client_sock = accept( socket_desc, (struct sockaddr *)&client, (socklen_t*)&c );
    }
    
    if (client_sock < 0)
    {
        cerr << "accept failed\n";
        return;
    }
}