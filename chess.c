#include "chess.h"

#define NUM_DIRS 4
#define MAX_MOVE_MOVES 256
#define MAX_GAME_MOVES 1024
#define TARGET_FPS 20
#define WIDTH_BOARD 600
#define HEIGHT_BOARD WIDTH_BOARD
#define NUM_COL 8
#define NUM_ROW 8
#define NUM_COLORS 2
#define NUM_TYPES 6
#define PX_PER_SQ (WIDTH_BOARD / NUM_COL)
#define PATH_TX "assets"

typedef enum color_t {c_NONE=-1,c_W,c_B} color_t;
typedef enum type_t {t_NONE=-1,t_P,t_N,t_B,t_R,t_Q,t_K} type_t;
typedef enum cap_t {cap_FALSE, cap_TRUE} cap_t;
typedef struct square_t {color_t color; type_t type;} square_t;
typedef struct board_t {square_t squares[NUM_ROW][NUM_COL];} board_t;
typedef struct coord_t {int row, col;} coord_t;
typedef struct move_t {coord_t start, end;cap_t cap;} move_t;
typedef struct texture_t {Texture2D pieces[NUM_COLORS][NUM_TYPES];Texture2D board;} texture_t;
typedef struct game_t {
	move_t moves[MAX_GAME_MOVES];
	move_t pmoves[MAX_MOVE_MOVES];
	board_t board;
	color_t turn;
	int score, flag_g;
	coord_t sel[2];
	size_t mc, pmc;
	Vector2 mp;
	int flag_ep;
} game_t;
typedef struct context_move {
	board_t *b;
	square_t *s;
	coord_t *c;
	move_t *pm;
	size_t *pmc;
	int *flag_ep;
} context_move;
typedef void mvt_t(context_move*);

void board_init(board_t *b) {
	square_t initial_b[NUM_ROW][NUM_COL]={
	{(square_t){c_B,t_R},(square_t){c_B,t_N},(square_t){c_B,t_B},(square_t){c_B,t_Q},(square_t){c_B,t_K},(square_t){c_B,t_B},(square_t){c_B,t_N},(square_t){c_B,t_R}},{(square_t){c_B,t_P},(square_t){c_B,t_P},(square_t){c_B,t_P},(square_t){c_B,t_P},(square_t){c_B,t_P},(square_t){c_B,t_P},(square_t){c_B,t_P},(square_t){c_B,t_P}},{(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE}},{(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE}},{(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE}},{(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE},(square_t){c_NONE,t_NONE}},{(square_t){c_W,t_P},(square_t){c_W,t_P},(square_t){c_W,t_P},(square_t){c_W,t_P},(square_t){c_W,t_P},(square_t){c_W,t_P},(square_t){c_W,t_P},(square_t){c_W,t_P}},{(square_t){c_W,t_R},(square_t){c_W,t_N},(square_t){c_W,t_B},(square_t){c_W,t_Q},(square_t){c_W,t_K},(square_t){c_W,t_B},(square_t){c_W,t_N},(square_t){c_W,t_R}}};

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			b->squares[row][col]=initial_b[row][col];
		}
	}
}

void textures_init(texture_t *t) {
	for (int color=c_W;color<NUM_COLORS;color++) {
		for (int type=t_P;type<NUM_TYPES;type++) {
			t->pieces[color][type]=LoadTexture(
				TextFormat("%s/%d%d.png",PATH_TX,color,type));
		}
	}
    	t->board = LoadTexture(TextFormat("%s/board.png",PATH_TX));
}

void textures_deinit(texture_t *t) {
	for (int color=c_W;color<NUM_COLORS;color++) {
		for (int type=t_P;type<NUM_TYPES;type++) {
			UnloadTexture(t->pieces[color][type]);
		}
	}
	UnloadTexture(t->board);
}

void px_to_coord(Vector2 *v, coord_t *c) {
	coord_t new = {0};
	new.row=v->y/PX_PER_SQ;
	new.col=v->x/PX_PER_SQ;
	*c=new;
}

void coord_to_px(coord_t *c, Vector2 *v, int centered) {
	Vector2 new = {c->col*PX_PER_SQ,c->row*PX_PER_SQ};
	if (centered) {
		new.x+=PX_PER_SQ/2;
		new.y+=PX_PER_SQ/2;
	}
	*v=new;
}

void board_draw(board_t *b, texture_t *t) {
	DrawTexture(t->board,0,0,WHITE);

	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square_t *s=&b->squares[row][col];
		        if (s->color!=c_NONE) {
		        	coord_t c={row,col};
			        Vector2 v={0};
			        coord_to_px(&c,&v,0);
		        	DrawTextureV(t->pieces[s->color][s->type], v, WHITE);
		        }
		}
	}
}

