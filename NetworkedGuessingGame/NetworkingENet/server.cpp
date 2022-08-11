#include <enet/enet.h>
#include <iostream>
#include <string>

#include "Misc.h"

using namespace std;

template <typename T>
void SendEncapPacket(ENetHost* server, GamePacket* packetType, bool sendToAll, T data);

ENetHost* CreateServerInstance();
void WaitForAllPlayerConnections(ENetHost* server);
void HandleReceivePacket(ENetHost* server, const ENetEvent& event);
int SwapPlayers(int currentPlayer);

// Has the current player been sent the Guessing Prompt
bool hasPromptedCurrentPlayer;

// Should we continue the game (ie has someone not yet guessed the number / has someone disconnected
bool continueGame = true;

// Total players connected (Max 2)
int currentPlayerCount = 0;

// Current player (0 = first connected player, 1 = second connected player
int currentPlayer;

// Goal number
int numberToGuess;

int main()
{
    // Server creation
    ENetHost* server = CreateServerInstance();
    if (server == NULL)
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    // Don't progress server until 2 players connected
    WaitForAllPlayerConnections(server);

    ENetEvent event;

    // Seed and get all random initializations
    srand(time(NULL));
    currentPlayer = rand() % 2;

    numberToGuess = rand() % (MAX_GUESS - MIN_GUESS) + MIN_GUESS;
    hasPromptedCurrentPlayer = false;

    while (continueGame)
    {
        if (!hasPromptedCurrentPlayer)
        {
            // Send prompt packet to player
            hasPromptedCurrentPlayer = true;
            PromptPacket packet = PromptPacket();
            SendEncapPacket<int>(server, &packet, false, 0);
        }
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0)
        {
            if(event.type == ENET_EVENT_TYPE_RECEIVE)
            {
                HandleReceivePacket(server, event);
            }
            else if(event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                ENetPacket* packet = enet_packet_create("A player has left the game!",
                    strlen("A player has left the game!") + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);
                /* Reset the peer's client information. */
                event.peer->data = NULL;

                // If 2 player max, and 1 leaves, send to all 
                // remaining connected they won (since that's only 1 person)
                PromptPacket promptPacket = PromptPacket();
                SendEncapPacket<int>(server, &promptPacket, true, 1);
            }
        }
    }

    enet_host_destroy(server);

    return EXIT_SUCCESS;
}

// Additional sendToAll parameter to push if packet should be sent to all connections
template <typename T>
void SendEncapPacket(ENetHost* server, GamePacket* packetType, bool sendToAll, T data)
{
    if (packetType->Type == PHT_Prompt)
    {
        // Send prompt to player according to passed data int (Check PromptMappings)
        PromptPacket* promptPacket = new PromptPacket();
        promptPacket->promptMap = data;
        ENetPacket* packet = enet_packet_create(promptPacket,
            sizeof(PromptPacket), ENET_PACKET_FLAG_RELIABLE);
        if (sendToAll)
        {
            enet_host_broadcast(server, 0, packet);
        }
        else
        {
            // If currentPlayer is 0, send to first peers element, other send second peers element
            enet_peer_send(server->peers + currentPlayer, 0, packet);
        }
        enet_host_flush(server);
        delete promptPacket;
    }
    else if (packetType->Type == PHT_IsGuessCorrect)
    {
        // Send first packet to currentPlayer
        IsCorrectPacket* isCorrectPacket = new IsCorrectPacket();
        isCorrectPacket->isCurrentPlayer = true;
        isCorrectPacket->isCorrect = (data == numberToGuess);

        ENetPacket* packet = enet_packet_create(isCorrectPacket,
            sizeof(IsCorrectPacket), ENET_PACKET_FLAG_RELIABLE);

        enet_peer_send(server->peers + currentPlayer, 0, packet);

        // Send packet again but to second player with associated bool
        isCorrectPacket->isCurrentPlayer = false;

        packet = enet_packet_create(isCorrectPacket,
            sizeof(IsCorrectPacket), ENET_PACKET_FLAG_RELIABLE);

        enet_peer_send(server->peers + SwapPlayers(currentPlayer), 0, packet);

        enet_host_flush(server);
        delete isCorrectPacket;

        // After sending response, swap to other player and resend prompt
        currentPlayer = SwapPlayers(currentPlayer);
        hasPromptedCurrentPlayer = false;

        if (data == numberToGuess)
        {
            continueGame = false;
        }
    }
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
        2      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server;
}

void WaitForAllPlayerConnections(ENetHost* server)
{
    ENetEvent event;

    while (currentPlayerCount < 2)
    {
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                /* Store any relevant client information here. */
                event.peer->data = (void*)"Client information";
                currentPlayerCount++;

                string output = "A new player has joined! " + to_string(currentPlayerCount) + "/2 Players Connected!";

                ENetPacket* packet = enet_packet_create(output.c_str(),
                    output.size() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);

            }
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                currentPlayerCount--;
                string output = "A player has disconnected! " + to_string(currentPlayerCount) + "/2 Players Connected!";

                ENetPacket* packet = enet_packet_create(output.c_str(),
                    output.size() + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, packet);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }
}

void HandleReceivePacket(ENetHost* server, const ENetEvent& event)
{
    GamePacket* RecGamePacket = (GamePacket*)(event.packet->data);
    if (RecGamePacket)
    {
        if (RecGamePacket->Type == PHT_Guess)
        {
            GuessPacket* guessPacket = (GuessPacket*)(event.packet->data);

            cout << "Is: " << guessPacket->guessValue << " == " << numberToGuess << endl;

            // Send IsCorrectPacket based on GuessPacket
            IsCorrectPacket isCorrectPacket = IsCorrectPacket();
            SendEncapPacket<int>(server, &isCorrectPacket, false, guessPacket->guessValue);
        }
        // Not a packet type so just a string to output
        else
        {
            cout << event.packet->data << endl;
        }
    }

    /* Clean up the packet now that we're done using it. */
    enet_packet_destroy(event.packet);
    {
        enet_host_flush(server);
    }
}

int SwapPlayers(int currentPlayer)
{
    if (currentPlayer == 0) return 1;
    return 0;
}