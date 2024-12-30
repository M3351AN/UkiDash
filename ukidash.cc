#include <vector>
#include <string>
#include <ncurses.h>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#include <cstdio>
#include <sstream>

void execute_commands(const std::vector<std::string>& commands, std::vector<std::string>& output) {
    // Concatenate all commands into a single string
    std::string full_command;
    for (const auto& cmd : commands) {
        full_command += cmd + "\n";
    }

    // Use popen to execute the command and capture the output
    FILE* pipe = popen(full_command.c_str(), "r");
    if (!pipe) {
        output.push_back("Failed to execute command\n");
        return;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output.push_back(buffer);
    }

    pclose(pipe);
}

std::string get_current_directory() {
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    } else {
        return std::string("Unable to get current directory");
    }
}

void change_directory(const std::string& path, std::vector<std::string>& output) {
    if (chdir(path.c_str()) != 0) {
        output.push_back("Unable to change directory to: " + path + "\n");
    }
}

int main() {
    initscr(); // Initialize ncurses
    cbreak(); // Disable line buffering
    noecho(); // Disable echo
    keypad(stdscr, TRUE); // Enable function keys

    std::vector<std::string> commands;
    std::vector<std::string> output;
    std::string current_command;

    while (true) {
        clear(); // Clear screen
        std::string current_directory = get_current_directory();
        printw("Current directory: %s\n", current_directory.c_str());
        printw("Dash wat?([F7] exec/[ctrl+z] quit)\n");

        // Print previous commands and their output
        for (const auto& line : output) {
            printw("%s", line.c_str());
        }

        printw("> %s", current_command.c_str());
        move(LINES - 1, current_command.length() + 2); // Move cursor to the end of the current command

        while (true) {
            int ch = getch();
            if (ch == KEY_F(7)) {
                break; // Exit input loop on F7 key press
            } else if (ch == '\n') {
                current_command.push_back('\n');
                int y, x;
                getyx(stdscr, y, x);
                move(y + 1, 0); // Move cursor to the next line
                printw("> ");
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (!current_command.empty()) {
                    if (current_command.back() == '\n') {
                        int y, x;
                        getyx(stdscr, y, x);
                        move(y - 1, 0); // Move cursor to the previous line
                        clrtoeol();
                    }
                    current_command.pop_back();
                    int y, x;
                    getyx(stdscr, y, x);
                    mvwdelch(stdscr, y, x - 1);
                }
            } else {
                current_command.push_back(ch);
                printw("%c", ch);
            }
        }

        if (!current_command.empty()) {
            if (current_command.substr(0, 3) == "cd ") {
                std::string path = current_command.substr(3);
                change_directory(path, output);
            } else {
                commands.push_back(current_command);
                execute_commands(commands, output);
            }
            commands.clear();
            current_command.clear();
        }
    }

    endwin(); // End ncurses mode
    return 0;
}