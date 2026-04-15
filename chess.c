#include "chess.h"

#define TARGET_FPS 20
#define NUM_COL 8
#define NUM_ROW 8
#define NUM_PIECES 12
#define CELL_SIZE_PX 80
#define TEXTURE_FILE_PATH "assets"

void load_textures(Texture2D *tb) {
    tb[0] = LoadTexture(TEXTURE_FILE_PATH "/white-pawn.png");
    tb[1] = LoadTexture(TEXTURE_FILE_PATH "/white-knight.png");
    tb[2] = LoadTexture(TEXTURE_FILE_PATH "/white-bishop.png");
    tb[3] = LoadTexture(TEXTURE_FILE_PATH "/white-rook.png");
    tb[4] = LoadTexture(TEXTURE_FILE_PATH "/white-queen.png");
    tb[5] = LoadTexture(TEXTURE_FILE_PATH "/white-king.png");

    tb[6] = LoadTexture(TEXTURE_FILE_PATH "/black-pawn.png");
    tb[7] = LoadTexture(TEXTURE_FILE_PATH "/black-knight.png");
    tb[8] = LoadTexture(TEXTURE_FILE_PATH "/black-bishop.png");
    tb[9] = LoadTexture(TEXTURE_FILE_PATH "/black-rook.png");
    tb[10] = LoadTexture(TEXTURE_FILE_PATH "/black-queen.png");
    tb[11] = LoadTexture(TEXTURE_FILE_PATH "/black-king.png");
}

void unload_textures(Texture2D *tb, size_t tb_size) {
    for (size_t i = 0; i < tb_size; i++) { UnloadTexture(tb[i]); }
}

void draw_board() {
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
}

int game_loop() {
    Texture2D texture_buf[sizeof(Texture2D) * 12] = {0};

    InitWindow(NUM_COL * CELL_SIZE_PX, NUM_ROW * CELL_SIZE_PX, "chess");
    SetTargetFPS(TARGET_FPS);
    load_textures((Texture2D *) texture_buf);

    while (!WindowShouldClose()) {
        BeginDrawing();

        draw_board();
        DrawTextureRec(texture_buf[0],
            (Rectangle) {100, 100, CELL_SIZE_PX, CELL_SIZE_PX},
            (Vector2) {100, 100}, WHITE);
        ClearBackground(WHITE);

        EndDrawing();
    }

    unload_textures((Texture2D *) texture_buf, NUM_PIECES);
    CloseWindow();
    return 0;
}
