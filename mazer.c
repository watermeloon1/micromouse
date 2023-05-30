#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include "SDL.h"

#define WIDTH 457
#define HIGHT 457

#define ROW 16
#define COL 16

#define WALL 1
#define PAD 20
#define CELL (WIDTH - 2 * PAD - 17) / 16

#define NORTH (uint8_t)0x01
#define EAST  (uint8_t)0x02
#define SOUTH (uint8_t)0x04
#define WEST  (uint8_t)0x08

enum orient {
    north,
    east,
    south,
    west,
    size
};

typedef struct {
    uint32_t x;
    uint32_t y;
    enum orient orient;
} Mouse;

uint8_t global_maze[256];

void read_bytes(FILE* file, void* buffer, const size_t buffer_capacity) {
    if (file == NULL) {
        fprintf(stderr, "ERROR: file read was unsuccessful: NULL file\n");
        exit(1);
    }

    else if (buffer == NULL) {
        fprintf(stderr, "ERROR: buffer write was unsuccessful: NULL buffer\n");
        exit(1);
    }

    else {
        size_t read = fread(buffer, buffer_capacity, 1, file);
        if (read != 1) {
            if (ferror(file)) {
                fprintf(stderr, "ERROR: could not read %zu bytes from file: %s\n",
                        read, strerror(errno));
                exit(1);
            } else if (feof(file)) {
                fprintf(stderr, "ERROR: could not read %zu bytes from file: end of file\n",
                        read);
                exit(1);
            } else {
                assert(0 && "unreachable");
            }
        }
    }    
}

void maze_read_file(const char* directory) {
    FILE* file = fopen(directory, "rb");
    if (file == NULL) {
        fprintf(stderr, "ERROR: file open was unsuccessful\n");
        exit(1);
    }
    
    read_bytes(file, global_maze, ROW * COL);
    fclose(file);
}

bool maze_valid() {
    if (global_maze[0] == 0x0E) {
        return true;
    }
    return false;
}


// can only draw lines that are horizontal or vertical
void SDL_RenderDrawCustomRect(SDL_Renderer* renderer, uint32_t start_x, uint32_t start_y,
                              uint32_t end_x, uint32_t end_y, uint32_t width) {
    SDL_Rect line;
    line.x = start_x;
    line.y = start_y;
    line.w = end_y - start_y;
    line.h = end_x - start_x;

    if (line.w == 0) { line.w += width; }
    else { line.h += width; }
    
    SDL_RenderDrawRect(renderer, &line);
    SDL_RenderFillRect(renderer, &line);
}

void maze_draw_cell(SDL_Renderer* renderer, uint8_t row, uint8_t col) {

    uint8_t wall_data = global_maze[row * COL + col];
    
    uint32_t topleft_x = (PAD) + row * (CELL + 1);
    uint32_t topleft_y = (PAD + 16 * CELL + 16) - (col + 1) * (CELL + 1);

    // WEST 
    if (0 <= wall_data - WEST) {
        SDL_RenderDrawLine(renderer,
                           topleft_x,
                           topleft_y,
                           topleft_x,
                           topleft_y + CELL + 1);
        wall_data -= WEST;
    }

    // SOUTH 
    if (0 <= wall_data - SOUTH) {
        SDL_RenderDrawLine(renderer,
                           topleft_x,
                           topleft_y + CELL + 1,
                           topleft_x + CELL + 1,
                           topleft_y + CELL + 1);
        wall_data -= SOUTH;
    }

    // EAST 
    if (0 <= wall_data - EAST) {
        SDL_RenderDrawLine(renderer,
                           topleft_x + CELL + 1,
                           topleft_y,
                           topleft_x + CELL + 1,
                           topleft_y + CELL + 1);
        wall_data -= EAST;
    }

    // NORTH
    if (0 <= wall_data - NORTH) {
        SDL_RenderDrawLine(renderer,
                           topleft_x,
                           topleft_y,
                           topleft_x + CELL + 1,
                           topleft_y);
        wall_data -= NORTH;
    }
}

/*
  maze dim = 468px
  line width = 1px (17) 
  cell dim = 25px (16)
*/   
void SDL_RenderDrawMaze(SDL_Renderer* renderer) {
    for (unsigned int i = 0; i < ROW; ++i) {
        for (unsigned int j = 0; j < COL; ++j) {
            maze_draw_cell(renderer, i, j);
        }
    }
}

void rotatePoint(float* x, float* y, float pivot_x, float pivot_y) {
    float dx = *x - pivot_x;
    float dy = *y - pivot_y;

    *x = pivot_x - dy;
    *y = pivot_y + dx;
}

// transforms between SDL context and maze context
void SDL_RenderDrawMouse(SDL_Renderer* renderer, Mouse* mouse) {
    
    uint32_t pos_x = PAD + (mouse -> x + 1) + mouse -> x * CELL + CELL / 2;
    uint32_t pos_y = PAD + (ROW - mouse -> y) +  (ROW - (mouse -> y + 1)) * CELL + CELL / 2;

    float v_mouse1[] = { pos_x, pos_y - 5 };
    float v_mouse2[] = { pos_x - 5,  pos_y + 5 };
    float v_mouse3[] = { pos_x + 5, pos_y + 5 };

    for (int i = 0; i < mouse -> orient; ++i) {
        rotatePoint(&v_mouse1[0], &v_mouse1[1], (float)pos_x, (float)pos_y);
        rotatePoint(&v_mouse2[0], &v_mouse2[1], (float)pos_x, (float)pos_y);
        rotatePoint(&v_mouse3[0], &v_mouse3[1], (float)pos_x, (float)pos_y);
    }    

    SDL_Vertex vertex_1 = { {v_mouse1[0], v_mouse1[1]}, { 255, 0, 0, 255 }, { 1, 1 } };
    SDL_Vertex vertex_2 = { {v_mouse2[0], v_mouse2[1]}, { 255, 0, 0, 255 }, { 1, 1 } };
    SDL_Vertex vertex_3 = { {v_mouse3[0], v_mouse3[1]}, { 255, 0, 0, 255 }, { 1, 1 } };

    // Put them into array
    SDL_Vertex vertices[] = {
        vertex_1,
        vertex_2,
        vertex_3
    };

    SDL_RenderGeometry(renderer, NULL,
                       vertices, sizeof(vertices) / sizeof(SDL_Vertex),
                       NULL, 0);
}

int main() {
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Maze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "ERROR: failed to create SDL window: %s\n", SDL_GetError());
        exit(1);
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "ERROR: failed to create SDL renderer: %s\n", SDL_GetError());
        exit(1);
    }

    printf("SDL init had no errors!\n");
    maze_read_file("binary_mazefiles/japan2007eq.maz");
    assert(maze_valid());
    
    Mouse* mouse = malloc(sizeof(Mouse));
    mouse -> x = 0;
    mouse -> y = 0;
    mouse -> orient = north;
        
    bool quit = false;
    while(!quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); 
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
        SDL_RenderDrawMaze(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); 
        SDL_RenderDrawMouse(renderer, mouse);
        
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
