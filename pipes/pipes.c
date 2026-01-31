#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

// UTF-8 Heavy Box Drawing Characters for JetBrains Mono
// Order: Up-Right, Up-Left, Down-Right, Down-Left, Horizontal, Vertical
const char* chars[] = {"┏", "┓", "┗", "┛", "━", "┃"};

typedef struct {
    int x, y, dir; // 0: Up, 1: Down, 2: Left, 3: Right
    int color;
} Pipe;

int main() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
    int height = w.ws_row;

    printf("\x1b[?25l\x1b[2J\x1b[H"); // Hide cursor, clear screen
    srand(time(NULL));

    // Colors: Cyan, Blue, White, Gray, Bright Black
    int colors[] = {36, 34, 37, 90, 94};
    
    Pipe p;
    // Reset function
    #define RESET_PIPE() { \
        p.x = rand() % width; \
        p.y = rand() % height; \
        p.dir = rand() % 4; \
        p.color = colors[rand() % 5]; \
    }

    RESET_PIPE();

    while (1) {
        int old_x = p.x;
        int old_y = p.y;
        int old_dir = p.dir;

        // Randomly decide to turn (20% chance)
        if (rand() % 5 == 0) {
            int turn = rand() % 2; // 0: left turn, 1: right turn
            if (p.dir == 0 || p.dir == 1) p.dir = turn + 2; // If vertical, go horizontal
            else p.dir = turn; // If horizontal, go vertical
        }

        // Move
        if (p.dir == 0) p.y--;      // Up
        else if (p.dir == 1) p.y++; // Down
        else if (p.dir == 2) p.x--; // Left
        else if (p.dir == 3) p.x++; // Right

        // Boundary Check
        if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) {
            RESET_PIPE();
            continue;
        }

        // Determine which character to draw based on turn
        const char* draw;
        if (old_dir == p.dir) {
            draw = (p.dir < 2) ? chars[5] : chars[4]; // Straight (┃ or ━)
        } else {
            // Logic for corners ┏ ┓ ┗ ┛
            if (old_dir == 0 && p.dir == 3) draw = chars[0]; // Up to Right
            else if (old_dir == 0 && p.dir == 2) draw = chars[1]; // Up to Left
            else if (old_dir == 1 && p.dir == 3) draw = chars[2]; // Down to Right
            else if (old_dir == 1 && p.dir == 2) draw = chars[3]; // Down to Left
            else if (old_dir == 2 && p.dir == 0) draw = chars[2]; // Left to Up
            else if (old_dir == 2 && p.dir == 1) draw = chars[0]; // Left to Down
            else if (old_dir == 3 && p.dir == 0) draw = chars[3]; // Right to Up
            else if (old_dir == 3 && p.dir == 1) draw = chars[1]; // Right to Down
            else draw = chars[4];
        }

        // Print character at old position
        printf("\x1b[%d;%dH\x1b[%dm%s", old_y + 1, old_x + 1, p.color, draw);
        fflush(stdout);
        usleep(30000); // Speed control
    }

    return 0;
}
