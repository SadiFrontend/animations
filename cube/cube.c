#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main() {
    float A = 0, B = 0;
    float cubeWidth = 7; // Smaller cube size
    
    // Get terminal size
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
    int height = w.ws_row;

    // Use a dynamic buffer based on terminal size
    float zBuffer[width * height];
    char buffer[width * height];

    printf("\x1b[2J"); // Clear screen

    while (1) {
        // Refresh terminal size in case user resized window
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
        height = w.ws_row;

        memset(buffer, ' ', width * height);
        memset(zBuffer, 0, width * height * sizeof(float));

        for (float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += 0.2) {
            for (float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += 0.2) {
                
                float faces[6][3] = {
                    {cubeX, cubeY, cubeWidth}, {cubeX, cubeY, -cubeWidth},
                    {cubeWidth, cubeY, cubeX}, {-cubeWidth, cubeY, cubeX},
                    {cubeX, cubeWidth, cubeY}, {cubeX, -cubeWidth, cubeY}
                };

                for (int i = 0; i < 6; i++) {
                    float x = faces[i][0], y = faces[i][1], z = faces[i][2];

                    // Rotation
                    float ny = y * cos(A) - z * sin(A);
                    float nz = y * sin(A) + z * cos(A);
                    y = ny; z = nz;
                    float nx = x * cos(B) + z * sin(B);
                    nz = -x * sin(B) + z * cos(B);
                    x = nx; z = nz;

                    z += 30; // Distance from camera

                    float ooZ = 1 / z; 
                    
                    // Projection + Centering
                    // We use (width / 2) and (height / 2) for the center point
                    int xp = (int)(width / 2 + 40 * ooZ * x * 2); // Multiplied by 2 for width correction
                    int yp = (int)(height / 2 + 20 * ooZ * y);

                    int idx = xp + yp * width;
                    if (idx >= 0 && idx < width * height && xp >= 0 && xp < width) {
                        if (ooZ > zBuffer[idx]) {
                            zBuffer[idx] = ooZ;
                            buffer[idx] = ".:-=+*#%@"[i]; 
                        }
                    }
                }
            }
        }

        printf("\x1b[H"); // Reset cursor to top-left
        for (int k = 0; k < width * height; k++) {
            putchar(k % width ? buffer[k] : 10);
        }

        A += 0.05; 
        B += 0.03; 
        usleep(20000); 
    }
    return 0;
}
