#include <enet/enet.h>
#include <iostream>
#include <string>
#include <thread>

#include "../NetworkingENet/Misc.h"

using namespace std;

void InGuessingGameLoop(ENetPeer* peer);
void RecieveEvents(ENetHost* client, ENetEvent event, ENetPeer* peer);
ENetHost* CreateClientInstance();
void HandleReceivePacket(ENetHost* client, const ENetEvent& event);
bool ChangeTurn();

template <typename T>
void SendEncapPacket(ENetPeer* peer, GamePacket* packetType, T data);

bool stillConnected;
bool turnToGuess;
bool gameOver;

int main()
{
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
        cout << "Connection to localhost : 1234 succeeded.\n";
        stillConnected = true;
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
    InGuessingGameLoop(peer);

    // Host Destroy and Cleanup
    enet_host_destroy(client);
    stillConnected = false;
    listeningThread.join();
    return 0;
}

void InGuessingGameLoop(ENetPeer* peer)
{
    while (!gameOver)
    {
        if (turnToGuess)
        {
            string userInput;
            getline(cin, userInput);

            if (userInput == "QUIT") {
                stillConnected = false;
                break;
            }

            try
            {
                int numberToGuess = stoi(userInput);
                GuessPacket guessPacket = GuessPacket();
                SendEncapPacket<int>(peer, &guessPacket, numberToGuess);
            }
            catch(exception e)
            {
                cout << userInput << " is not a number! Try again!: ";
            }
        }
    }
}

void RecieveEvents(ENetHost* client, ENetEvent event, ENetPeer* peer)
{
    while (stillConnected)
    {
        while (enet_host_service(client, &event, 1000) > 0)
        {
            if(event.type == ENET_EVENT_TYPE_RECEIVE) HandleReceivePacket(client, event);
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

void HandleReceivePacket(ENetHost* client, const ENetEvent& event)
{
    GamePacket* RecGamePacket = (GamePacket*)(event.packet->data);
    if (RecGamePacket)
    {
        if (RecGamePacket->Type == PHT_Prompt)
        {
            PromptPacket* promptPacket = (PromptPacket*)(event.packet->data);
            cout << PromptMappings[promptPacket->promptMap];
            turnToGuess = true;
        }
        else if (RecGamePacket->Type == PHT_IsGuessCorrect)
        {
            IsCorrectPacket* isCorrectPacket = (IsCorrectPacket*)(event.packet->data);
            if (isCorrectPacket->isCorrect)
            {
                if (isCorrectPacket->isCurrentPlayer) cout << PromptMappings[1];
                else cout << PromptMappings[2];

                gameOver = true;
            }
            else
            {
                cout << "\nIncorrect Guess!\n";
            }
        }
        // Not a packet type so just a string to output
        else
        {
            cout << endl << event.packet->data << endl;
        }
    }

    /* Clean up the packet now that we're done using it. */
    enet_packet_destroy(event.packet);
    {
        enet_host_flush(client);
    }
}

bool ChangeTurn()
{
    if (turnToGuess == true) return false;
    return true;
}

template <typename T>
void SendEncapPacket(ENetPeer* peer, GamePacket* packetType, T data)
{
    if (packetType->Type == PHT_Guess)
    {
        GuessPacket* guessPacket = new GuessPacket();
        guessPacket->guessValue = data;

        ENetPacket* packet = enet_packet_create(guessPacket,
            sizeof(GuessPacket), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        delete guessPacket;

        turnToGuess = ChangeTurn();
    }
}