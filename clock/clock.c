#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

const int tty_font[] = {
    0xF999F, 0x22222, 0xF1F8F, 0xF1F1F, 0x99F11, 
    0xF8F1F, 0xF8F9F, 0xF1111, 0xF9F9F, 0xF9F1F
};

void draw_block_digit(int num, int x_off, int y_off, int width, char grid[100][500]) {
    int data = tty_font[num];
    for (int i = 0; i < 20; i++) {
        if ((data >> (19 - i)) & 1) {
            int r = i / 4, c = i % 4;
            int py = y_off + r;
            int px = x_off + (c * 2);
            if (py >= 0 && py < 100 && px >= 0 && px + 1 < 500) {
                grid[py][px] = 'M';
                grid[py][px + 1] = 'M';
            }
        }
    }
}

int main() {
    struct winsize w;
    printf("\x1b[?25l\x1b[2J"); 

    while (1) {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_w = w.ws_col;
        int term_h = w.ws_row;
        
        char grid[100][500];
        memset(grid, ' ', sizeof(grid));

        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        // Clock is 72 chars wide including spacing, 5 chars high
        int clock_w = 72;
        int clock_h = 5;

        // PERFECT CENTERING MATH
        int x = (term_w - clock_w) / 2;
        int y = (term_h - clock_h) / 2;

        // Safety clamps
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        draw_block_digit(t->tm_hour / 10, x,      y, term_w, grid);
        draw_block_digit(t->tm_hour % 10, x + 10, y, term_w, grid);
        draw_block_digit(t->tm_min / 10,  x + 28, y, term_w, grid);
        draw_block_digit(t->tm_min % 10,  x + 38, y, term_w, grid);
        draw_block_digit(t->tm_sec / 10,  x + 56, y, term_w, grid);
        draw_block_digit(t->tm_sec % 10,  x + 66, y, term_w, grid);

        printf("\x1b[H"); 
        for (int r = 0; r < term_h; r++) {
            for (int c = 0; c < term_w; c++) {
                if (r < 100 && c < 500 && grid[r][c] == 'M') {
                    printf("█"); 
                } 
                // Colons positioned precisely between HH:MM and MM:SS
                else if ((r == y + 1 || r == y + 3) && (c == x + 22 || c == x + 50)) {
                    printf(t->tm_sec % 2 == 0 ? "█" : " "); 
                } 
                else {
                    putchar(' ');
                }
            }
            if (r < term_h - 1) putchar('\n');
        }
        
        fflush(stdout);
        usleep(200000); 
    }
    return 0;
}
