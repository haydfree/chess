#include "chess.h"

#define NUM_DIRS 4
#define MAX_MOVES_ACTUAL 255
#define MAX_MOVES_POSSIBLE 64
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

typedef struct dir_t {
	i8 rank, file;
} dir_t;

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
	move_t *moves_selected, move_selected;
	square_t *square_selected, *square_hovered;
	bool active, flag_move;
} input_t;

typedef struct game_t {
	move_t moves_actual[MAX_MOVES_ACTUAL];
	move_t moves_possible[MAX_MOVES_POSSIBLE];
	move_t moves_selected[MAX_MOVES_POSSIBLE];
	board_t board;
	input_t input;
	color_t turn;
	i8 score;
	u8 score_white, score_black, counter_ma, counter_mp, counter_ms;
	bool flag_en_passant, flag_check_white, flag_check_black, 
		flag_move_gen;
} game_t;

typedef struct ctx_move_t {
	move_t *moves_possible;
	board_t *board;
	square_t *square;
	u8 *counter_mp;
} ctx_move_t;

typedef void mvt_t(ctx_move_t *);

void board_init(board_t *board) {
	i8 initial_board[RANKS][FILES] = {
		{13,11,12,14,15,12,11,13},
		{10,10,10,10,10,10,10,10},
		{-1,-1,-1,-1,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1},
		{00,00,00,00,00,00,00,00},
		{03,01,02,04,05,02,01,03}
	};

	for (u8 rank = 0; rank < RANKS; rank++) {
		for (u8 file = 0; file < FILES; file++) {
			i8 code = initial_board[rank][file];
			square_t square;
			coord_t coord;
			coord.rank = rank;
			coord.file = file;
			square.coord = coord;
			if (code == -1) {
				square.color = c_NONE;
				square.type = t_NONE;
			} else {
				square.color = code / 10;
				square.type = code % 10;
			}
			board->squares[rank][file] = square;
		}
	}
}

