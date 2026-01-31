#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main() {
    float A = 0, B = 0;

    for (;;) {
        // Get terminal size
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int width = w.ws_col;
        int height = w.ws_row;

        int size = width * height;
        float z[size];
        char b[size];

        memset(b, ' ', size);
        memset(z, 0, size * sizeof(float));

        float cx = width / 2.0f;
        float cy = height / 2.0f;

        for (float j = 0; j < 6.28; j += 0.07) {
            for (float i = 0; i < 6.28; i += 0.02) {
                float sinA = sin(A), cosA = cos(A);
                float sinB = sin(B), cosB = cos(B);
                float sini = sin(i), cosi = cos(i);
                float sinj = sin(j), cosj = cos(j);

                float h = cosj + 2;
                float D = 1 / (sini * h * sinA + sinj * cosA + 5);
                float t = sini * h * cosA - sinj * sinA;

                int x = (int)(cx + (width / 4.0f) * D *
                             (cosi * h * cosB - t * sinB));
                int y = (int)(cy + (height / 4.0f) * D *
                             (cosi * h * sinB + t * cosB));

                int o = x + width * y;

                int N = (int)(8 * (
                    (sinj * sinA - sini * cosj * cosA) * cosB -
                    sini * cosj * sinA -
                    sinj * cosA -
                    cosi * cosj * sinB
                ));

                if (y >= 0 && y < height && x >= 0 && x < width && D > z[o]) {
                    z[o] = D;
                    b[o] = ".,-~:;=!*#$@"[(N > 0) ? N : 0];
                }
            }
        }

        printf("\x1b[H"); // cursor home
        for (int k = 0; k < size; k++) {
            putchar(k % width ? b[k] : '\n');
        }

        A += 0.04;
        B += 0.02;
        usleep(30000);
    }
}

