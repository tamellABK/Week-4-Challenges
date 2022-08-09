#include <enet/enet.h>
#include <iostream>
#include <string>

using namespace std;

ENetHost* CreateServerInstance();

int main()
{
    ENetHost* server = CreateServerInstance();
    if (server == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    ENetEvent event;

    while (true)
    {
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0)
        {
            switch (event.type)
            {

            case ENET_EVENT_TYPE_CONNECT:
            {
                /* Store any relevant client information here. */
                event.peer->data = (void*)"Client information";
                ENetPacket* packet = enet_packet_create("A new member has joined the channel",
                    strlen("A new member has joined the channel") + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);
            }
                break;
            case ENET_EVENT_TYPE_RECEIVE:
            {
                ENetPacket* packet = enet_packet_create((char*)event.packet->data,
                    strlen((char*)event.packet->data) + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);
            }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                ENetPacket* packet = enet_packet_create("A member has left the channel",
                    strlen("A member has left the channel") + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }

    enet_host_destroy(server);

    return EXIT_SUCCESS;
}

ENetHost* CreateServerInstance()
{
    if (enet_initialize() != 0)
    {
        cout << "An error occurred while initializing ENet.\n";
        return NULL;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost* server;
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server;
}