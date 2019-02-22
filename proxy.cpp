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
size_t BUF_SIZE = 1024;
unordered_set<string> blacklist;

string hostname_from_req(string req, bool tunnel)
{
    int i, j;
    if (!tunnel)
    {
        i = req.find("Host:");
        j = req.find("\n", i);
        i+=6;
    }
    else
    {
        i = req.find(" ");
        j = req.find(":", i);
        i++;
    }

    int length = j - i - 1;
    string name = req.substr(i+6, length);

    return name;
}

string port_from_req(string req)
{
    int i = req.find(" ");
    i = req.find(":", i);
    i++;
    int j = req.find(" ", i);

    int length = j - i - 1;
    string port = req.substr(i, length);
    return port;
}

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
	char *client_buf = new char[BUF_SIZE];
    string request;

	// Receive a message from client
    int read_size = recv(sock, client_buf, BUF_SIZE, 0);
    // remove this if fixed loop
    request.append(client_buf, read_size);
    /*
    while ( read_size > 0 )
    {
        request.append(client_buf, read_size);
        read_size = recv(sock, client_buf, BUF_SIZE, 0);
    }
    */
    cout << "REQUEST RECEIVED:\n" << request;

    bool connect_req = (request.compare(0, 7, "CONNECT") == 0);

    string host_str;
    const char* port;
    if (connect_req)
    {
        string port_str = port_from_req(request);
        port = port_str.c_str();
    }

    host_str = hostname_from_req(request, connect_req);
    const char* hostname = host_str.c_str();
    const char* req = request.c_str();
    //IF NOT BLOCKED get webpage ELSE return error

    // grab url and check against blacklist
    if ( !check_item_blocked(host_str) )
    {
        if ( connect_req )
        {
            http_tunnel(hostname, sock, port);
        }
        else if ( get_html( req, hostname, sock ) != 0 )
        {
            // write error page into response
        }
    }
    else
    {
        // write block page into http_response 
    }

    delete[] client_buf;
    close(sock);

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