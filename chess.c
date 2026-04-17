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
typedef void fp_moves_gen(piece *);

void pieces_init(piece pcs[NUM_ROW][NUM_COL], Texture2D txs[NUM_PC][NUM_PT]) {
	pct board_state[NUM_ROW][NUM_COL]={{ {pc_B,pt_R},{pc_B,pt_N},{pc_B,pt_B},{pc_B,pt_Q},{pc_B,pt_K},{pc_B,pt_B},{pc_B,pt_N},{pc_B,pt_R} },{ {pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P},{pc_B,pt_P} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE},{pc_NONE,pt_NONE} },{ {pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P},{pc_W,pt_P} },{ {pc_W,pt_R},{pc_W,pt_N},{pc_W,pt_B},{pc_W,pt_Q},{pc_W,pt_K},{pc_W,pt_B},{pc_W,pt_N},{pc_W,pt_R} }};	

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			pct v_pct = board_state[row][col];
			piece *p = &pcs[row][col];
			*p=(piece) {
				.c=(cell){col, row}, 
				.pc=v_pct.pc,
				.pt=v_pct.pt,
				.pcap=pcap_FALSE, 
				.moves_size=0, 
			};
			if (v_pct.pc==-1||v_pct.pt==-1) p->active=0;
			else {
				p->active=1;
				p->tx=&txs[p->pc][p->pt];
			}
		}
	}
}

void tx_load(Texture2D txs[NUM_PC][NUM_PT], Texture2D *txb) {
	for (int pc=pc_W;pc<NUM_PC;pc++) {
		for (int pt=pt_P;pt<NUM_PT;pt++) {
			txs[pc][pt]=LoadTexture(
				TextFormat("%s/%d%d.png",PATH_TX,pc,pt));
		}
	}
    	*txb = LoadTexture(TextFormat("%s/board.png",PATH_TX));
}

void tx_unload(Texture2D txs[NUM_PC][NUM_PT], Texture2D *txb) {
	for (int pc=pc_W;pc<NUM_PC;pc++) {
		for (int pt=pt_P;pt<NUM_PT;pt++) {
			UnloadTexture(txs[pc][pt]);
		}
	}
	UnloadTexture(*txb);
}

void board_draw(Texture2D *txb) {DrawTexture(*txb,0,0,WHITE);}

void px_to_cell(Vector2 *v, cell *c) {
	*c=(cell){v->x/CELL_SIZE_PX,v->y/CELL_SIZE_PX};
}

void cell_to_px(cell *c, Vector2 *v, int centered) {
	Vector2 tmp = (Vector2){c->x*CELL_SIZE_PX,c->y*CELL_SIZE_PX};
	if (centered) {
		tmp.x+=CELL_SIZE_PX/2;
		tmp.y+=CELL_SIZE_PX/2;
	}
	*v = tmp;
}

void pieces_draw(piece pcs[NUM_ROW][NUM_COL]) {
    for (int row = 0; row < NUM_ROW; row++) {
    	for (int col = 0; col < NUM_COL; col++) {
	        piece p = pcs[row][col];
	        if (p.active) {
		        Vector2 v={0};
		        cell_to_px(&p.c,&v,0);
	        	DrawTextureV(*p.tx, v, WHITE);
	        }
	}
    }
}

void moves_gen_p(piece *p) {
	piece_color pc = p->pc;
	move *moves = p->moves;
	int starting_row = pc == pc_W ? 6 : 1;
	if (p->c.y == starting_row) {
		if (pc == pc_W) {
			moves[p->moves_size++] = (move) {0, -2};
			moves[p->moves_size++] = (move) {0, -1};
		} else {
			moves[p->moves_size++] = (move) {0, 2};
			moves[p->moves_size++] = (move) {0, 1};
		}
	} else {
		moves[p->moves_size++] = pc == pc_W ? (move) {0, -1} 
			: (move) {0, 1};
	}
}

void moves_gen_n(piece *p) {(void) p;}
void moves_gen_b(piece *p) {(void) p;}
void moves_gen_r(piece *p) {(void) p;}
void moves_gen_q(piece *p) {(void) p;}
void moves_gen_k(piece *p) {(void) p;}
void moves_gen_all(piece pcs[NUM_ROW][NUM_COL], fp_moves_gen **mdt) {
	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			piece *p=&pcs[row][col];
			if (p->active) mdt[p->pt](p);
		}
	}
}
void make_move() {}

void cell_add_move(cell *c, move *m) {
	c->x+=m->x;
	c->y+=m->y;
}

void moves_draw(piece *sp) {
	move *moves = sp->moves;
	int ms = sp->moves_size;
	for (int i=0;i<ms;i++) {
		cell c = sp->c;
	        move m=moves[i];
		Vector2 v={0};
		cell_add_move(&c,&m);
		cell_to_px(&c,&v,1);
		DrawCircleV(v, 5.0, GRAY);
	}
}

int is_click(Vector2 *mp) {
	int ret = 0;
	if (IsMouseButtonPressed(0)) {
		*mp = GetMousePosition();
		ret = 1;
	}
	return ret;
}

void get_piece_clicked(Vector2 *mp, piece pcs[NUM_ROW][NUM_COL], piece cp[2]) {
	cell c;
	piece *p; 
	px_to_cell(mp,&c);
	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			p=&pcs[row][col];
			if (p->c.x==c.x&&p->c.y==c.y) {
				cp[0] = cp[1];
				cp[1] = *p;
			}
		}
	}
}

int is_valid_move(piece cp[2]) {
	int ret = 0;
	piece p1=cp[0],p2=cp[1];
	int ms=p1.moves_size;

	for (int i=0;i<ms;i++) {
		move m=p1.moves[i];
		cell c=p1.c;
		cell_add_move(&c,&m);
		if (p2.c.x==c.x&&p2.c.y==c.y) {
			ret=1;
			goto cleanup;
		}
	}
cleanup:
	return ret;
}

int game_loop() {
	Texture2D tx_pieces[NUM_PC][NUM_PT] = {0};
	Texture2D tx_board = {0};
	piece pieces[NUM_ROW][NUM_COL] = {0};
	fp_moves_gen *mdt[NUM_PT] = {
		moves_gen_p,moves_gen_n,moves_gen_b,
		moves_gen_r,moves_gen_q,moves_gen_k
	};
	piece clicked_pcs[2] = {0};
	Vector2 mouse_pos = {0};
	int flag_gen = 1;
	//int flag_moving = 0;

	InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
	SetTargetFPS(TARGET_FPS);
	tx_load(tx_pieces, &tx_board);
	pieces_init(pieces, tx_pieces);

	while (!WindowShouldClose()) {
	        BeginDrawing();
	        ClearBackground(WHITE);

	        board_draw(&tx_board);
	        pieces_draw(pieces);
	        if (flag_gen) {
	        	moves_gen_all(pieces, (fp_moves_gen**)mdt);
	        	flag_gen=0;
	        }
	        if (is_click(&mouse_pos)) {
			get_piece_clicked(&mouse_pos, pieces, clicked_pcs);
		        if (is_valid_move(clicked_pcs)) {
				make_move();
		        }
	        }
		moves_draw(&clicked_pcs[1]);

	        EndDrawing();
	}

	tx_unload(tx_pieces, &tx_board);
	CloseWindow();
	return 0;
}
