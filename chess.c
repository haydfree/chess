#include "chess.h"

#define MAX_MOVE_MOVES 256
#define MAX_GAME_MOVES 1024
#define TARGET_FPS 20
#define WIDTH_BOARD 600
#define HEIGHT_BOARD WIDTH_BOARD
#define NUM_COL 8
#define NUM_ROW 8
#define NUM_PC 2
#define NUM_PT 6
#define PX_PER_SQ (WIDTH_BOARD / NUM_COL)
#define PATH_TX "assets"

typedef enum piece_color {pc_NONE=-1,pc_W,pc_B} piece_color;
typedef enum piece_type {pt_NONE=-1,pt_P,pt_N,pt_B,pt_R,pt_Q,pt_K} piece_type;
typedef enum piece_captured {pcap_FALSE, pcap_TRUE} piece_captured;
typedef struct square {
	piece_color pc;
	piece_type pt;
} square;
typedef struct board {square squares[NUM_ROW][NUM_COL];} board;
typedef struct coord {int row, col;} coord;
typedef struct move {
	coord start, end;
	piece_captured pcap;
} move;
typedef struct game {
	move m[MAX_GAME_MOVES];
	move pm[MAX_MOVE_MOVES];
	board bd;
	piece_color turn;
	int score, fm, fg;
	coord cb[2];
	size_t mc, pmc;
	Vector2 mp;
} game;
typedef struct textures {
	Texture2D pieces[NUM_PC][NUM_PT];
	Texture2D board;
} textures;

typedef void mv_vtable(board*,piece_color,move[MAX_MOVE_MOVES],size_t*);

void board_init(board *b) {
square initial_b[NUM_ROW][NUM_COL]={
	{(square){pc_B,pt_R},(square){pc_B,pt_N},(square){pc_B,pt_B},(square){pc_B,pt_Q},(square){pc_B,pt_K},(square){pc_B,pt_B},(square){pc_B,pt_N},(square){pc_B,pt_R}},{(square){pc_B,pt_P},(square){pc_B,pt_P},(square){pc_B,pt_P},(square){pc_B,pt_P},(square){pc_B,pt_P},(square){pc_B,pt_P},(square){pc_B,pt_P},(square){pc_B,pt_P}},{(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE}},{(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE}},{(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE}},{(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE},(square){pc_NONE,pt_NONE}},{(square){pc_W,pt_P},(square){pc_W,pt_P},(square){pc_W,pt_P},(square){pc_W,pt_P},(square){pc_W,pt_P},(square){pc_W,pt_P},(square){pc_W,pt_P},(square){pc_W,pt_P}},{(square){pc_W,pt_R},(square){pc_W,pt_N},(square){pc_W,pt_B},(square){pc_W,pt_Q},(square){pc_W,pt_K},(square){pc_W,pt_B},(square){pc_W,pt_N},(square){pc_W,pt_R}}};

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			b->squares[row][col]=initial_b[row][col];
		}
	}
}

void textures_init(textures *t) {
	for (int pc=pc_W;pc<NUM_PC;pc++) {
		for (int pt=pt_P;pt<NUM_PT;pt++) {
			t->pieces[pc][pt]=LoadTexture(
				TextFormat("%s/%d%d.png",PATH_TX,pc,pt));
		}
	}
    	t->board = LoadTexture(TextFormat("%s/board.png",PATH_TX));
}

void textures_deinit(textures *t) {
	for (int pc=pc_W;pc<NUM_PC;pc++) {
		for (int pt=pt_P;pt<NUM_PT;pt++) {
			UnloadTexture(t->pieces[pc][pt]);
		}
	}
	UnloadTexture(t->board);
}

void px_to_coord(Vector2 *v, coord *c) {
	coord new = {0};
	new.row=v->y/PX_PER_SQ;
	new.col=v->x/PX_PER_SQ;
	*c=new;
}

void coord_to_px(coord *c, Vector2 *v, int centered) {
	Vector2 new = {c->col*PX_PER_SQ,c->row*PX_PER_SQ};
	if (centered) {
		new.x+=PX_PER_SQ/2;
		new.y+=PX_PER_SQ/2;
	}
	*v=new;
}

void board_draw(board *b, textures *t) {
	DrawTexture(t->board,0,0,WHITE);

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square *s=&b->squares[row][col];
		        if (s->pc!=pc_NONE) {
		        	coord c={row,col};
			        Vector2 v={0};
			        coord_to_px(&c,&v,0);
		        	DrawTextureV(t->pieces[s->pc][s->pt], v, WHITE);
		        }
		}
	}
}

void coord_add(coord *a, coord *b) {
	coord new = {0};
	new.row = a->row+b->row;
	new.col = a->col+b->col;
	*a=new;
}

int is_ob(coord *a) {
	int ret = 0;
	if (a->row>=NUM_ROW||a->row<0||a->col>=NUM_COL||a->col<0) ret=1;
	return ret;
}

void flip_deltas(coord *d, size_t ds) {
	for (size_t i=0;i<ds;i++) {
		coord new={0};
		new.row=d[i].row*-1;
		new.col=d[i].col;
		d[i]=new;
	}
}