void coord_add(coord_t *a, coord_t *b) {
	coord_t new = {0};
	new.row = a->row+b->row;
	new.col = a->col+b->col;
	*a=new;
}

int is_coord_equal(coord_t *a, coord_t *b) {
	return a->row==b->row&&a->col==b->col;
}

int is_ob(coord_t *a) {
	int ret = 0;
	if (a->row>=NUM_ROW||a->row<0||a->col>=NUM_COL||a->col<0) ret=1;
	return ret;
}

void dirs_invert_row(coord_t *dirs, size_t s) {
	for (size_t i=0;i<s;i++) dirs[i].row*=-1;
}

void dirs_invert_col(coord_t *dirs, size_t s) {
	for (size_t i=0;i<s;i++) dirs[i].col*=-1;
}

void moves_p_gen(context_move *cm) {
	int starting_row=1;
	coord_t dirs[NUM_DIRS]={(coord_t){1,0},(coord_t){2,0},(coord_t){1,1},(coord_t){1,-1}};
	board_t *b=cm->b;
	square_t *s=cm->s;
	coord_t *c=cm->c;
	move_t *pm=cm->pm;
	size_t *pmc=cm->pmc;
	if (s->color==c_W) {
		starting_row=6;
		dirs_invert_row(dirs, NUM_DIRS);
	}
	color_t cd0=b->squares[c->row+dirs[0].row][c->col].color;
	for (int d=0;d<NUM_DIRS;d++) {
		cap_t cap = cap_FALSE;
		coord_t tc=(coord_t){c->row,c->col};
		coord_add(&tc,&dirs[d]);
		color_t tcc=b->squares[tc.row][tc.col].color;
		if (d==0&&tcc!=c_NONE) continue;
		if (d==1&&(cd0!=c_NONE||tcc!=c_NONE||c->row!=starting_row)) continue;
		if ((d==2||d==3)&&(tcc==s->color||tcc==c_NONE)) continue;
		pm[(*pmc)++]= (move_t) {
			.start=(coord_t){c->row,c->col},
			.end=tc,
			.cap=cap
		};
	}
}

void moves_dir_gen(context_move *cm, coord_t *dirs, size_t ds, size_t step_size) {
	board_t *b=cm->b;
	square_t *s=cm->s;
	coord_t *c=cm->c;
	move_t *pm=cm->pm;
	size_t *pmc=cm->pmc;

	for (size_t d=0;d<ds;d++) {
		for (size_t step=1;step<=step_size;step++) {
			coord_t tc=(coord_t){c->row,c->col};
			coord_t dir=(coord_t){dirs[d].row*step,dirs[d].col*step};
			coord_add(&tc,&dir);
			move_t move=(move_t) {*c,tc,cap_FALSE};
			if (is_ob(&tc)) continue;
			square_t ts=b->squares[tc.row][tc.col];
			if (ts.color==s->color) break;
			if (ts.color!=c_NONE&&ts.color!=s->color) {
				move.cap=cap_TRUE;
				pm[(*pmc)++]=move;
				break;
			}
			pm[(*pmc)++]=move;
		}
	}
}

void moves_n_gen(context_move *cm) {
	const size_t ds=8;
	const size_t step_size=1;
	coord_t dirs[]={{2,1},{1,2},{-2,1},{-1,2},{2,-1},{1,-2},{-2,-1},{-1,-2}};
	moves_dir_gen(cm, dirs, ds, step_size);
}

void moves_b_gen(context_move *cm) {
	const size_t ds=4;
	const size_t step_size=NUM_ROW;
	coord_t dirs[]={{1,1},{1,-1},{-1,1},{-1,-1}};
	moves_dir_gen(cm, dirs, ds, step_size);
}

void moves_r_gen(context_move *cm) {
	const size_t ds=4;
	const size_t step_size=NUM_ROW;
	coord_t dirs[]={{1,0},{-1,0},{0,1},{0,-1}};
	moves_dir_gen(cm, dirs, ds, step_size);
}

void moves_q_gen(context_move *cm) {
	moves_b_gen(cm);
	moves_r_gen(cm);
}

void moves_k_gen(context_move *cm) {
	(void)cm;
}

void moves_all_gen(context_move *cm, mvt_t **mvt) {
	for (int row=0;row<NUM_ROW;row++) {
		for (int col=0;col<NUM_COL;col++) {
			square_t *s=&cm->b->squares[row][col];
			coord_t c=(coord_t){row,col};
			if (s->type==t_NONE||s->color==c_NONE) continue;
			cm->s=s;cm->c=&c;
			mvt[s->type](cm);
		}
	}
}

void clear_square(square_t *s) {
	s->color=c_NONE;
	s->type=t_NONE;
}

