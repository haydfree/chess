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

typedef struct piece_t {
	color_t color;
	type_t type;
} piece_t;

typedef struct board_t {
	piece_t pieces[RANKS][FILES];
} board_t;

typedef struct coord_t {
	u8 rank, file;
} coord_t;

typedef struct dir_t {
	i8 rank, file;
} dir_t;

typedef struct move_t {
	coord_t start, end;
	bool captured;
} move_t;

typedef struct texture_t {
	Texture2D pieces[COLORS][TYPES];
	Texture2D board;
} texture_t;

typedef struct input_t {
	Vector2 mouse_pos;
	coord_t coord_hovered, coord_selected;
	move_t *moves_selected, *move_selected;
	bool active, flag_move;
} input_t;

typedef struct ctx_move_t {
	move_t *moves_possible;
	board_t *board;
	piece_t *piece;
	coord_t coord;
	u8 *counter_mp;
} ctx_move_t;

typedef struct game_t {
	move_t moves_actual[MAX_MOVES_ACTUAL];
	move_t moves_possible[MAX_MOVES_POSSIBLE];
	move_t moves_selected[MAX_MOVES_POSSIBLE];
	texture_t textures;
	ctx_move_t ctx_move;
	board_t board;
	input_t input;
	color_t turn;
	i8 score;
	u8 score_white, score_black, counter_ma, counter_mp, counter_ms;
	bool flag_en_passant, flag_check_white, flag_check_black, 
		flag_move_gen;
} game_t;

typedef void mvt_t(ctx_move_t *);

void board_init(game_t *game) {
	board_t *board = &game->board;
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
			piece_t piece;
			if (code == -1) {
				piece.color = c_NONE;
				piece.type = t_NONE;
			} else {
				piece.color = code / 10;
				piece.type = code % 10;
			}
			board->pieces[rank][file] = piece;
		}
	}
}

void textures_init(game_t *game) {
	texture_t *textures = &game->textures;
	for (u8 color = c_W; color < COLORS; color++) {
		for (u8 type = t_P; type < TYPES; type++) {
			textures->pieces[color][type] = LoadTexture(TextFormat(
				"%s/%d%d.png", PATH_TX, color, type));
		}
	}
	textures->board = LoadTexture(TextFormat("%s/board.png", PATH_TX));
}

