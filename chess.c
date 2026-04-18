#include "chess.h"

#define MAX_MOVE_MOVES 20
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
	move moves[MAX_GAME_MOVES];
	move p_moves[MAX_MOVE_MOVES];
	board board;
	piece_color turn;
	int score;
	size_t mc, p_mc;
} game;
typedef struct textures {
	Texture2D pieces[NUM_PC][NUM_PT];
	Texture2D board;
} textures;

typedef void mv_vtable(board*,piece_color,move[MAX_MOVE_MOVES],size_t*);

void board_init(board *b) {
square initial_b[8][8]={
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
	if (a->row>=NUM_ROW||a->row<0||a->col>=NUM_COL||a->col<0) return 1;
	return 0;
}

void flip_deltas(coord *d, size_t ds) {
	for (size_t i=0;i<ds;i++) {
		coord new={0};
		new.row=d[i].row*-1;
		new.col=d[i].col*-1;
		d[i]=new;
	}
}

void moves_gen_p(board *b, piece_color pc, move *mbuf, size_t *mc) {
	const size_t ds=4;
	coord deltas[]={{1,-1},{1,0},{1,1},{2,0}};
	if (pc==pc_W) flip_deltas(deltas,ds);

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square s=b->squares[row][col];
			if (s.pc!=pc) continue;
			for (size_t d=0;d<ds;d++) {
				coord c={0};
				piece_captured pcap = pcap_FALSE;
				coord_add(&c,&deltas[d]);
				if (is_ob(&c)) continue;
				if (b->squares[c.row][c.col].pc!=pc_NONE)
					pcap = pcap_TRUE;
				mbuf[*mc++]= (move) {
					.start=(coord){row,col},
					.end=c,
					.pcap=pcap
				};
			}
		}
	}
}
void moves_gen_n(board *b, piece_color pc, move *mbuf, size_t *mc) {
	(void)b;(void)pc;(void)mbuf;(void)mc;
}
void moves_gen_b(board *b, piece_color pc, move *mbuf, size_t *mc) {
	(void)b;(void)pc;(void)mbuf;(void)mc;
}
void moves_gen_r(board *b, piece_color pc, move *mbuf, size_t *mc) {
	(void)b;(void)pc;(void)mbuf;(void)mc;
}
void moves_gen_q(board *b, piece_color pc, move *mbuf, size_t *mc) {
	(void)b;(void)pc;(void)mbuf;(void)mc;
}
void moves_gen_k(board *b, piece_color pc, move *mbuf, size_t *mc) {
	(void)b;(void)pc;(void)mbuf;(void)mc;
}
void moves_gen_all(board *b,piece_color pc,move mbuf[MAX_MOVE_MOVES],
	size_t *mc,mv_vtable **mvt) {
	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square *s=&b->squares[row][col];
			if (s->pt!=pt_NONE&&s->pc==pc) 
				mvt[s->pt](b,pc,mbuf,mc);
		}
	}
}

void clear_square(square *s) {
	s->pc=pc_NONE;
	s->pt=pt_NONE;
}

void move_make(board *b, move *m) {
	square *start=&b->squares[m->start.row][m->start.col];
	square *end=&b->squares[m->end.row][m->end.col];
	*end=*start;
	clear_square(start);
}

void moves_draw(move *moves, size_t s, coord *c) {
	for (size_t i=0;i<s;i++) {
		if (moves[i].start.row!=c->row||moves[i].start.col!=c->col)
			continue;
		Vector2 v={0};
		coord_to_px(&moves[i].start,&v,1);
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

int is_valid_move(move *m, move *moves, size_t s) {
	int ret = 0;

	for (size_t i=0;i<s;i++) {
		if (m->start.row==moves[i].start.row \
			&& m->start.col==moves[i].start.col \
			&& m->end.row==moves[i].end.row \
			&& m->end.col==moves[i].end.col) {
				ret = 1;
				goto cleanup;
			}
	}
cleanup:
	return ret;
}

void clear_mbuf(move *mbuf, size_t s) {
	for (size_t i=0;i<s;i++) {
		mbuf[i] = (move) {
			.start=(coord){-1,-1},
			.end=(coord){-1,-1},
			.pcap=pcap_FALSE
		};
	}
}

int game_loop() {
	board bd = {0};
	game gm = {0};
	textures tx = {0};
	mv_vtable *mvt[NUM_PT] = {
		moves_gen_p,moves_gen_n,moves_gen_b,
		moves_gen_r,moves_gen_q,moves_gen_k
	};

	InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
	SetTargetFPS(TARGET_FPS);

	int flag_gen = 1;
	Vector2 mp = {0};
	coord c = {0};
	gm.turn=pc_W;

	board_init(&bd);
	textures_init(&tx);

	while (!WindowShouldClose()) {
	        BeginDrawing();
	        ClearBackground(WHITE);

	        board_draw(&bd, &tx);
	        if (flag_gen) {
	        	clear_mbuf(gm.p_moves, gm.p_mc);
	        	moves_gen_all(&bd,gm.turn,gm.p_moves,&gm.p_mc,
	        		(mv_vtable**)mvt);
	        	flag_gen=0;
	        }

	        if (is_click(&mp)) {
	        	px_to_coord(&mp, &c);
		        moves_draw(gm.p_moves, gm.p_mc, &c);
	        }

	        EndDrawing();
	}
	textures_deinit(&tx);

	CloseWindow();
	return 0;
}
