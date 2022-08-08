#include <enet/enet.h>
#include <iostream>

using namespace std;

int main()
{
    ENetHost* client;
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);
    if (client == NULL)
    {
        cout << "An error occurred while initializing ENet Client.\n";
        exit(EXIT_FAILURE);
    }

    enet_host_destroy(client);
}

