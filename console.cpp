#include <cstdio>
#include <thread>
#include <iostream>

using namespace std;

void add_to_blacklist(string url)
{
    cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGGGGHHHH" << endl;
}

void listener()
{
    cout << "AAAAAAAAAAAAAAAAAA" << endl;
}

int main()
{
    thread proxy_service (listener);
    
    bool exit = false;
    while (!exit)
    {
        string cmd;
        getline(cin, cmd);

        if ( cmd.compare("exit") == 0 )
            exit = true;
        else if ( cmd.compare(0, 6, "block ") == 0 )
            if (cmd.length() > 7)
                add_to_blacklist( cmd.substr(6, cmd.length() - 6) );
            else
            {
                cout << "Enter url after 'block'" << endl;
            }
            
        else
            cout << "Command not recognised" << endl;
        
    }

    proxy_service.join();

    return 0;
}