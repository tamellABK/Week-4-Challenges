#pragma once
#include <string>
#include <map>

const int MAX_GUESS = 20;
const int MIN_GUESS = 1;

std::map<int, std::string> PromptMappings
{
    { 0 , "\nGuess a number between " + std::to_string(MIN_GUESS) + "-" + std::to_string(MAX_GUESS) + "!: "},
    { 1 , "\nYou Won!\n"},
    { 2 , "\nYou Lost!\n"}
};

enum PlayerTurnTable
{
    PTT_First = 0,
    PTT_Second = 1
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

struct IsCorrectPacket : public GamePacket
{
    IsCorrectPacket()
    {
        Type = PHT_IsGuessCorrect;
    }

    bool isCorrect = false;
    bool isCurrentPlayer = false;
};

struct GuessPacket : public GamePacket
{
    GuessPacket()
    {
        Type = PHT_Guess;
    }

    int guessValue = 0;
};

struct PromptPacket : public GamePacket
{
    PromptPacket()
    {
        Type = PHT_Prompt;
    }

    int promptMap = 0;
};