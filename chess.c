#include "chess.h"

#define NUM_DIRS 4
#define MAX_MOVES_ACTUAL 255
#define MAX_MOVES_POSSIBLE 32
#define RANKS 8
#define FILES 8
#define COLORS 2
#define TYPES 6
#define TARGET_FPS 20
#define BOARD_WIDTH 600
#define BOARD_HEIGHT BOARD_WIDTH
#define PX_PER_SQ (BOARD_WIDTH / FILES)
#define PATH_TX "assets"

typedef enum color_t { c_NONE = -1, c_W, c_B } color_t;

typedef enum type_t { t_NONE = -1, t_P, t_N, t_B, t_R, t_Q, t_K } type_t;

typedef struct coord_t {
	u8 rank, file;
} coord_t;

typedef struct square_t {
	color_t color;
	type_t type;
	coord_t coord;
} square_t;

typedef struct board_t {
	square_t squares[RANKS][FILES];
} board_t;

typedef struct move_t {
	square_t *start, *end;
	bool captured;
} move_t;

typedef struct texture_t {
	Texture2D pieces[COLORS][TYPES];
	Texture2D board;
} texture_t;

typedef struct input_t {
	Vector2 mouse_pos;
	coord_t coord;
	move_t *moves_selected;
	bool active;
} input_t;

typedef struct game_t {
	move_t moves_actual[MAX_MOVES_ACTUAL];
	move_t moves_possible[MAX_MOVES_POSSIBLE];
	move_t moves_selected[MAX_MOVES_POSSIBLE];
	board_t board;
	input_t input;
	u16 counter_ma, counter_mp, counter_ms;
	color_t turn;
	i8 score;
	u8 score_white, score_black;
	bool flag_en_passant, flag_check_white, flag_check_black, 
		flag_move_gen;
} game_t;

typedef struct ctx_move_t {
	move_t *moves_possible;
	board_t *board;
	square_t *square;
	u16 *counter_mp;
} ctx_move_t;

typedef void mvt_t(ctx_move_t *);

void board_init(board_t *b) {
	(void)b;
}

void textures_init(texture_t *t) {
	for (u8 color = c_W; color < COLORS; color++) {
		for (u8 type = t_P; type < TYPES; type++) {
			t->pieces[color][type] = LoadTexture(TextFormat(
				"%s/%d%d.png", PATH_TX, color, type));
		}
	}
	t->board = LoadTexture(TextFormat("%s/board.png", PATH_TX));
}

void textures_deinit(texture_t *t) {
	for (u8 color = c_W; color < COLORS; color++) {
		for (u8 type = t_P; type < TYPES; type++) {
			UnloadTexture(t->pieces[color][type]);
		}
	}
	UnloadTexture(t->board);
}

void px_to_coord(Vector2 *v, coord_t *c) {
	coord_t new = {0};
	new.rank = v->y / PX_PER_SQ;
	new.file = v->x / PX_PER_SQ;
	*c = new;
}

void coord_to_px(coord_t *c, Vector2 *v, bool centered) {
	Vector2 new = {c->file * PX_PER_SQ, c->rank * PX_PER_SQ};
	if (centered) {
		new.x += PX_PER_SQ / 2;
		new.y += PX_PER_SQ / 2;
	}
	*v = new;
}

void coord_add(coord_t *a, coord_t *b) {
	coord_t new = {0};
	new.rank = a->rank + b->rank;
	new.file = a->file + b->file;
	*a = new;
}

void is_coord_equal(coord_t *a, coord_t *b, bool *res) {
	*res = a->rank == b->rank && a->file == b->file;
}

void is_ob(square_t *square, bool *res) {
	*res = false;
	coord_t c = square->coord;
	if (c.rank >= RANKS || c.file >= FILES)
		*res = true;
}

void board_draw(board_t *b, texture_t *t) {
	DrawTexture(t->board, 0, 0, WHITE);

	for (u8 rank = 0; rank < RANKS; rank++) {
		for (u8 file = 0; file < FILES; file++) {
			square_t s = b->squares[rank][file];
			if (s.color != c_NONE) {
				Vector2 v = {0};
				coord_t c = {rank, file};
				coord_to_px(&c, &v, false);
				DrawTextureV(
					t->pieces[s.color][s.type], v, WHITE);
			}
		}
	}
}

// TOOD: stop early if found
void determine_check(ctx_move_t *ctx_move) {
	square_t wk, bk;
	move_t *moves_possible = ctx_move->moves_possible;
	for (u8 rank = 0; rank < RANKS; rank++) {
		for (u8 file = 0; file < FILES; file++) {
			square_t square = ctx_move->board->squares[rank][file];
			if (square.color == c_W && square.type == t_K)
				wk = square;
			if (square.color == c_B && square.type == t_K)
				bk = square;
		}
	}
	bool resw = false, resb = false;
	for (u8 i = 0; i < MAX_MOVES_POSSIBLE; i++) {
		is_coord_equal(&moves_possible[i].end->coord, &wk.coord, &resw);
		is_coord_equal(&moves_possible[i].end->coord, &bk.coord, &resb);
	}
}

void dirs_invert_rank(coord_t *dirs, u8 s) {
	for (u8 i = 0; i < s; i++) dirs[i].rank *= -1;
}