void moves_gen_p(board *b, piece_color pc, move *pm, size_t *pmc) {
	const size_t ds=4;
	coord deltas[]={{1,-1},{1,0},{1,1},{2,0}};
	if (pc==pc_W) flip_deltas(deltas,ds);

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square s=b->squares[row][col];
			if (s.pc!=pc||s.pt!=pt_P) continue;
			for (size_t d=0;d<ds;d++) {
				coord c={row,col};
				piece_captured pcap = pcap_FALSE;
				coord_add(&c,&deltas[d]);
				if (is_ob(&c)) continue;
				if (b->squares[c.row][c.col].pc!=pc_NONE)
					pcap = pcap_TRUE;
				pm[(*pmc)++]= (move) {
					.start=(coord){row,col},
					.end=c,
					.pcap=pcap
				};
			}
		}
	}
}
void moves_gen_n(board *b, piece_color pc, move *pm, size_t *pmc) {
	(void)b;(void)pc;(void)pm;(void)pmc;
}
void moves_gen_b(board *b, piece_color pc, move *pm, size_t *pmc) {
	(void)b;(void)pc;(void)pm;(void)pmc;
}
void moves_gen_r(board *b, piece_color pc, move *pm, size_t *pmc) {
	(void)b;(void)pc;(void)pm;(void)pmc;
}
void moves_gen_q(board *b, piece_color pc, move *pm, size_t *pmc) {
	(void)b;(void)pc;(void)pm;(void)pmc;
}
void moves_gen_k(board *b, piece_color pc, move *pm, size_t *pmc) {
	(void)b;(void)pc;(void)pm;(void)pmc;
}
void moves_gen_all(board *b,piece_color pc,move pm[MAX_MOVE_MOVES],
	size_t *pmc,mv_vtable **mvt) {
	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square *s=&b->squares[row][col];
			if (s->pt!=pt_NONE&&s->pc==pc) 
				mvt[s->pt](b,pc,pm,pmc);
		}
	}
}

void clear_square(square *s) {
	s->pc=pc_NONE;
	s->pt=pt_NONE;
}

void move_make(board *b, coord cb[2]) {
	square *start=&b->squares[cb[0].row][cb[0].col];
	square *end=&b->squares[cb[1].row][cb[1].col];
	*end=*start;
	clear_square(start);
}

void moves_draw(move *moves, size_t s, coord *c) {
	for (size_t i=0;i<s;i++) {
		if (moves[i].start.row!=c->row||moves[i].start.col!=c->col)
			continue;
		Vector2 v={0};
		coord_to_px(&moves[i].end,&v,1);
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

int is_click_piece(board *b, Vector2 *mp, coord *c) {
	int ret=0;
	coord new={0};
	px_to_coord(mp, &new);
	if (b->squares[new.row][new.col].pc!=pc_NONE) {
		ret=1;
		*c=new;
	}
	return ret;
}

void clear_pm(move *pm, size_t *s) {
	for (size_t i=0;i<*s;i++) {
		pm[i] = (move) {
			.start=(coord){-1,-1},
			.end=(coord){-1,-1},
			.pcap=pcap_FALSE
		};
	}
	*s=0;
}

void cb_push(coord cb[2], coord *c) {
	cb[0]=cb[1];
	cb[1]=*c;
}

int is_valid_move(move *moves, size_t s, coord cb[2]) {
	int ret = 0;
	for (size_t i=0;i<s;i++) {
		if (cb[0].row==moves[i].start.row&&\
			cb[0].col==moves[i].start.col&&\
			cb[1].row==moves[i].end.row&&\
			cb[1].col==moves[i].end.col) {
			ret=1;
			goto cleanup;
		}
	}
cleanup:
	return ret;
}

int game_loop() {
	game gm = {0};
	textures tx = {0};
	mv_vtable *mvt[NUM_PT] = {
		moves_gen_p,moves_gen_n,moves_gen_b,
		moves_gen_r,moves_gen_q,moves_gen_k
	};

	InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
	SetTargetFPS(TARGET_FPS);

	gm.fg=1;
	gm.fm=0;
	gm.turn=pc_W;
	coord c = {0};

	board_init(&gm.bd);
	textures_init(&tx);

	while (!WindowShouldClose()) {
	        BeginDrawing();
	        ClearBackground(WHITE);

	        board_draw(&gm.bd, &tx);
	        if (gm.fg) {
	        	clear_pm(gm.pm, &gm.pmc);
	        	moves_gen_all(&gm.bd,gm.turn,gm.pm,&gm.pmc,
	        		(mv_vtable**)mvt);
	        	gm.fg=0;
	        }
		if (gm.fm)
		        moves_draw(gm.pm, gm.pmc, &gm.cb[0]);
		if (is_click(&gm.mp)) {
			gm.fm=!gm.fm;
			px_to_coord(&gm.mp,&c);
			cb_push(gm.cb,&c);
		}
	        if (is_valid_move(gm.pm, gm.pmc, gm.cb)) {
	        	move_make(&gm.bd, gm.cb);
	        	gm.fg=1;
	        }

	        EndDrawing();
	}
	textures_deinit(&tx);

	CloseWindow();
	return 0;
}
