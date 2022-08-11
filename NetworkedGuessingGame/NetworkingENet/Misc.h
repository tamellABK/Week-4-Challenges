#pragma once
#include <string>
#include <map>

const int MAX_GUESS = 20;
const int MIN_GUESS = 1;

// Map for reached PHT_Prompt packets to reference
std::map<int, std::string> PromptMappings
{
    { 0 , "\nGuess a number between " + std::to_string(MIN_GUESS) + "-" + std::to_string(MAX_GUESS) + "!: "},
    { 1 , "\nYou Won!\n"},
    { 2 , "\nYou Lost!\n"}
};

enum PacketHeaderTypes
{
    PHT_Invalid = 0,
    PHT_IsGuessCorrect,
    PHT_Guess,
    PHT_Prompt
};

struct GamePacket
{
    GamePacket() {}
    PacketHeaderTypes Type = PHT_Invalid;
};

// Response packet to received guess from client
struct IsCorrectPacket : public GamePacket
{
    IsCorrectPacket()
    {
        Type = PHT_IsGuessCorrect;
    }

    bool isCorrect = false;
    bool isCurrentPlayer = false;
};

// Packet sent to server to include player guess
struct GuessPacket : public GamePacket
{
    GuessPacket()
    {
        Type = PHT_Guess;
    }

    int guessValue = 0;
};

// Packet sent to client to send messages
struct PromptPacket : public GamePacket
{
    PromptPacket()
    {
        Type = PHT_Prompt;
    }

    int promptMap = 0;
};