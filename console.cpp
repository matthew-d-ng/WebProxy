/*
*   Matthew Ng, 16323205, ngm1@tcd.ie
*/

#include <cstdio>
#include <thread>
#include <iostream>
#include "proxy.h"

using namespace std;

int main()
{
    thread proxy_service (req_listener);
    
    bool exit = false;
    while (!exit)
    {
        string cmd;
        getline(cin, cmd);

        if ( cmd.compare("exit") == 0 )
            exit = true;

        else if ( cmd.compare(0, 6, "block ") == 0 )
        {
            if (cmd.length() > 7)
                add_to_blacklist( cmd.substr(6, cmd.length() - 6) );
            else
                cout << "Enter url after 'block'" << endl;
        }
        else if ( cmd.compare(0, 8, "unblock ") == 0 )
        {
            if (cmd.length() > 9)
                remove_from_blacklist( cmd.substr(8, cmd.length() - 8) );
            else
                cout << "Enter url after 'unblock'" << endl;
        }
        else if ( cmd.compare(0, 4, "list") == 0 )
            print_blacklist();

        else
            cout << "Command not recognised" << endl;

    }

    return 0;
}