/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <thread>
#include <sys/socket.h>
#include <iostream>
#include <unordered_set>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "hostcontact.h"
#include "proxy.h"

using namespace std;

const int PORT = 8888;
const char *HTTP_HEADER_END = "\r\n\r\n";
const char *HTTP_SERVER_ERROR = "HTTP/1.1 502 Bad Gateway\r\n\r\n";

const char *BLOCK_PAGE = "HTTP/1.1 449 Webpage Blocked\r\n\r\n"
                         "<!DOCTYPE html>"
                         "<html><head>"
                         "<h1>THIS WEBPAGE IS BLOCKED</h1>"
                         "</head></html>";

size_t BUF_SIZE = 4096;
unordered_set<string> blacklist;

string hostname_from_req(string req, bool tunnel)
{
    int i, j;

    i = req.find("Host:");
    i += 6;
    if (!tunnel)
    {
        j = req.find("\r\n", i);
    }
    else
    {
        j = req.find(":", i);
    }

    int length = j - i;
    string name = req.substr(i, length);
    return name;
}

string port_from_req(string req)
{
    int i = req.find(" ");
    i = req.find(":", i);
    i++;
    int j = req.find(" ", i);

    int length = j - i;
    string port = req.substr(i, length);
    return port;
}

int contentlen_from_req(string req)
{
    int i = req.find("Content-Length: ");
    if (i == -1)
        return 0;

    i += 16;
    int j = req.find("\r\n", i);
    return stoi(req.substr(i, j - i));
}

bool check_item_blocked(string url)
{
    return blacklist.find(url) != blacklist.end();
}

void add_to_blacklist(string url)
{
    if (!check_item_blocked(url))
    {
        blacklist.insert(url);
    }
}

void remove_from_blacklist(string url)
{
    if (check_item_blocked(url))
    {
        blacklist.erase(url);
    }
}

void print_blacklist()
{
    cout << "Blocked websites: \n";
    for (const string &url : blacklist)
        cout << "  " << url << "\n";
}

void req_handler(int sock)
{
    char *client_buf = new char[BUF_SIZE];
    string request = "";

    // read in header
    int read_size = recv(sock, client_buf, BUF_SIZE, 0);
    request.append(client_buf, read_size);
    int empty_line = request.find(HTTP_HEADER_END);

    while (empty_line == -1)
    {
        read_size = recv(sock, client_buf, BUF_SIZE, 0);
        request.append(client_buf, read_size);
        empty_line = request.find(HTTP_HEADER_END);
    }

    int content_len = contentlen_from_req(request);
    int received = request.length() - 1 - empty_line + 3;

    // read in body if it exists
    while (received < content_len)
    {
        read_size = recv(sock, client_buf, BUF_SIZE, 0);
        request.append(client_buf, read_size);
        received += read_size;
    }

    bool connect_req = (request.compare(0, 7, "CONNECT") == 0);

    string hostname;
    string port;

    if (connect_req)
        port = port_from_req(request);

    hostname = hostname_from_req(request, connect_req);

    //IF NOT BLOCKED get webpage ELSE return error

    // grab url and check against blacklist
    if (!check_item_blocked(hostname))
    {
        cout << "REQUEST RECEIVED:\n"
             << request;
        if (connect_req)
        {
            http_tunnel(hostname.c_str(), sock, port.c_str());
        }
        else if (get_html(request.c_str(), hostname.c_str(), sock) != 0)
        {
            send(sock, HTTP_SERVER_ERROR, strlen(HTTP_SERVER_ERROR), 0);
        }
    }
    else
    {
        cout << hostname << " is blocked" << endl;
        send(sock, BLOCK_PAGE, strlen(BLOCK_PAGE), 0);
    }

    // cout << "bye bye" << endl;
    delete[] client_buf;
    close(sock);
}

void req_listener()
{
    int socket_desc, client_sock, c;
    int *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        cerr << "Could not create socket\n";
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        cerr << "bind failed. Error\n";
        return;
    }

    listen(socket_desc, 20);

    cout << "Waiting for incoming connections...\n";
    c = sizeof(struct sockaddr_in);

    while (true)
    {
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock)
        {
            cout << "Connection accepted\n";
            thread request(req_handler, client_sock);
            request.detach();
        }
    }

    if (client_sock < 0)
    {
        cerr << "accept failed\n";
        return;
    }
}