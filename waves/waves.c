#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main() {
    float t = 0; 
    
    // REDUCED SIZE SETTINGS:
    int gridSize = 12;      // Fewer points = smaller grid footprint
    float spacing = 0.6;    // Space between points
    float cameraDist = 60.0;// Increased distance (zoomed out)

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
    int height = w.ws_row;

    // Use a large buffer to prevent segfaults on resize
    char buffer[400 * 200]; 
    float zBuffer[400 * 200]; // To ensure waves don't draw over themselves

    printf("\x1b[2J"); // Clear screen

    while (1) {
        // Refresh terminal dimensions
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        width = w.ws_col;
        height = w.ws_row;
        if (width > 400) width = 400;   // Safety cap
        if (height > 200) height = 200; // Safety cap

        // Reset buffers
        memset(buffer, ' ', width * height);
        // Initialize Z-buffer with a very high number (depth)
        for(int i=0; i<width*height; i++) zBuffer[i] = 1000.0f;

        // Camera angles
        float pitch = 0.8; // Looking down
        float yaw = t * 0.1; // Slow rotation

        // Center calculation: (ix - gridSize/2.0) creates coords from -12 to +12
        for (int ix = 0; ix < gridSize; ix++) {
            for (int iy = 0; iy < gridSize; iy++) {
                
                // 1. Grid Generation (Centered)
                float x = (ix - gridSize / 2.0) * spacing;
                float y = (iy - gridSize / 2.0) * spacing;
                
                float dist = sqrt(x*x + y*y);
                float z = sin(dist - t) + sin(x*0.5 + t)*0.5;

                // 2. Rotation
                // Rotate around Y
                float rx = x * cos(yaw) - y * sin(yaw);
                float ry = x * sin(yaw) + y * cos(yaw);
                
                // Rotate around X
                float rz = ry * sin(pitch) + z * cos(pitch);
                ry = ry * cos(pitch) - z * sin(pitch);

                // 3. Projection
                // Push object away by cameraDist
                float depth = cameraDist + ry;
                float oneOverZ = 1 / depth;

                // SCALING:
                // We use a fixed factor (400) combined with 1/depth to shrink it.
                // The (width/2) and (height/2) forces it to the center.
                int xp = (int)(width / 2 + (rx * oneOverZ) * 600); 
                int yp = (int)(height / 2 + (rz * oneOverZ) * 300); // Y is scaled less because chars are tall

                // 4. Drawing
                int idx = xp + yp * width;
                if (xp >= 0 && xp < width && yp >= 0 && yp < height) {
                    // Only draw if this point is closer than what's already there
                    if (depth < zBuffer[idx]) {
                        zBuffer[idx] = depth;
                        
                        // Shading based on wave height
                        char pixel = '.';
                        if (z > 0.8) pixel = '@';
                        else if (z > 0.4) pixel = '%';
                        else if (z > 0.0) pixel = '#';
                        else if (z > -0.4) pixel = '+';
                        else if (z > -0.8) pixel = ':';

                        buffer[idx] = pixel;
                    }
                }
            }
        }

        // Print frame
        printf("\x1b[H");
        for (int k = 0; k < width * height; k++) {
            putchar(k % width ? buffer[k] : 10);
        }

        t += 0.1; 
        usleep(30000);
    }
    return 0;
}