void board_update(game_t *game) {
	square_t *start = game->moves_actual[game->counter_ma - 1].start;
	square_t *end = game->moves_actual[game->counter_ma - 1].end;
	*end = *start;
	start->color = c_NONE;
	start->type = t_NONE;
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

void is_ob(coord_t *coord, bool *res) {
	*res = false;
	if (coord->rank >= RANKS || coord->file >= FILES)
		*res = true;
}

void board_draw(board_t *board, texture_t *textures, input_t *input) {
	DrawTexture(textures->board, 0, 0, WHITE);
	
	if (input->active) {
		coord_t c = input->square_selected->coord;
		Vector2 v = {0};
		coord_to_px(&c, &v, false);
		DrawRectangle(v.x, v.y, PX_PER_SQ, PX_PER_SQ, RED);
	}

	for (u8 rank = 0; rank < RANKS; rank++) {
		for (u8 file = 0; file < FILES; file++) {
			square_t s = board->squares[rank][file];
			if (s.color != c_NONE) {
				Vector2 v = {0};
				coord_t c = {rank, file};
				coord_to_px(&c, &v, false);
				DrawTextureV(
					textures->pieces[s.color][s.type], 
						v, WHITE);
			}
		}
	}
}

void determine_check(ctx_move_t *ctx_move) {
	square_t wk, bk;
	move_t *moves_possible = ctx_move->moves_possible;
	bool resw = false, resb = false;
	for (u8 i = 0; i < MAX_MOVES_POSSIBLE; i++) {
		is_coord_equal(&moves_possible[i].end->coord, &wk.coord, &resw);
		is_coord_equal(&moves_possible[i].end->coord, &bk.coord, &resb);
	}
}

void dirs_invert_rank(dir_t *dirs, u8 s) {
	for (u8 i = 0; i < s; i++) dirs[i].rank *= -1;
}

void moves_dir_gen(ctx_move_t *ctx_move, dir_t *dirs, 
	u8 dirs_size, u8 max_steps) {
	move_t *moves_possible = ctx_move->moves_possible;
	board_t *board = ctx_move->board;
	square_t *square = ctx_move->square;
	u8 *counter_mp = ctx_move->counter_mp;

	for (u8 d = 0; d < dirs_size; d++) {
		for (u8 step = 1; step <= max_steps; step++) {
			bool ob = false;
			u8 rank = square->coord.rank + dirs[d].rank * step;
			u8 file = square->coord.file + dirs[d].file * step;
			coord_t coord = {rank, file};
			is_ob(&coord, &ob);
			if (ob) continue;
			square_t *target = &board->squares[rank][file];
			if (target->color == square->color) break;
			move_t move;
			move.start = square;
			move.end = target;
			move.captured = false;
			if (target->color != c_NONE &&\
				target->color != square->color) {
				move.captured = true;
				moves_possible[(*counter_mp)++] = move;
				break;
			}
			moves_possible[(*counter_mp)++] = move;
		}
	}
}

void moves_diag_gen(ctx_move_t *ctx_move, u8 max_steps) {
	dir_t dirs[NUM_DIRS] = {{1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
	const u8 dir_size = 4;
	moves_dir_gen(ctx_move, dirs, dir_size, max_steps);
}

void moves_strt_gen(ctx_move_t *ctx_move, u8 max_steps) {
	dir_t dirs[NUM_DIRS] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
	const u8 dir_size = 4;
	moves_dir_gen(ctx_move, dirs, dir_size, max_steps);
}

void moves_p_gen(ctx_move_t *ctx_move) {
	dir_t dirs[NUM_DIRS] = {{1, 0}, {2, 0}, {1, 1}, {1, -1}};
	u8 starting_rank = 1;
	move_t *moves_possible = ctx_move->moves_possible;
	board_t *board = ctx_move->board;
	square_t *square = ctx_move->square;
	u8 *counter_mp = ctx_move->counter_mp;
	if (square->color == c_W) {
		starting_rank = 6;
		dirs_invert_rank(dirs, NUM_DIRS);
	}
	color_t cd0 = board->squares[square->coord.rank + dirs[0].rank]
		[square->coord.file].color;
	for (u8 d = 0; d < NUM_DIRS; d++) {
		bool ob = false;
		u8 rank = square->coord.rank + dirs[d].rank;
		u8 file = square->coord.file + dirs[d].file;
		coord_t coord = {rank, file};
		is_ob(&coord, &ob);
		if (ob) continue;
		square_t *target = &board->squares[rank][file];
		bool cap = false;
		if (d == 0 && target->color != c_NONE) continue;
		if (d == 1
			&& (cd0 != c_NONE || target->color != c_NONE
				|| square->coord.rank != starting_rank))
			continue;
		if (d == 2 || d == 3) {
			if (target->color == square->color ||\
				 target->color == c_NONE) {
				 continue;
			} else cap = true;
		}
		move_t move;
		move.start = square;
		move.end = target;
		move.captured = cap;
		moves_possible[(*counter_mp)++] = move;
			
	}
}

void moves_n_gen(ctx_move_t *ctx_move) {
	dir_t dirs[] = {{2, 1}, {1, 2}, {-2, 1}, {-1, 2},
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
			square_t *square = &ctx_move->board->squares\
				[rank][file];
			if (square->type == t_NONE\
				 || square->color == c_NONE) continue;
			ctx_move->square = square;
			mvt[square->type](ctx_move);
		}
	}
}

void move_make(game_t *game, input_t *input) {
	move_t move;
	move.start = input->square_selected;
	move.end = input->square_hovered;
	color_t target_color = move.end->color;
	move.captured = false;
	if (target_color != game->turn && target_color != c_NONE)
		move.captured = true;

	game->moves_actual[game->counter_ma++] = move;
	game->turn = !game->turn;
	game->flag_move_gen = true;
}

void moves_clear(move_t *moves, u8 *counter, u8 size) {
	*counter = 0;
	memset(moves, 0, sizeof(move_t) * size);
}

void moves_select(game_t *game, input_t *input) {
	for (u8 i = 0; i < game->counter_mp; i++) {
		if (game->moves_possible[i].start == input->square_selected) {
			game->moves_selected[game->counter_ms++] = \
				game->moves_possible[i];
		}
	}
}

void moves_draw(move_t *moves, u8 size) {
	for (u8 i = 0; i < size; i++) {
		coord_t c = moves[i].end->coord;
		Vector2 v = {0};
		coord_to_px(&c, &v, true);
		DrawCircle(v.x, v.y, 5, RED);
	}
}

void input_init(input_t *input, game_t *game) {
	input->active = false;
	input->flag_move = false;
	input->moves_selected = game->moves_selected;
	input->square_selected = NULL;
	input->square_hovered = NULL;
}

void input_update(input_t *input, game_t *game) {
	coord_t coord;
	input->mouse_pos = GetMousePosition();
	px_to_coord(&input->mouse_pos, &coord);
	input->coord = coord;
	input->square_hovered = &game->board.squares[coord.rank][coord.file];
	input->flag_move = false;
	if (!input->active) {
		input->square_selected = NULL;
	}
	if (IsMouseButtonPressed(0)) {
		if (input->active) {
			if (input->square_hovered->color == game->turn) {
				input->active = false;
				input->square_selected = NULL;
			} else {
				input->flag_move = true;
				input->active = false;
			}
		} else {
			if (input->square_hovered->color == game->turn) {
				input->active = true;
				input->square_selected = input->square_hovered;
			}
		}
	}
}

void ctx_move_init(ctx_move_t *ctx_move, game_t *game) {
	ctx_move->board = &game->board;
	ctx_move->moves_possible = game->moves_possible;
	ctx_move->counter_mp = &game->counter_mp;
}

void game_init(game_t *game) {
	board_init(&game->board);
	input_init(&game->input, game);
	game->counter_ma = 0;
	game->counter_mp = 0;
	game->counter_ms = 0;
	game->turn = c_W;
	game->score = 0;
	game->score_white = 0;
	game->score_black = 0;
	game->flag_en_passant = false;
	game->flag_check_white = false;
	game->flag_check_black = false;
	game->flag_move_gen = true;

	memset(game->moves_actual, 0, MAX_MOVES_ACTUAL*sizeof(move_t));
	memset(game->moves_possible, 0, MAX_MOVES_POSSIBLE*sizeof(move_t));
	memset(game->moves_selected, 0, MAX_MOVES_POSSIBLE*sizeof(move_t));
}

void game_loop() {
	game_t game;
	input_t input;
	ctx_move_t ctx_move;
	texture_t textures;
	mvt_t *mvt[TYPES] = {moves_p_gen, moves_n_gen, moves_b_gen, 
		moves_r_gen, moves_q_gen, moves_k_gen};

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(BOARD_WIDTH, BOARD_HEIGHT, "chess");
	SetTargetFPS(TARGET_FPS);

	game_init(&game);
	ctx_move_init(&ctx_move, &game);
	textures_init(&textures);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(WHITE);

		board_draw(&game.board, &textures, &input);
		input_update(&input, &game);

		if (input.active) {
			moves_clear(game.moves_selected, &game.counter_ms,
				MAX_MOVES_POSSIBLE);
			moves_select(&game, &input);
			moves_draw(game.moves_selected, game.counter_ms);
		}

		if (input.flag_move) {
			move_make(&game, &input);
			board_update(&game);
		}

		if (game.flag_move_gen) {
			moves_all_gen(&ctx_move, (mvt_t**) mvt);
			game.flag_move_gen = false;
		}

		EndDrawing();
	}
	textures_deinit(&textures);
	CloseWindow();
}
