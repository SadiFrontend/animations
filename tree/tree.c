#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define MAX_H 300
#define MAX_W 600

char screen[MAX_H][MAX_W];
int width, height;

// Textures for different parts of the tree
const char* trunk_tex = "█";
const char* branch_tex = "▓";
const char* twig_tex = "▒";
const char* leaf_tex = "♣";

void draw_pot(int x, int y) {
    int pot_w = 20, pot_h = 3;
    for (int i = 0; i < pot_h; i++) {
        for (int j = -pot_w/2; j <= pot_w/2; j++) {
            int px = x + j, py = y + i;
            if (px >= 0 && px < width && py >= 0 && py < height) {
                // Tapered pot design
                if (i == 0 || abs(j) < (pot_w/2 - i)) screen[py][px] = 'P';
            }
        }
    }
}

void draw_tree(float x, float y, float angle, float length, int depth, int state) {
    if (depth <= 0 || length < 1) return;

    // Drawing the segment
    for (int i = 0; i < (int)length; i++) {
        float rad = (depth > 6) ? 1.5 : 0.5; // Artificial thickness
        for(float r_off = -rad; r_off <= rad; r_off += 1.0) {
            int nx = (int)(x + cos(angle) * i * 2.2 + r_off); 
            int ny = (int)(y - sin(angle) * i);
            
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (depth > 8) screen[ny][nx] = 'T';      // Main Trunk
                else if (depth > 4) screen[ny][nx] = 'B'; // Branches
                else screen[ny][nx] = 'w';                // Twigs
            }
        }
    }

    float next_x = x + cos(angle) * length * 2.2;
    float next_y = y - sin(angle) * length;

    // Detailed leaf clusters at the tips
    if (depth <= 2) {
        for(int lo = -1; lo <= 1; lo++) {
            int lx = (int)next_x + lo;
            int ly = (int)next_y + (lo % 2);
            if (ly >= 0 && ly < height && lx >= 0 && lx < width)
                screen[ly][lx] = (state == 0) ? 'L' : 'D';
        }
    }

    // High-detail branching (shorter length reduction for a "Bigger" tree)
    draw_tree(next_x, next_y, angle + 0.42, length * 0.82, depth - 1, state);
    draw_tree(next_x, next_y, angle - 0.38, length * 0.78, depth - 1, state);
    if (depth > 5) // Occasional third branch for "Fullness"
        draw_tree(next_x, next_y, angle + 0.1, length * 0.6, depth - 2, state);
}

int main() {
    struct winsize w;
    printf("\x1b[?25l\x1b[2J"); 

    while (1) {
        // --- GROWING PHASE ---
        for (int g = 1; g <= 12; g++) { // 12 levels of recursion = Huge tree
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            width = w.ws_col; height = w.ws_row;
            memset(screen, ' ', sizeof(screen));

            int bx = width / 2;
            int by = height - 6;

            draw_pot(bx, by + 1);
            // Height/3.5 makes the trunk much taller
            draw_tree(bx, by, M_PI / 2, height / 7.0, g, 0);
            
            printf("\x1b[H");
            for (int r = 0; r < height; r++) {
                for (int c = 0; c < width; c++) {
                    char ch = screen[r][c];
                    if (ch == 'P') printf("\x1b[38;5;237m█\x1b[0m");      // Matte pot
                    else if (ch == 'T') printf("\x1b[1;37m%s\x1b[0m", trunk_tex); 
                    else if (ch == 'B') printf("\x1b[0;37m%s\x1b[0m", branch_tex);
                    else if (ch == 'w') printf("\x1b[38;5;245m%s\x1b[0m", twig_tex);
                    else if (ch == 'L') printf("\x1b[38;5;40m%s\x1b[0m", leaf_tex);
                    else putchar(' ');
                }
                if (r < height - 1) putchar('\n');
            }
            fflush(stdout);
            usleep(300000);
        }

        sleep(4); // Bloom time

        // --- WITHER PHASE ---
        printf("\x1b[H");
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                char ch = screen[r][c];
                if (ch == 'L' || ch == 'D') printf("\x1b[38;5;94m♣\x1b[0m"); // Brown
                else if (ch == 'P') printf("\x1b[38;5;237m█\x1b[0m");
                else if (ch == 'T') printf("\x1b[38;5;242m█\x1b[0m"); // Fading trunk
                else if (ch == 'B' || ch == 'w') printf("\x1b[38;5;239m▓\x1b[0m");
                else putchar(' ');
            }
            if (r < height - 1) putchar('\n');
        }
        fflush(stdout);
        sleep(2);

        printf("\x1b[2J");
        sleep(1);
    }
    return 0;
}
