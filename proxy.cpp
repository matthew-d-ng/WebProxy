/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <sys/socket.h>
#include <iostream>
#include <unordered_set>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write

using namespace std;

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
    for (const string &url: blacklist)
        cout << url << "\n";
}

void req_listener()
{
    cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;

    int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[2000];

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		cout << "Could not create socket" << endl;
	}
	cout << "Socket created" << endl;
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		cerr << "bind failed. Error" << endl;
		return;
	}
	cout << "bind done" << endl;
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	cout << "Waiting for incoming connections..." << endl;
	c = sizeof(struct sockaddr_in);
	
	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		cerr << "accept failed" << endl;
		return;
	}
	cout << "Connection accepted" << endl;
	
	//Receive a message from client
	while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
	{
		//Send the message back to client
        cout << client_message << endl;
		write(client_sock , client_message , strlen(client_message));
	}
	
	if(read_size == 0)
	{
		cout << "Client disconnected" << endl;
	}
	else if(read_size == -1)
	{
		cerr << "recv failed" << endl;
	}
}