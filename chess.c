#include "chess.h"

#define NUM_COL 8
#define NUM_ROW 8
#define CELL_SIZE_PX 80

int draw_board() {
    int cf = 0;
    for (int col = 0; col < NUM_COL; col++) {
        for (int row = 0; row < NUM_ROW; row++) {
            int x = row * CELL_SIZE_PX, y = col * CELL_SIZE_PX;
            if (cf) {
                DrawRectangle(x, y, CELL_SIZE_PX, CELL_SIZE_PX, BLACK);
            } else {
                DrawRectangle(x, y, CELL_SIZE_PX, CELL_SIZE_PX, WHITE);
            }
            cf = !cf;
        }
        cf = !cf;
    }
    return 0;
}

int game_loop() {
    InitWindow(NUM_COL * CELL_SIZE_PX, NUM_ROW * CELL_SIZE_PX, "chess");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();

        draw_board();
        ClearBackground(WHITE);

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