void moves_dir_gen(ctx_move_t *ctx_move, coord_t *dirs, 
	u8 dirs_size, u8 max_steps) {
	move_t *moves_possible = ctx_move->moves_possible;
	board_t *board = ctx_move->board;
	square_t *square = ctx_move->square;
	u16 *counter_mp = ctx_move->counter_mp;

	for (u8 d = 0; d < dirs_size; d++) {
		for (u8 step = 1; step <= max_steps; step++) {
			bool ob = false;
			square_t target = board->squares\
				[square->coord.rank + dirs[d].rank * step]\
				[square->coord.file + dirs[d].file * step];
			is_ob(&target, &ob);
			if (ob) continue;
			if (target.color == square->color) break;
			move_t move;
			move.start = square;
			move.end = &target;
			if (target.color != c_NONE &&\
				target.color != square->color) {
				move.captured = true;
				moves_possible[(*counter_mp)++] = move;
				break;
			}
			move.captured = false;
			moves_possible[(*counter_mp)++] = move;
		}
	}
}

void moves_diag_gen(ctx_move_t *ctx_move, u8 max_steps) {
	coord_t dirs[NUM_DIRS] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
	const u8 dir_size = 4;
	moves_dir_gen(ctx_move, dirs, dir_size, max_steps);
}

void moves_strt_gen(ctx_move_t *ctx_move, u8 max_steps) {
	coord_t dirs[NUM_DIRS] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
	const u8 dir_size = 4;
	moves_dir_gen(ctx_move, dirs, dir_size, max_steps);
}

void moves_p_gen(ctx_move_t *ctx_move) {
	coord_t dirs[NUM_DIRS] = {
		(coord_t) {1, 0}, (coord_t) {2, 0},
		(coord_t) {1, 1}, (coord_t) {1, -1}
	};
	u8 starting_rank = 1;
	move_t *moves_possible = ctx_move->moves_possible;
	board_t *board = ctx_move->board;
	square_t *square = ctx_move->square;
	u16 *counter_mp = ctx_move->counter_mp;
	if (square->color == c_W) {
		starting_rank = 6;
		dirs_invert_rank(dirs, NUM_DIRS);
	}
	color_t cd0 = board->squares[square->coord.rank + dirs[0].rank]
		[square->coord.file].color;
	for (int d = 0; d < NUM_DIRS; d++) {
		square_t target = board->squares\
			[square->coord.rank + dirs[d].rank]\
			[square->coord.file + dirs[d].file];
		bool cap = false;
		if (d == 0 && target.color != c_NONE) continue;
		if (d == 1
			&& (cd0 != c_NONE || target.color != c_NONE
				|| square->coord.rank != starting_rank))
			continue;
		if (d == 2 || d == 3) {
			if (target.color == square->color ||\
				 target.color == c_NONE) {
				 continue;
			} else {
				cap = true;
			}
		}
		move_t move;
		move.start = square;
		move.end = &target;
		move.captured = cap;
		moves_possible[(*counter_mp)++] = move;
			
	}
}

void moves_n_gen(ctx_move_t *ctx_move) {
	coord_t dirs[] = {{2, 1}, {1, 2}, {-2, 1}, {-1, 2},
		{2, -1}, {1, -2}, {-2, -1}, {-1, -2}};
	const u8 dir_size = 8;
	const u8 max_steps = 1;
	moves_dir_gen(ctx_move, dirs, dir_size, max_steps);
}

void moves_b_gen(ctx_move_t *ctx_move) {
	const u8 max_steps = RANKS-1;
	moves_diag_gen(ctx_move, max_steps);
}

void moves_r_gen(ctx_move_t *ctx_move) {
	const u8 max_steps = RANKS-1;
	moves_strt_gen(ctx_move, max_steps);
}

void moves_q_gen(ctx_move_t *ctx_move) {
	const size_t max_steps = RANKS-1;
	moves_strt_gen(ctx_move, max_steps);
	moves_diag_gen(ctx_move, max_steps);
}

void moves_k_gen(ctx_move_t *ctx_move) {
	const u8 max_steps = 1;
	moves_strt_gen(ctx_move, max_steps);
	moves_diag_gen(ctx_move, max_steps);
}

void moves_all_gen(ctx_move_t *ctx_move, mvt_t **mvt) {
	for (u8 rank = 0; rank < RANKS; rank++) {
		for (u8 file = 0; file < FILES; file++) {
			square_t square = ctx_move->board->squares\
				[rank][file];
			if (square.type == t_NONE\
				 || square.color == c_NONE) continue;
			ctx_move->square = &square;
			mvt[square.type](ctx_move);
		}
	}
}

void game_loop() {
	game_t game;
	texture_t textures;
	mvt_t *mvt[TYPES] = {moves_p_gen, moves_n_gen, moves_b_gen, 
		moves_r_gen, moves_q_gen, moves_k_gen};

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(BOARD_WIDTH, BOARD_HEIGHT, "chess");
	SetTargetFPS(TARGET_FPS);

	board_init(&game.board);
	textures_init(&textures);

	ctx_move_t ctx_move;
	game.flag_move_gen = true;

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(WHITE);

		if (game.flag_move_gen) {
			moves_all_gen(&ctx_move, (mvt_t**) mvt);
		}

		board_draw(&game.board, &textures);

		EndDrawing();
	}
	textures_deinit(&textures);
	CloseWindow();
}
