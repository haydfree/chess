#include "chess.h"

#define MAX_PIECES 32
#define MAX_MOVES 32
#define TARGET_FPS 20
#define NUM_TEXTURES 13
#define WIDTH_BOARD 600
#define HEIGHT_BOARD WIDTH_BOARD
#define NUM_COL 8
#define NUM_ROW 8
#define CELL_SIZE_PX (WIDTH_BOARD / NUM_COL)
#define TEXTURE_FILE_PATH "assets"

typedef enum piece_color { pc_WHITE, pc_BLACK } piece_color;

typedef enum piece_type {
    pt_PAWN,
    pt_KNIGHT,
    pt_BISHOP,
    pt_ROOK,
    pt_QUEEN,
    pt_KING,
} piece_type;

typedef enum piece_captured { NOT_CAPTURED, CAPTURED } piece_captured;

typedef enum texture_buffer_idx {
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
    BLACK_KING
} texture_buffer_idx;

typedef struct piece {
    int x, y;
    piece_color pc;
    piece_type pt;
    piece_captured pcap;
    Texture2D tx;
} piece;

typedef struct move {
    int x, y;
} move;

typedef size_t moves_gen(piece *p, piece *pb, move *mv);

void pieces_buffer_init(piece *pb, Texture2D *tb) {
    pb[0] = (piece) {0, 0, pc_BLACK, pt_ROOK, NOT_CAPTURED, tb[BLACK_ROOK]};
    pb[1] = (piece) {1, 0, pc_BLACK, pt_KNIGHT, NOT_CAPTURED, tb[BLACK_KNIGHT]};
    pb[2] = (piece) {2, 0, pc_BLACK, pt_BISHOP, NOT_CAPTURED, tb[BLACK_BISHOP]};
    pb[3] = (piece) {3, 0, pc_BLACK, pt_QUEEN, NOT_CAPTURED, tb[BLACK_QUEEN]};
    pb[4] = (piece) {4, 0, pc_BLACK, pt_KING, NOT_CAPTURED, tb[BLACK_KING]};
    pb[5] = (piece) {5, 0, pc_BLACK, pt_BISHOP, NOT_CAPTURED, tb[BLACK_BISHOP]};
    pb[6] = (piece) {6, 0, pc_BLACK, pt_KNIGHT, NOT_CAPTURED, tb[BLACK_KNIGHT]};
    pb[7] = (piece) {7, 0, pc_BLACK, pt_ROOK, NOT_CAPTURED, tb[BLACK_ROOK]};
    pb[8] = (piece) {0, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[9] = (piece) {1, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[10] = (piece) {2, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[11] = (piece) {3, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[12] = (piece) {4, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[13] = (piece) {5, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[14] = (piece) {6, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};
    pb[15] = (piece) {7, 1, pc_BLACK, pt_PAWN, NOT_CAPTURED, tb[BLACK_PAWN]};

    pb[16] = (piece) {0, 7, pc_WHITE, pt_ROOK, NOT_CAPTURED, tb[WHITE_ROOK]};
    pb[17] = (piece) {1, 7, pc_WHITE, pt_KNIGHT, NOT_CAPTURED, tb[WHITE_KNIGHT]};
    pb[18] = (piece) {2, 7, pc_WHITE, pt_BISHOP, NOT_CAPTURED, tb[WHITE_BISHOP]};
    pb[19] = (piece) {3, 7, pc_WHITE, pt_QUEEN, NOT_CAPTURED, tb[WHITE_QUEEN]};
    pb[20] = (piece) {4, 7, pc_WHITE, pt_KING, NOT_CAPTURED, tb[WHITE_KING]};
    pb[21] = (piece) {5, 7, pc_WHITE, pt_BISHOP, NOT_CAPTURED, tb[WHITE_BISHOP]};
    pb[22] = (piece) {6, 7, pc_WHITE, pt_KNIGHT, NOT_CAPTURED, tb[WHITE_KNIGHT]};
    pb[23] = (piece) {7, 7, pc_WHITE, pt_ROOK, NOT_CAPTURED, tb[WHITE_ROOK]};
    pb[24] = (piece) {0, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[25] = (piece) {1, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[26] = (piece) {2, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[27] = (piece) {3, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[28] = (piece) {4, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[29] = (piece) {5, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[30] = (piece) {6, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
    pb[31] = (piece) {7, 6, pc_WHITE, pt_PAWN, NOT_CAPTURED, tb[WHITE_PAWN]};
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

size_t moves_gen_p(piece *p, piece *pb, move *mv) {
    (void) pb;
    piece_color pc = p->pc;
    int starting_row = pc == pc_WHITE ? 6 : 1;
    int mv_counter = 0;
    if (p->y == starting_row) {
        if (pc == pc_WHITE) {
            mv[mv_counter++] = (move) {0, -2};
            mv[mv_counter++] = (move) {0, -1};
        } else {
            mv[mv_counter++] = (move) {0, 2};
            mv[mv_counter++] = (move) {0, 1};
        }
    } else {
        mv[mv_counter++] = pc == pc_WHITE ? (move) {0, -1} : (move) {0, 1};
    }
    return mv_counter;
}

size_t moves_gen_n(piece *p, piece *pb, move *moves) {
    (void) p;
    (void) pb;
    (void) moves;
    return 0;
}

size_t moves_gen_b(piece *p, piece *pb, move *moves) {
    (void) p;
    (void) pb;
    (void) moves;
    return 0;
}

size_t moves_gen_r(piece *p, piece *pb, move *moves) {
    (void) p;
    (void) pb;
    (void) moves;
    return 0;
}

size_t moves_gen_q(piece *p, piece *pb, move *moves) {
    (void) p;
    (void) pb;
    (void) moves;
    return 0;
}

size_t moves_gen_k(piece *p, piece *pb, move *moves) {
    (void) p;
    (void) pb;
    (void) moves;
    return 0;
}

size_t moves_generate(piece *p, moves_gen **mdt, piece *pb, move *moves) {
    piece_type pt = p->pt;

    return mdt[pt](p, pb, moves);
}

void moves_display(piece *p, move *moves, size_t moves_size) {
    Vector2 pos = {p->x, p->y};

    for (size_t i = 0; i < moves_size; i++) {
        Vector2 v = {pos.x + moves[i].x + .5, pos.y + moves[i].y + .5};
        coord_to_px(&v);
        DrawCircleV(v, 5.0, GRAY);
    }
}

void get_piece_at_coord(Vector2 *c, piece *p, piece *pb, size_t pb_size) {
    for (size_t i = 0; i < pb_size; i++) {
        if (c->x == pb[i].x && c->y == pb[i].y) { *p = pb[i]; }
    }
}

void cell_clicked(Vector2 *v) {
    Vector2 mp = GetMousePosition();
    int cell_x = mp.x / CELL_SIZE_PX;
    int cell_y = mp.y / CELL_SIZE_PX;

    if (IsMouseButtonPressed(0)) { *v = (Vector2) {cell_x, cell_y}; }
}

int game_loop() {
    Texture2D textures_buf[NUM_TEXTURES] = {0};
    piece pieces_buf[MAX_PIECES] = {0};
    moves_gen *mdt[6] = {
        moves_gen_p,
        moves_gen_n,
        moves_gen_b,
        moves_gen_r,
        moves_gen_q,
        moves_gen_k,
    };

    Vector2 sel_c = {-1, -1};
    piece sel_p = {0};
    move moves[MAX_MOVES] = {0};
    size_t moves_size = 0;

    InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
    SetTargetFPS(TARGET_FPS);
    textures_load((Texture2D *) textures_buf);
    pieces_buffer_init((piece *) &pieces_buf, (Texture2D *) &textures_buf);

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(WHITE);

        board_draw((Texture2D *) &textures_buf[12]);
        pieces_draw((piece *) &pieces_buf, MAX_PIECES);

        cell_clicked(&sel_c);
        if (sel_c.x != -1) {
            get_piece_at_coord(
                &sel_c, &sel_p, (piece *) &pieces_buf, MAX_PIECES);
            moves_size = moves_generate(&sel_p, (moves_gen **) &mdt,
                (piece *) &pieces_buf, (move *) moves);
            moves_display((piece *) &sel_p, (move *) moves, moves_size);
        }

        EndDrawing();
    }

    textures_unload((Texture2D *) textures_buf, NUM_TEXTURES);
    CloseWindow();
    return 0;
}
