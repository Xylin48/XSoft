#include "tictactoe.hpp"

#include <array>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace myos {
namespace {
using Board = std::array<char, 9>;

void draw(const Board& board, std::ostream& output) {
    output << '\n';
    for (std::size_t row = 0; row < board.size(); row += 3) {
        if (row != 0) output << "---+---+---\n";
        output << ' ' << board[row] << " | " << board[row + 1]
               << " | " << board[row + 2] << '\n';
    }
    output << '\n';
}

bool wins(const Board& board, char mark) {
    constexpr std::size_t lines[][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
        {0, 4, 8}, {2, 4, 6}
    };
    for (const auto& line : lines) {
        if (board[line[0]] == mark && board[line[1]] == mark && board[line[2]] == mark)
            return true;
    }
    return false;
}

bool play_round(bool against_bot, std::mt19937& random, std::istream& input,
                std::ostream& output) {
    std::array<std::string, 2> names{"Player1", "Player2"};
    if (!against_bot) {
        for (auto& name : names) {
            output << name << " Name > " << std::flush;
            std::string entered;
            if (!std::getline(input, entered)) return false;
            if (!entered.empty()) name = entered;
        }
    } else {
        names = {"Your turn", "Bot"};
    }

    Board board{'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    std::uniform_int_distribution<int> coin(0, 1);
    const std::array<char, 2> marks = coin(random) == 0
        ? std::array<char, 2>{'X', 'O'} : std::array<char, 2>{'O', 'X'};
    int player = coin(random);
    for (std::size_t turn = 0; turn < board.size(); ++turn, player = 1 - player) {
        const char mark = marks[player];
        draw(board, output);
        if (against_bot && player == 1) {
            std::vector<std::size_t> available;
            for (std::size_t position = 0; position < board.size(); ++position) {
                if (board[position] != 'X' && board[position] != 'O')
                    available.push_back(position);
            }
            std::uniform_int_distribution<std::size_t> pick(0, available.size() - 1);
            const auto position = available[pick(random)];
            board[position] = mark;
            output << "Bot (" << mark << ") chose " << position + 1 << ".\n";
        } else {
            for (;;) {
                output << names[player] << " (" << mark << ") > " << std::flush;
                std::string choice;
                if (!std::getline(input, choice)) return false;
                if (choice == "0") return true;
                if (choice.size() == 1 && choice[0] >= '1' && choice[0] <= '9') {
                    const auto position = static_cast<std::size_t>(choice[0] - '1');
                    if (board[position] == choice[0]) {
                        board[position] = mark;
                        break;
                    }
                }
                output << "Choose an open square from 1 to 9, or 0 to leave the match.\n";
            }
        }
        if (wins(board, mark)) {
            draw(board, output);
            if (!against_bot) output << names[player] << " Wins!\n";
            else if (player == 0) output << "You win!\n";
            else output << "Better luck next time!\n";
            return true;
        }
    }
    draw(board, output);
    output << "Match draw!\n";
    return true;
}
}

void run_tictactoe(std::istream& input, std::ostream& output) {
    std::mt19937 random{std::random_device{}()};
    for (;;) {
        output << "\nTic-Tac-Toe\n1. Play with Bot\n2. Play multiplayer\n0. Exit\n> "
               << std::flush;
        std::string choice;
        if (!std::getline(input, choice) || choice == "0") return;
        if (choice == "1" || choice == "2") {
            if (!play_round(choice == "1", random, input, output)) return;
        } else {
            output << "Choose 1, 2, or 0.\n";
        }
    }
}
}
