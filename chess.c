#include "chess.h"

#define MAX_PIECES 32
#define TARGET_FPS 20
#define NUM_TEXTURES 13
#define WIDTH_BOARD 600
#define HEIGHT_BOARD WIDTH_BOARD
#define NUM_COL 8
#define NUM_ROW 8
#define CELL_SIZE_PX WIDTH_BOARD / NUM_COL
#define TEXTURE_FILE_PATH "assets"

typedef enum captured { NOT_CAPTURED, CAPTURED } captured;

typedef enum piece_type {
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
} piece_type;

typedef struct piece {
    int x, y;
    piece_type type;
    Texture2D tx;
    captured cap;
} piece;

void pieces_buffer_init(piece *pb, Texture2D *tb) {
    pb[0] = (piece) {0, 0, BLACK_ROOK, tb[BLACK_ROOK], NOT_CAPTURED};
    pb[1] = (piece) {1, 0, BLACK_KNIGHT, tb[BLACK_KNIGHT], NOT_CAPTURED};
    pb[2] = (piece) {2, 0, BLACK_BISHOP, tb[BLACK_BISHOP], NOT_CAPTURED};
    pb[3] = (piece) {3, 0, BLACK_QUEEN, tb[BLACK_QUEEN], NOT_CAPTURED};
    pb[4] = (piece) {4, 0, BLACK_KING, tb[BLACK_KING], NOT_CAPTURED};
    pb[5] = (piece) {5, 0, BLACK_BISHOP, tb[BLACK_BISHOP], NOT_CAPTURED};
    pb[6] = (piece) {6, 0, BLACK_KNIGHT, tb[BLACK_KNIGHT], NOT_CAPTURED};
    pb[7] = (piece) {7, 0, BLACK_ROOK, tb[BLACK_ROOK], NOT_CAPTURED};
    pb[8] = (piece) {0, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[9] = (piece) {1, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[10] = (piece) {2, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[11] = (piece) {3, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[12] = (piece) {4, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[13] = (piece) {5, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[14] = (piece) {6, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};
    pb[15] = (piece) {7, 1, BLACK_PAWN, tb[BLACK_PAWN], NOT_CAPTURED};

    pb[16] = (piece) {0, 7, WHITE_ROOK, tb[WHITE_ROOK], NOT_CAPTURED};
    pb[17] = (piece) {1, 7, WHITE_KNIGHT, tb[WHITE_KNIGHT], NOT_CAPTURED};
    pb[18] = (piece) {2, 7, WHITE_BISHOP, tb[WHITE_BISHOP], NOT_CAPTURED};
    pb[19] = (piece) {3, 7, WHITE_QUEEN, tb[WHITE_QUEEN], NOT_CAPTURED};
    pb[20] = (piece) {4, 7, WHITE_KING, tb[WHITE_KING], NOT_CAPTURED};
    pb[21] = (piece) {5, 7, WHITE_BISHOP, tb[WHITE_BISHOP], NOT_CAPTURED};
    pb[22] = (piece) {6, 7, WHITE_KNIGHT, tb[WHITE_KNIGHT], NOT_CAPTURED};
    pb[23] = (piece) {7, 7, WHITE_ROOK, tb[WHITE_ROOK], NOT_CAPTURED};
    pb[24] = (piece) {0, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[25] = (piece) {1, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[26] = (piece) {2, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[27] = (piece) {3, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[28] = (piece) {4, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[29] = (piece) {5, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[30] = (piece) {6, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
    pb[31] = (piece) {7, 6, WHITE_PAWN, tb[WHITE_PAWN], NOT_CAPTURED};
}

void textures_load(Texture2D *tb) {
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

    tb[12] = LoadTexture(TEXTURE_FILE_PATH "/board.png");
}

void textures_unload(Texture2D *tb, size_t tb_size) {
    for (size_t i = 0; i < tb_size; i++) { UnloadTexture(tb[i]); }
}

void board_draw(Texture2D *t) {
    DrawTextureEx(*t, (Vector2) {0, 0}, 0.0f, 1.0f, WHITE);
}

void coord_to_px(Vector2 *v) {
    v->x = v->x * CELL_SIZE_PX;
    v->y = v->y * CELL_SIZE_PX;
}

void pieces_draw(piece *pb, size_t pb_size) {
    for (size_t i = 0; i < pb_size; i++) {
        piece p = pb[i];
        Vector2 pos = {p.x, p.y};
        coord_to_px(&pos);
        DrawTextureV(p.tx, pos, WHITE);
    }
}

void input_get() {
    Vector2 mp = GetMousePosition();
    Vector2 cell_hover
        = {floorf(mp.x / CELL_SIZE_PX), floorf(mp.y / CELL_SIZE_PX)};
    printf("%f %f\n", cell_hover.x, cell_hover.y);

    DrawRectangle(cell_hover.x * CELL_SIZE_PX, cell_hover.y * CELL_SIZE_PX,
        CELL_SIZE_PX, CELL_SIZE_PX, RED);
}

int game_loop() {
    Texture2D textures_buf[NUM_TEXTURES] = {0};
    piece pieces_buf[MAX_PIECES] = {0};

    InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
    SetTargetFPS(TARGET_FPS);
    textures_load((Texture2D *) textures_buf);
    pieces_buffer_init((piece *) &pieces_buf, (Texture2D *) &textures_buf);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        board_draw((Texture2D *) &textures_buf[12]);
        pieces_draw((piece *) &pieces_buf, MAX_PIECES);
        input_get();

        EndDrawing();
    }

    textures_unload((Texture2D *) textures_buf, NUM_TEXTURES);
    CloseWindow();
    return 0;
}
