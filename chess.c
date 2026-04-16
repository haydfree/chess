#include "chess.h"

#define MAX_MOVES 32
#define TARGET_FPS 20
#define WIDTH_BOARD 600
#define HEIGHT_BOARD WIDTH_BOARD
#define NUM_COL 8
#define NUM_ROW 8
#define NUM_PC 2
#define NUM_PT 6
#define CELL_SIZE_PX (WIDTH_BOARD / NUM_COL)
#define PATH_TX "assets"

typedef enum piece_color { pc_NONE=-1,pc_W=0, pc_B=1 } piece_color;
typedef enum piece_type { pt_NONE=-1,pt_P=0,pt_N=1,pt_B=2,
	pt_R=3,pt_Q=4,pt_K=5 } piece_type;
typedef enum piece_captured { pcap_FALSE, pcap_TRUE } piece_captured;
typedef struct pct { piece_color pc; piece_type pt; } pct;
typedef struct cell {int x, y;} cell;
typedef struct move {int x, y;} move;
typedef struct piece {
	cell c;
	piece_color pc;
	piece_type pt;
	piece_captured pcap;
	move moves[MAX_MOVES];
	int moves_size;
	int active;
	Texture2D *tx;
} piece;
typedef void moves_gen(piece *p);

void pieces_init(piece pcs[NUM_ROW][NUM_COL], Texture2D txs[NUM_PC][NUM_PT]) {
	pct board_state[NUM_ROW][NUM_COL]={{ {pc_B,pt_R},{pc_B,pt_N},{pc_B,pt_B},{pc_B,pt_Q},{pc_B,pt_K},{pc_B,pt_B},{pc_B,pt_N},{pc_B,pt_R} },{ {pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P} },{ {pc_W,pt_R},{pc_W,pt_N},{pc_W,pt_B},{pc_W,pt_Q},{pc_W,pt_K},{pc_W,pt_B},{pc_W,pt_N},{pc_W,pt_R} }};	

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			pct v_pct = board_state[row][col];
			if (v_pct.pc==-1||v_pct.pt==-1) continue;
			pcs[row][col] = (piece) {
				.c=(cell){col, row}, 
				.pc=v_pct.pc,
				.pt=v_pct.pt,
				.pcap=pcap_FALSE, 
				.moves_size=0, 
				.active=1,
				.tx=&txs[v_pct.pc][v_pct.pt]
			};
		}
	}
}

void textures_load(Texture2D txs[NUM_PC][NUM_PT], Texture2D *txb) {
	for (int pc=pc_W;pc<NUM_PC;pc++) {
		for (int pt=pt_P;pt<NUM_PT;pt++) {
			txs[pc][pt]=LoadTexture(
				TextFormat("%s/%d%d.png",PATH_TX,pc,pt));
		}
	}
    	*txb = LoadTexture(TextFormat("%s/board.png",PATH_TX));
}

void textures_unload(Texture2D txs[NUM_PC][NUM_PT], Texture2D *txb) {
	for (int pc=pc_W;pc<NUM_PC;pc++) {
		for (int pt=pt_P;pt<NUM_PT;pt++) {
			UnloadTexture(txs[pc][pt]);
		}
	}
	UnloadTexture(*txb);
}

void board_draw(Texture2D *txb) {DrawTexture(*txb,0,0,WHITE);}

void cell_to_px(cell *c) {
    c->x = c->x * CELL_SIZE_PX;
    c->y = c->y * CELL_SIZE_PX;
}

void pieces_draw(piece pcs[NUM_ROW][NUM_COL]) {
    for (int row = 0; row < NUM_ROW; row++) {
    	for (int col = 0; col < NUM_COL; col++) {
	        piece p = pcs[row][col];
	        cell pos = p.c;
	        cell_to_px(&pos);
	        if (p.active)
		        DrawTextureV(*p.tx, (Vector2){pos.x,pos.y}, WHITE);
	}
    }
}

void moves_gen_p(piece *p) {
    piece_color pc = p->pc;
    move *moves = p->moves;
    int starting_row = pc == pc_W ? 6 : 1;
    int mv_counter = 0;
    if (p->c.y == starting_row) {
        if (pc == pc_W) {
            moves[mv_counter++] = (move) {0, -2};
            moves[mv_counter++] = (move) {0, -1};
        } else {
            moves[mv_counter++] = (move) {0, 2};
            moves[mv_counter++] = (move) {0, 1};
        }
    } else {
        moves[mv_counter++] = pc == pc_W ? (move) {0, -1} : (move) {0, 1};
    }
    p->moves_size = mv_counter;
}

void moves_gen_n(piece *p) {(void) p;}
void moves_gen_b(piece *p) {(void) p;}
void moves_gen_r(piece *p) {(void) p;}
void moves_gen_q(piece *p) {(void) p;}
void moves_gen_k(piece *p) {(void) p;}
void moves_generate(piece *p, moves_gen **mdt) {
    piece_type pt = p->pt;
    mdt[pt](p);
}

void moves_display(piece *sel_p) {
	cell c = sel_p->c;
	move *moves = sel_p->moves;
	int ms = sel_p->moves_size;
	for (int i=0;i<ms;i++) {
		cell new = (cell){c.x+moves[i].x,c.y+moves[i].y};
		cell_to_px(&new);
		DrawCircleV((Vector2){new.x,new.y}, 5.0, GRAY);
	}
}

int is_click(Vector2 *mp) {
	*mp = GetMousePosition();
	if (IsMouseButtonPressed(0)) return 1;
	else return 0;
}

void cell_set(cell *c, Vector2 *mp) {
	*c = (cell){mp->x/CELL_SIZE_PX,mp->y/CELL_SIZE_PX};
}

int game_loop() {
	Texture2D textures[NUM_PC][NUM_PT] = {0};
	Texture2D tx_board = {0};
	piece pieces[NUM_ROW][NUM_COL] = {0};
	moves_gen *mdt[NUM_PT] = {
		moves_gen_p,moves_gen_n,moves_gen_b,
		moves_gen_r,moves_gen_q,moves_gen_k
	};
	cell sel_c = {-1, -1};
	piece sel_p = {0};
	Vector2 mouse_pos = {0};

	InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
	SetTargetFPS(TARGET_FPS);
	textures_load(textures, &tx_board);
	pieces_init(pieces, textures);

	while (!WindowShouldClose()) {
	        BeginDrawing();
	        ClearBackground(WHITE);

	        board_draw(&tx_board);
	        pieces_draw(pieces);

	        if (is_click(&mouse_pos)) {
			cell_set(&sel_c, &mouse_pos);
			moves_generate(&sel_p,(moves_gen**)&mdt);
	        }

	        EndDrawing();
	}

	textures_unload(textures, &tx_board);
	CloseWindow();
	return 0;
}