void textures_deinit(game_t *game) {
	texture_t *textures = &game->textures;
	for (u8 color = c_W; color < COLORS; color++) {
		for (u8 type = t_P; type < TYPES; type++) {
			UnloadTexture(textures->pieces[color][type]);
		}
	}
	UnloadTexture(textures->board);
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

void board_draw(game_t *game) {
	texture_t *textures = &game->textures;
	input_t *input = &game->input;
	board_t *board = &game->board;
	DrawTexture(textures->board, 0, 0, WHITE);
	
	if (input->active) {
		coord_t c = input->coord_selected;
		Vector2 v = {0};
		coord_to_px(&c, &v, false);
		DrawRectangle(v.x, v.y, PX_PER_SQ, PX_PER_SQ, RED);
	}

	for (u8 rank = 0; rank < RANKS; rank++) {
		for (u8 file = 0; file < FILES; file++) {
			piece_t piece = board->pieces[rank][file];
			if (piece.color != c_NONE) {
				Vector2 v = {0};
				coord_t c = {rank, file};
				coord_to_px(&c, &v, false);
				DrawTextureV(
					textures->pieces[piece.color]\
					[piece.type], v, WHITE);
			}
		}
	}
}

void determine_check(ctx_move_t *ctx_move) {
	coord_t wk, bk;
	move_t *moves_possible = ctx_move->moves_possible;
	bool resw = false, resb = false;
	for (u8 i = 0; i < MAX_MOVES_POSSIBLE; i++) {
		is_coord_equal(&moves_possible[i].end, &wk, &resw);
		is_coord_equal(&moves_possible[i].end, &bk, &resb);
	}
}

void dirs_invert_rank(dir_t *dirs, u8 s) {
	for (u8 i = 0; i < s; i++) dirs[i].rank *= -1;
}

void moves_dir_gen(ctx_move_t *ctx_move, dir_t *dirs, 
	u8 dirs_size, u8 max_steps) {
	move_t *moves_possible = ctx_move->moves_possible;
	board_t *board = ctx_move->board;
	piece_t *piece = ctx_move->piece;
	coord_t coord = ctx_move->coord;
	u8 *counter_mp = ctx_move->counter_mp;

	for (u8 d = 0; d < dirs_size; d++) {
		for (u8 step = 1; step <= max_steps; step++) {
			bool ob = false;
			u8 rank = coord.rank + dirs[d].rank * step;
			u8 file = coord.file + dirs[d].file * step;
			coord_t target_coord = {rank, file};
			is_ob(&target_coord, &ob);
			if (ob) continue;
			piece_t *target_piece = &board->pieces[rank][file];
			if (target_piece->color == piece->color) break;
			move_t move;
			move.start = coord;
			move.end = target_coord;
			move.captured = false;
			if (target_piece->color != c_NONE &&\
				target_piece->color != piece->color) {
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
	piece_t *piece = ctx_move->piece;
	coord_t coord = ctx_move->coord;
	u8 *counter_mp = ctx_move->counter_mp;
	if (piece->color == c_W) {
		starting_rank = 6;
		dirs_invert_rank(dirs, NUM_DIRS);
	}
	color_t cd0 = board->pieces[coord.rank + dirs[0].rank]\
		[coord.file].color;
	for (u8 d = 0; d < NUM_DIRS; d++) {
		bool ob = false;
		u8 rank = coord.rank + dirs[d].rank;
		u8 file = coord.file + dirs[d].file;
		coord_t target_coord = {rank, file};
		is_ob(&target_coord, &ob);
		if (ob) continue;
		piece_t *target_piece = &board->pieces[rank][file];
		bool cap = false;
		if (d == 0 && target_piece->color != c_NONE) continue;
		if (d == 1
			&& (cd0 != c_NONE || target_piece->color != c_NONE
				|| coord.rank != starting_rank))
			continue;
		if (d == 2 || d == 3) {
			if (target_piece->color == piece->color ||\
				 target_piece->color == c_NONE) {
				 continue;
			} else cap = true;
		}
		move_t move;
		move.start = coord;
		move.end = target_coord;
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
			piece_t *piece = &ctx_move->board->pieces[rank][file];
			if (piece->type == t_NONE\
				 || piece->color == c_NONE) continue;
			ctx_move->piece = piece;
			ctx_move->coord.rank = rank;
			ctx_move->coord.file = file;
			mvt[piece->type](ctx_move);
		}
	}
}

void move_make(game_t *game) {
	move_t *move = game->input.move_selected;

	game->board.pieces[move->end.rank][move->end.file] = *game->ctx_move.piece;
	game->board.pieces[move->start.rank][move->start.file].color = c_NONE;
	game->board.pieces[move->start.rank][move->start.file].type = t_NONE;
	game->moves_actual[game->counter_ma++] = *move;
	game->turn = !game->turn;
	game->flag_move_gen = true;
}

void moves_clear(move_t *moves, u8 *counter, u8 size) {
	*counter = 0;
	memset(moves, SENTINEL, sizeof(move_t) * size);
}

void moves_select(game_t *game) {
	input_t *input = &game->input;
	for (u8 i = 0; i < game->counter_mp; i++) {
		bool res = false;
		is_coord_equal(&game->moves_possible[i].start, 
			&input->coord_selected, &res);
		if (res) {
			game->moves_selected[game->counter_ms++] = \
				game->moves_possible[i];
		}
	}
}

void moves_draw(move_t *moves, u8 size) {
	for (u8 i = 0; i < size; i++) {
		coord_t c = moves[i].end;
		Vector2 v = {0};
		coord_to_px(&c, &v, true);
		DrawCircle(v.x, v.y, 5, BLACK);
	}
}

void move_select(game_t *game) {
	input_t *input = &game->input;
	for (u8 i = 0; i < MAX_MOVES_POSSIBLE; i++) {
		move_t *m = &game->moves_possible[i];
		bool start_equal = false, end_equal = false;
		is_coord_equal(&m->start, &input->coord_selected,
			&start_equal);
		is_coord_equal(&m->end, &input->coord_hovered, &end_equal);
		if (start_equal && end_equal) {
			input->move_selected = m;
			return;
		}
	}
}

void input_init(game_t *game) {
	input_t *input = &game->input;

	input->active = false;
	input->flag_move = false;
	input->moves_selected = game->moves_selected;
	input->move_selected = NULL;
	memset(&input->coord_selected, SENTINEL, sizeof(coord_t));
	memset(&input->coord_hovered, SENTINEL, sizeof(coord_t));
}

void input_update(game_t *game) {
	input_t *input = &game->input;
	input->mouse_pos = GetMousePosition();
	px_to_coord(&input->mouse_pos, &input->coord_hovered);
	color_t color = game->board.pieces[input->coord_hovered.rank]\
		[input->coord_hovered.file].color;
	input->flag_move = false;
	if (!input->active) {
		memset(&input->coord_selected, SENTINEL, sizeof(coord_t));
	}
	if (IsMouseButtonPressed(0)) {
		if (input->active) {
			if (color == game->turn) {
				input->active = false;
				memset(&input->coord_selected,
					SENTINEL, sizeof(coord_t));
			} else {
				input->flag_move = true;
				move_select(game);
				input->active = false;
			}
		} else {
			if (color == game->turn) {
				input->active = true;
				input->coord_selected = input->coord_hovered;
			}
		}
	}
}

void ctx_move_init(game_t *game) {
	ctx_move_t *ctx_move = &game->ctx_move;
	ctx_move->board = &game->board;
	ctx_move->moves_possible = game->moves_possible;
	ctx_move->counter_mp = &game->counter_mp;
}

void ctx_move_update(game_t *game) {
	game->ctx_move.coord = game->input.coord_selected;
	game->ctx_move.piece = &game->board.pieces[game->ctx_move.coord.rank]\
		[game->ctx_move.coord.file];
}

void game_init(game_t *game) {
	board_init(game);
	input_init(game);
	ctx_move_init(game);
	textures_init(game);
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

	memset(game->moves_actual, SENTINEL,
		MAX_MOVES_ACTUAL*sizeof(move_t));
	memset(game->moves_possible, SENTINEL, 
		MAX_MOVES_POSSIBLE*sizeof(move_t));
	memset(game->moves_selected, SENTINEL,
		MAX_MOVES_POSSIBLE*sizeof(move_t));
}

void game_loop() {
	game_t game;
	mvt_t *mvt[TYPES] = {moves_p_gen, moves_n_gen, moves_b_gen, 
		moves_r_gen, moves_q_gen, moves_k_gen};

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(BOARD_WIDTH, BOARD_HEIGHT, "chess");
	SetTargetFPS(TARGET_FPS);

	game_init(&game);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(WHITE);

		board_draw(&game);
		input_update(&game);

		if (game.input.active) {
			moves_clear(game.moves_selected, &game.counter_ms,
				MAX_MOVES_POSSIBLE);
			moves_select(&game);
			moves_draw(game.moves_selected, game.counter_ms);
			DEBUG("mp size: %u, ms size: %u", game.counter_mp, game.counter_ms);
		}

		if (game.input.flag_move) {
			ctx_move_update(&game);
			move_make(&game);
		}

		if (game.flag_move_gen) {
			moves_clear(game.moves_possible, &game.counter_mp,
				MAX_MOVES_POSSIBLE);
			moves_all_gen(&game.ctx_move, (mvt_t**) mvt);
			game.flag_move_gen = false;
		}

		EndDrawing();
	}
	textures_deinit(&game);
	CloseWindow();
}
