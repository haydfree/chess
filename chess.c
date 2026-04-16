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

typedef enum piece_color { pc_W, pc_B } piece_color;
typedef enum piece_type { pt_P, pt_N, pt_B, pt_R, pt_Q, pt_K } piece_type;
typedef enum piece_captured { pcap_FALSE, pcap_TRUE } piece_captured;
typedef enum piece_idx {
	WP, WN, WB, WR, WQ, WK,
	BP, BN, BB, BR, BQ, BK,
	NO=-1
} piece_idx;
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
	Texture2D *tx;
} piece;
typedef void moves_gen(piece *p);

pct pi_to_pct(piece_idx pi) {
	pct res = {0};

	if (pi<=5&&pi!=NO) res.pc=pc_W;
	else if (pi>=6) res.pc=pc_B;

	if (pi%6==0) res.pt=pt_P;
	else if (pi%6==1) res.pt=pt_K;
	else if (pi%6==2) res.pt=pt_B;
	else if (pi%6==3) res.pt=pt_R;
	else if (pi%6==4) res.pt=pt_Q;
	else if (pi%6==5) res.pt=pt_K;

	return res;
}

void pieces_init(piece pcs[NUM_ROW][NUM_COL], 
	Texture2D txs[NUM_PC][NUM_PT], 
	int board[NUM_ROW][NUM_COL]) {
	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			pct piece_info = pi_to_pct(board[row][col]);
			pcs[row][col] = (piece) {
				.c=(cell){col, row}, 
				.pc=piece_info.pc, 
				.pt=piece_info.pt,
				.pcap=pcap_FALSE, 
				.moves_size=0, 
				.tx=&txs[row][col]
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
    for (int row = 0; row < NUM_PC; row++) {
    	for (int col = 0; col < NUM_PT; col++) {
	        piece p = pcs[row][col];
	        cell pos = p.c;
	        cell_to_px(&pos);
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
	int board_state[NUM_ROW][NUM_COL] = {
		{BR,BN,BB,BQ,BK,BB,BN,BR},
		{BP,BP,BP,BP,BP,BP,BP,BP},
		{NO,NO,NO,NO,NO,NO,NO,NO},
		{NO,NO,NO,NO,NO,NO,NO,NO},
		{NO,NO,NO,NO,NO,NO,NO,NO},
		{NO,NO,NO,NO,NO,NO,NO,NO},
		{WP,WP,WP,WP,WP,WP,WP,WP},
		{WR,WN,WB,WQ,WK,WB,WN,WR},
	};
	cell sel_c = {-1, -1};
	piece sel_p = {0};
	Vector2 mouse_pos = {0};

	InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
	SetTargetFPS(TARGET_FPS);
	textures_load(textures, &tx_board);
	pieces_init(pieces, textures, board_state);

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
