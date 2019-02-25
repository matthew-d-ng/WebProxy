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
#include <unordered_map>
#include "proxy.h"
#include <chrono>

using namespace std;

const char *HTTP_OK = "HTTP/1.1 200 Connection established\r\n\r\n";
const char *HTTP_PORT = "80";
// const char* HTTPS_PORT = "443";
const int BUF_SIZE = 4096;
unordered_map<string, string> cache;

/*
*   For sending a large amount of data, that could potentially overload our buffer
*/
void send_large_data(int socket, const char *data)
{
    int data_len = strlen(data);
    int current = 0;
    while (current < data_len)
    {
        int bytes_left = data_len - current;
        int chunk_size = bytes_left > BUF_SIZE ? BUF_SIZE : bytes_left;
        send(socket, &data[current], chunk_size, 0);
        current += chunk_size;
    }
}

bool valid_cache(string hostname)
{
    return cache.find(hostname) != cache.end();
}

void add_to_cache(string hostname, string response)
{
    if (cache.find(hostname) == cache.end())
    {
        cache.insert({hostname, response});
    }
}

string get_from_cache(string hostname)
{
    string result = "";
    if (cache.find(hostname) != cache.end())
        result = cache.at(hostname);
    return result;
}

int connect_to_server(const char *hostname, const char *port)
{
    int sock;
    char *server_address;

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
    int addr_ret = getaddrinfo(hostname, port, &hints, &addrs);
    if (addr_ret != 0)
    {
        cerr << "Hostname or port seems invalid! Error: " << addr_ret << endl;
        return -1;
    }

    // Connecting to host
    if (connect(sock, addrs->ai_addr, sizeof(*addrs)) < 0)
    {
        cerr << "Connect to host failed. Error" << endl;
        return -1;
    }

    cout << "Connected to host: " << hostname << ":" << port << "\n";
    return sock;
}

int make_http_request(const char *request, const char *hostname, int client_sock)
{
    int sock = connect_to_server(hostname, HTTP_PORT);
    if (sock == -1)
    {
        return -1;
    }
    cout << "Sending: " << strlen(request) << "B\n";
    send_large_data(sock, request);

    char *response_buf = new char[BUF_SIZE];
    string response = "";
    int byteSum = 0;

    int bytes = recv(sock, response_buf, BUF_SIZE, 0);
    response.append(response_buf, bytes);
    while (bytes > 0)
    {  
        byteSum += bytes;
        send(client_sock, response_buf, bytes, 0);
        bytes = recv(sock, response_buf, BUF_SIZE, 0);

        if (bytes == -1)
            cerr << "Something has broken badly oh god oh fuck";
        else
            response.append(response_buf, bytes);
    }
    cout << "Received: " << byteSum << "B\n";
    delete[] response_buf;
    cout << "Goodbye " << hostname << endl;
    close(sock);

    // add_to_cache(hostname, response);

    return 0;
}

int http_tunnel(const char *hostname, int client_sock, const char *port)
{
    int server_sock = connect_to_server(hostname, port);
    if (server_sock == -1)
        return -1;

    send(client_sock, HTTP_OK, strlen(HTTP_OK), 0);

    // send stuff back & forth
    char *response_buf = new char[BUF_SIZE];
    fd_set read;
    int maxsock = max(client_sock, server_sock);

    int c_bytes = 1;
    int s_bytes = 1;
    do
    {
        FD_ZERO(&read);
        FD_SET(client_sock, &read);
        FD_SET(server_sock, &read);

        if (select(maxsock + 1, &read, NULL, NULL, NULL) > 0)
        {
            if (FD_ISSET(client_sock, &read))
            {
                c_bytes = recv(client_sock, response_buf, BUF_SIZE, 0);
                send(server_sock, response_buf, c_bytes, 0);
            }
            if (FD_ISSET(server_sock, &read))
            {
                s_bytes = recv(server_sock, response_buf, BUF_SIZE, 0);
                send(client_sock, response_buf, s_bytes, 0);
            }
        }
    } while (c_bytes > 0 && s_bytes > 0);

    delete[] response_buf;
    cout << "Goodbye " << hostname << ":" << port << endl;
    close(server_sock);
    return 0;
}

int get_html(const char *request, const char *hostname, int client_sock)
{
    // IF in cache
    // THEN return cached stuff
    // ELSE get it from the real host
    
    auto start = chrono::high_resolution_clock::now();

    bool get_req = (request[0] =='G'); 

    if (valid_cache(hostname))
    {
        string cache_ret = get_from_cache(hostname);
        if (cache_ret != "")
        {
            cout << "Returning cache result!\n";
            const char *data = cache_ret.c_str();
            send_large_data(client_sock, data);
        }
        else if (make_http_request(request, hostname, client_sock) != 0)
        {
            return -1;
        }
    }
    else if (make_http_request(request, hostname, client_sock) != 0)
    {
        return -1;
    }

    auto finish = chrono::high_resolution_clock::now();
    auto microseconds = chrono::duration_cast<chrono::microseconds>(finish-start);
    cout << "TIME TAKEN: " << microseconds.count() << "µs\n";

    return 0;
}
