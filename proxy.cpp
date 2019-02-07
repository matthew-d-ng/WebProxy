#include <iostream>
#include <unordered_set>

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
}