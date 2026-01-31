#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define MAX_DROPS 512

// High-frequency English character set
const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$#@&%*";

typedef struct {
    int x, y, speed, len;
} Drop;

int main() {
    struct winsize w;
    printf("\x1b[?25l\x1b[2J"); // Hide cursor, clear screen
    srand(time(NULL));

    Drop drops[MAX_DROPS];
    for(int i = 0; i < MAX_DROPS; i++) {
        drops[i].x = rand() % 400;
        drops[i].y = rand() % 100;
        drops[i].speed = (rand() % 3) + 1;
        drops[i].len = (rand() % 20) + 10;
    }

    while (1) {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_w = w.ws_col, term_h = w.ws_row;

        printf("\x1b[H"); // Cursor to home

        // 0:empty, 1:head, 2:bright, 3:dim, 4:vapor
        int screen[term_h][term_w];
        memset(screen, 0, sizeof(screen));

        // Update logic: term_w / 2 because we use a 2-char column width for balance
        for(int i = 0; i < term_w / 2; i++) {
            drops[i].y += drops[i].speed;
            if (drops[i].y - drops[i].len > term_h) {
                drops[i].y = 0;
                drops[i].x = rand() % (term_w / 2);
            }

            for(int j = 0; j < drops[i].len; j++) {
                int py = drops[i].y - j;
                if(py >= 0 && py < term_h) {
                    int px = drops[i].x * 2;
                    if (px < term_w) {
                        if (j == 0) screen[py][px] = 1;
                        else if (j < 5) screen[py][px] = 2;
                        else if (j < 12) screen[py][px] = 3;
                        else screen[py][px] = 4;
                    }
                }
            }
        }

        // Render pass
        for (int r = 0; r < term_h; r++) {
            for (int c = 0; c < term_w; c++) {
                int val = screen[r][c];
                switch(val) {
                    case 1: // White Head
                        printf("\x1b[1;37m%c\x1b[0m", charset[rand() % 41]);
                        break;
                    case 2: // Bright Body
                        printf("\x1b[0;37m%c\x1b[0m", charset[rand() % 41]);
                        break;
                    case 3: // Dim Body
                        printf("\x1b[1;30m%c\x1b[0m", charset[rand() % 41]);
                        break;
                    case 4: // Vapor (UTF-8 Dot)
                        printf("\x1b[0;30m.\x1b[0m");
                        break;
                    default:
                        putchar(' ');
                }
                putchar(' '); // Spacing to maintain JetBrains grid balance
                c++; 
            }
            if (r < term_h - 1) putchar('\n');
        }
        usleep(40000); 
    }
    return 0;
}