void move_make(board_t *b, move_t *pm, size_t pmi, move_t *m,size_t *mc) {
	move_t move=pm[pmi];
	square_t *start=&b->squares[move.start.row][move.start.col];
	square_t *end=&b->squares[move.end.row][move.end.col];
	*end=*start;
	clear_square(start);
	m[(*mc++)]=move;
}

void moves_draw(move_t *pmoves, size_t s, coord_t *c) {
	for (size_t i=0;i<s;i++) {
		if (pmoves[i].start.row!=c->row||pmoves[i].start.col!=c->col)
			continue;
		Vector2 v={0};
		coord_to_px(&pmoves[i].end,&v,1);
		DrawCircleV(v, 5.0, GRAY);
	}
}

int is_click(Vector2 *mp, coord_t *c) {
	int ret = 0;
	if (IsMouseButtonPressed(0)) {
		*mp = GetMousePosition();
		px_to_coord(mp,c);
		ret = 1;
	}
	return ret;
}

int is_click_piece(board_t *b, coord_t *c) {
	int ret=0;
	if (b->squares[c->row][c->col].color!=c_NONE) ret=1;
	return ret;
}

void pmoves_clear(move_t *pmoves, size_t *s) {
	for (size_t i=0;i<*s;i++) {
		pmoves[i] = (move_t) {
			.start=(coord_t){-1,-1},
			.end=(coord_t){-1,-1},
			.cap=cap_FALSE
		};
	}
	*s=0;
}

void sel_clear(coord_t sel[2]) {
	coord_t c={-1,-1};
	sel[0]=c;
	sel[1]=c;
}

int is_sel_empty(coord_t sel[2]) {
	int ret=0;
	coord_t c={-1,-1};
	if (is_coord_equal(&sel[0],&c)&&is_coord_equal(&sel[1],&c)) ret=1;
	return ret;
}

int is_sel_full(coord_t sel[2]) {
	int ret=0;
	coord_t c={-1,-1};
	if (!is_coord_equal(&sel[0],&c)&&!is_coord_equal(&sel[1],&c)) ret=1;
	return ret;
}

void sel_push(coord_t sel[2], coord_t *c) {
	sel[0]=sel[1];
	sel[1]=*c;
}

int is_valid_move(move_t *moves, size_t s, coord_t sel[2], size_t *idx) {
	int ret = 0;
	for (size_t i=0;i<s;i++) {
		if (is_coord_equal(&moves[i].start,&sel[0])&&\
			is_coord_equal(&moves[i].end,&sel[1])) {
			*idx=i;
			ret=1;
			goto cleanup;
		}
	}
cleanup:
	return ret;
}

void context_move_init(context_move *cm,board_t *b,move_t *pm, size_t *pmc) {
	cm->b=b;
	cm->pm=pm;
	cm->pmc=pmc;
} 

int game_loop() {
	game_t game;
	texture_t textures;
	context_move cm;
	coord_t c;
	size_t valid_move_idx;
	mvt_t *mvt[NUM_TYPES] = {
		moves_p_gen,moves_n_gen,moves_b_gen,
		moves_r_gen,moves_q_gen,moves_k_gen
	};

	InitWindow(WIDTH_BOARD, HEIGHT_BOARD, "chess");
	SetTargetFPS(TARGET_FPS);

	game.flag_g=1;
	game.turn=c_W;
	game.pmc=0;game.mc=0;
	sel_clear(game.sel);

	board_init(&game.board);
	textures_init(&textures);
	context_move_init(&cm,&game.board,game.pmoves,&game.pmc);

	while (!WindowShouldClose()) {
	        BeginDrawing();
	        ClearBackground(WHITE);

		int sel_empty=is_sel_empty(game.sel);
		int sel_full=is_sel_full(game.sel);
		int clicked=is_click(&game.mp, &c);
		int piece_clicked=is_click_piece(&game.board,&c);
		int valid_move=is_valid_move(game.pmoves,game.pmc,game.sel,&valid_move_idx);

	        board_draw(&game.board, &textures);
	        if (game.flag_g) {
	        	pmoves_clear(game.pmoves, &game.pmc);
	        	moves_all_gen(&cm,(mvt_t**)mvt);
	        	game.flag_g=0;
	        }
	        if (clicked)
	        	sel_push(game.sel, &c);
	        if (sel_full) {
	        	if (valid_move) {
	        		move_make(&game.board, game.pmoves, valid_move_idx, game.moves,&game.mc);
	        		game.flag_g=1;
	        		game.turn=game.turn==c_W?c_B:c_W;
	        	}
	        	sel_clear(game.sel);
	        }
	        if (!sel_empty&&!sel_full&&piece_clicked)
	        	moves_draw(game.pmoves,game.pmc,&c);
	        EndDrawing();
	}
	textures_deinit(&textures);

	CloseWindow();
	return 0;
}
