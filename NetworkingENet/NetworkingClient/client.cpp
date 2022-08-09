#include <enet/enet.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

void InClientChatLoop(ENetPeer* peer);
void RecieveEvents(ENetHost* client, ENetEvent event, ENetPeer* peer);
ENetHost* CreateClientInstance();
void SetUsername();
void PrintAndConcatHistory(string str);
void UpdateScreen();

string chatUsername = "";
string chatHistory = "";
bool newUpdateForScreen = false;
bool stillConnected;

int main()
{
    SetUsername();

    ENetHost* client = CreateClientInstance();
    if (client == NULL)
    {
        cout << "An error occurred while initializing ENet Client.\n";
        exit(EXIT_FAILURE);
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;

    /* Connect to localhost:1234. */
    enet_address_set_host(&address, "localhost");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(client, &address, 2, 0);

    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        PrintAndConcatHistory("Connection to localhost : 1234 succeeded.");
        stillConnected = true;
        PrintAndConcatHistory("Type 'QUIT' anytime to exit the chat and program\n");
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        cout << "Connection to localhost:1234 failed.\n";
        return EXIT_FAILURE;
    }

    thread listeningThread(RecieveEvents, client, event, peer);

    // Chat Input Loop
    InClientChatLoop(peer);

    // Host Destroy and Cleanup
    enet_host_destroy(client);
    stillConnected = false;
    listeningThread.join();
    return 0;
}

void InClientChatLoop(ENetPeer* peer)
{
    while (true)
    {
        string messageToSend;
        getline(cin, messageToSend);

        if (messageToSend == "QUIT") {
            stillConnected = false;
            break;
        }

        // Prepend message with username
        messageToSend = chatUsername + messageToSend;

        ENetPacket* packet = enet_packet_create(messageToSend.c_str(),
            messageToSend.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
    }
}

void RecieveEvents(ENetHost* client, ENetEvent event, ENetPeer* peer)
{
    while (stillConnected)
    {
        while (enet_host_service(client, &event, 100) > 0)
        {
            // Update Screen if possible
            if (newUpdateForScreen) UpdateScreen();

            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                // Update Char with new packet, prompt for screen update
                PrintAndConcatHistory((char*)event.packet->data);
                newUpdateForScreen = true;
                enet_packet_destroy(event.packet);
                break;
            default:
                break;
            }
        }
    }
}

ENetHost* CreateClientInstance()
{
    if (enet_initialize() != 0)
    {
        cout << "An error occurred while initializing ENet.\n";
        return NULL;
    }
    atexit(enet_deinitialize);

    ENetHost* client;
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);
    return client;
}

// Initial Input From User to Get Username for connection and messages
void SetUsername()
{
    while (true)
    {
        cout << "Choose a chat username (20 character limit): ";
        getline(cin, chatUsername);

        if (chatUsername.size() <= 20 && chatUsername != "") {
            chatUsername += ": ";
            break;
        }
        system("cls");
    }
}

// Prints and Holds Chat History
void PrintAndConcatHistory(string str)
{
    cout << str << endl;
    chatHistory += str + "\n";
}

// Clears screen and replaces with Chat History
void UpdateScreen()
{
    system("cls");
    cout << chatHistory;
}