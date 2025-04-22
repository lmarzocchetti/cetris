#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <raylib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#ifdef PLATFORM_WEB
#define COLS 10
#define ROWS 20
#define HIDDEN_ROWS 2
#define TOTAL_ROWS (ROWS + HIDDEN_ROWS)
#define SQUARE_SIZE 40
#define GUI_SIZE 300
#define LINE_THICKNESS 2.0f
#else
#define COLS 10
#define ROWS 20
#define HIDDEN_ROWS 2
#define TOTAL_ROWS (ROWS + HIDDEN_ROWS)
#define SQUARE_SIZE 50
#define GUI_SIZE 400
#define LINE_THICKNESS 2.0f
#endif

#define LEVEL_TIME(g) powf((0.8 - (g) * 0.007), g)
#define A_TYPE_P(level, destr_lines) ((level * 10 + 10) <= destr_lines)

#define SELECT_LEVEL(l)               \
    do {                              \
        *start_level = l;             \
        *level_delay = LEVEL_TIME(l); \
        return true;                  \
    } while (0)

#define ARRAY_LEN_INT(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))

typedef int Square[2];

typedef enum {
    T,
    J,
    Z,
    O,
    S,
    L,
    I,
    Empty
} PieceKind;

PieceKind PieceKind_get_random(void)
{
    return (PieceKind)(rand() % Empty);
}

#define ColorFromPiece(piece) (                    \
    (piece) == T ? PURPLE : (piece) == J ? BLUE    \
        : (piece) == Z                   ? RED     \
        : (piece) == O                   ? YELLOW  \
        : (piece) == S                   ? GREEN   \
        : (piece) == L                   ? ORANGE  \
        : (piece) == I                   ? SKYBLUE \
                                         : (assert(false), BLACK))

typedef struct {
    PieceKind kind;
    Square squares[4];
} Piece;

typedef enum {
    Left,
    Right
} Direction;

typedef struct {
    bool active;
    PieceKind type;
} Slot;

typedef Slot Board[TOTAL_ROWS][COLS];

typedef struct {
    Board board;
    Piece active_piece;
    Piece next_piece;
    int destroyed_lines;
    int score;
    int best_score;
    int current_level;
} Game;

int Piece_left_square(Piece* piece);
int Piece_right_square(Piece* piece);
void Piece_rotate(Piece* piece, float direction, Game* game);

Piece spawn_piece(void);

Game Game_init(int level);
void Game_reset(Game* game, int start_level);
bool Game_touch_other_square(const Game* game, Square square);
bool Game_active_piece_can_go_right(Game* game);
bool Game_active_piece_can_go_left(Game* game);
bool Game_gravity_active_piece(Game* game);
void Game_release_active_piece(Game* game);
void Game_move_active_piece(Game* game, Direction direction);
void Game_rotate_active_piece(Game* game, Direction direction);
void Game_update_score(Game* game, int lines);
int Game_delete_full_rows_if_exists(Game* game);
bool Game_check_game_over(Game* game);
void Game_draw_on_window(const Game* game, int starting_x, Shader shader, float delta_time);

void play_screen_render(
    Game* game,
    Shader* square_shader,
    bool* game_over,
    float* delta_time,
    int screen_width,
    int screen_height);

void play_screen_input(
    Game* game,
    Sound* theme,
    float* move_timer,
    float* move_delay,
    float* level_timer,
    float* level_delay,
    bool* music_paused,
    bool* is_soft_drop,
    bool* game_over,
    bool* level_selection_screen,
    int start_level);

void play_screen_logic(
    Game* game,
    Sound* theme,
    Sound* line_clear_sound,
    Sound* tetris_sound,
    Sound* next_level_sound,
    int* start_level,
    float* delta_time,
    float* level_timer,
    float* level_delay,
    float* move_timer,
    bool* is_soft_drop,
    bool* music_paused,
    bool* game_over);

bool level_selection_screen_input(int* start_level, float* level_delay);

void level_selection_screen_render(int screen_width, int screen_height);

int main(void)
{
    srand(time(NULL)); // SEED

    constexpr int screen_width = COLS * SQUARE_SIZE + GUI_SIZE;
    constexpr int screen_height = ROWS * SQUARE_SIZE;

    // Global Render various screen
    bool level_selection_screen = true;

    // TODO: Refactor variable in a struct level selection
    int start_level = 0;

    // TODO: Refactor these variables in a struct
    // START Play Screen variables
    bool game_over = false;
    bool music_paused = false;
    float move_delay = 0.2f;
    float move_timer = 0.0f;
    float level_delay = LEVEL_TIME(start_level);
    float level_timer = 0.0f;
    bool is_soft_drop = false;
    // END Play Screen variables

    InitWindow(screen_width, screen_height, "Cetris");
    SetTargetFPS(60);

    InitAudioDevice();

    Sound line_clear_sound = LoadSound("resources/music/line_clear.mp3");
    Sound tetris_sound = LoadSound("resources/music/tetris.mp3");
    Sound next_level_sound = LoadSound("resources/music/next_level.mp3");
    Sound theme = LoadSound("resources/music/b-type_theme.mp3");

#ifdef PLATFORM_WEB
    Shader square_shader = LoadShader("resources/shaders/liquid_square_vertex.glsl", "resources/shaders/liquid_square_web.glsl");
#else
    Shader square_shader = LoadShader(nullptr, "resources/shaders/liquid_square.glsl");
#endif

    Game game = Game_init(start_level);
    float delta_time = 0.0f;

    while (!WindowShouldClose()) {
        if (level_selection_screen) {
            // INPUT
            if (level_selection_screen_input(&start_level, &level_delay)) {
                level_selection_screen = false;
                game.current_level = start_level;
            }

            // RENDER
            level_selection_screen_render(screen_width, screen_height);
        } else {
            // INPUT
            play_screen_input(&game, &theme, &move_timer, &move_delay, &level_timer, &level_delay, &music_paused, &is_soft_drop, &game_over, &level_selection_screen, start_level);

            // RENDER
            play_screen_render(&game, &square_shader, &game_over, &delta_time, screen_width, screen_height);

            // LOGIC
            if (!game_over) {
                play_screen_logic(&game, &theme, &line_clear_sound, &tetris_sound, &next_level_sound, &start_level, &delta_time, &level_timer, &level_delay, &move_timer, &is_soft_drop, &music_paused, &game_over);
            }
        }
    }

    // Frees
    UnloadShader(square_shader);
    UnloadSound(theme);
    UnloadSound(next_level_sound);
    UnloadSound(tetris_sound);
    UnloadSound(line_clear_sound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

//              //
//              //
//  FUNCTIONS   //
//              //
//              //

bool level_selection_screen_input(int* start_level, float* level_delay)
{
    if (IsKeyPressed(KEY_ZERO)) {
        SELECT_LEVEL(0);
    } else if (IsKeyPressed(KEY_ONE)) {
        SELECT_LEVEL(1);
    } else if (IsKeyPressed(KEY_TWO)) {
        SELECT_LEVEL(2);
    } else if (IsKeyPressed(KEY_THREE)) {
        SELECT_LEVEL(3);
    } else if (IsKeyPressed(KEY_FOUR)) {
        SELECT_LEVEL(4);
    } else if (IsKeyPressed(KEY_FIVE)) {
        SELECT_LEVEL(5);
    } else if (IsKeyPressed(KEY_SIX)) {
        SELECT_LEVEL(6);
    } else if (IsKeyPressed(KEY_SEVEN)) {
        SELECT_LEVEL(7);
    } else if (IsKeyPressed(KEY_EIGHT)) {
        SELECT_LEVEL(8);
    } else if (IsKeyPressed(KEY_NINE)) {
        SELECT_LEVEL(9);
    }

    return false;
}

void level_selection_screen_render(int screen_width, int screen_height)
{
    BeginDrawing();
    ClearBackground((Color) { 0x1E, 0x20, 0x1E, 0xFF });

    DrawText("Select a level 0-9", ((float)screen_width / 2.0) - 110, ((float)screen_height / 2.0) - 25, 25, RAYWHITE);

    EndDrawing();
}

void play_screen_input(
    Game* game,
    Sound* theme,
    float* move_timer,
    float* move_delay,
    float* level_timer,
    float* level_delay,
    bool* music_paused,
    bool* is_soft_drop,
    bool* game_over,
    bool* level_selection_screen,
    int start_level)
{
    if (IsKeyDown(KEY_RIGHT) && *move_timer >= *move_delay) {
        Game_move_active_piece(game, Right);
        *move_timer = 0.0f;
    }
    if (IsKeyDown(KEY_LEFT) && *move_timer >= *move_delay) {
        Game_move_active_piece(game, Left);
        *move_timer = 0.0f;
    }
    if (IsKeyPressed(KEY_Z)) {
        Game_rotate_active_piece(game, Left);
    }
    if (IsKeyPressed(KEY_X)) {
        Game_rotate_active_piece(game, Right);
    }
    if (IsKeyPressed(KEY_L)) {
        *level_selection_screen = true;
        Game_reset(game, start_level);
    }
    if (IsKeyPressed(KEY_M)) {
        if (IsSoundPlaying(*theme)) {
            PauseSound(*theme);
            *music_paused = !(*music_paused);
        } else {
            ResumeSound(*theme);
            *music_paused = !(*music_paused);
        }
    }
    if (IsKeyDown(KEY_DOWN)) {
        *is_soft_drop = true;
        *level_timer += (*level_delay) / 2;
    }
    if (IsKeyReleased(KEY_DOWN)) {
        *is_soft_drop = false;
    }
    if (IsKeyPressed(KEY_R)) {
        Game_reset(game, start_level);
        *game_over = false;
    }
}

void play_screen_render(
    Game* game,
    Shader* square_shader,
    bool* game_over,
    float* delta_time,
    int screen_width,
    int screen_height)
{
    if (*game_over == false) {
        BeginDrawing();
        ClearBackground((Color) { 0x1E, 0x20, 0x1E, 0xFF });

        Game_draw_on_window(game, GUI_SIZE, *square_shader, *delta_time);

        // GUI DRAWING
        {
            DrawRectangleLinesEx((Rectangle) { 0, 0, GUI_SIZE, screen_height }, 15, (Color) { 0x3C, 0x3D, 0x37, 0xFF });

            // Score Text and value
            DrawText("Score:", 25, 50, 25, LIGHTGRAY);
            char score_as_str[50] = { 0 };
            sprintf(score_as_str, "%d", game->score);
            DrawText(score_as_str, 110, 51, 25, SKYBLUE);

            // Best Score text and value
            DrawText("Best Score:", 25, 100, 25, LIGHTGRAY);
            char best_as_str[50] = { 0 };
            sprintf(best_as_str, "%d", game->best_score);
            DrawText(best_as_str, 175, 101, 25, SKYBLUE);

            // Deleted Lines text and value
            DrawText("Del. Lines:", 25, 150, 25, LIGHTGRAY);
            char del_lines_as_str[50] = { 0 };
            sprintf(del_lines_as_str, "%d", game->destroyed_lines);
            DrawText(del_lines_as_str, 150, 151, 25, SKYBLUE);

            // Level
            DrawText("Level:", 25, 200, 25, LIGHTGRAY);
            char level_as_str[50] = { 0 };
            sprintf(level_as_str, "%d", game->current_level);
            DrawText(level_as_str, 100, 201, 25, SKYBLUE);

            // Next Piece text and new piece
            DrawText("Next Piece", GUI_SIZE / 2 - 75, 420, 25, LIGHTGRAY);
            Color next_piece_color = ColorFromPiece(game->next_piece.kind);

            for (int i = 0; i < ARRAY_LEN_INT(game->next_piece.squares); ++i) {
                float new_x = 0.0f;
                if (game->next_piece.kind == I) {
                    new_x = (float)(game->next_piece.squares[i][1] * SQUARE_SIZE - 85);
                } else if (game->next_piece.kind == O) {
                    new_x = (float)(game->next_piece.squares[i][1] * SQUARE_SIZE - 50);
                } else {
                    new_x = (float)(game->next_piece.squares[i][1] * SQUARE_SIZE - 70);
                }

                Rectangle rect = {
                    .x = new_x - 14.0,
                    .y = (float)(game->next_piece.squares[i][0] * SQUARE_SIZE + 510),
                    .width = (float)SQUARE_SIZE,
                    .height = (float)SQUARE_SIZE
                };

                BeginShaderMode(*square_shader);
                DrawRectangleRec(rect, next_piece_color);
                DrawRectangleLinesEx(rect, LINE_THICKNESS, BLACK);
                EndShaderMode();
            }
        }
        EndDrawing();
    } else {
        BeginDrawing();
        Game_draw_on_window(game, GUI_SIZE, *square_shader, *delta_time);
        DrawText("Si pers fra :(", screen_width / 4, screen_height / 3, 30, DARKBLUE);
        DrawText("Schiaccj lu tast R per continua'", screen_width / 4, screen_height / 3 + 40, 25, LIME);
        EndDrawing();
        return;
    }
}

void play_screen_logic(
    Game* game,
    Sound* theme,
    Sound* line_clear_sound,
    Sound* tetris_sound,
    Sound* next_level_sound,
    int* start_level,
    float* delta_time,
    float* level_timer,
    float* level_delay,
    float* move_timer,
    bool* is_soft_drop,
    bool* music_paused,
    bool* game_over)
{
    if (*level_timer >= *level_delay) {
        *level_timer = 0.0f;
        if (Game_gravity_active_piece(game) == true) {
            Game_release_active_piece(game);
            int deleted_rows = Game_delete_full_rows_if_exists(game);
            // SOUND
            switch (deleted_rows) {
            case 1:
            case 2:
            case 3:
                PlaySound(*line_clear_sound);
                break;
            case 4:
                PlaySound(*tetris_sound);
                break;
            }
            Game_update_score(game, deleted_rows);
            // Change level if need
            if ((game->current_level == *start_level && A_TYPE_P(*start_level, game->destroyed_lines)) || (game->current_level > *start_level && (game->destroyed_lines >= ((*start_level * 10 + 10) + (game->current_level - *start_level) * 10)))) {
                game->current_level += 1;
                *level_delay = LEVEL_TIME(game->current_level);
                PlaySound(*next_level_sound);
            }
            *game_over = Game_check_game_over(game);
        }
    }

    // Audio
    if (!IsSoundPlaying(*theme) && !(*music_paused)) {
        PlaySound(*theme);
    }
    *move_timer += GetFrameTime();
    if (!(*is_soft_drop)) {
        *level_timer += GetFrameTime();
    }
    *delta_time += GetFrameTime();
}

int Piece_left_square(Piece* piece)
{
    int min_col = 100;
    for (int i = 0; i < ARRAY_LEN_INT(piece->squares); ++i) {
        if (piece->squares[i][1] < min_col) {
            min_col = piece->squares[i][1];
        }
    }
    return min_col;
}

int Piece_right_square(Piece* piece)
{
    int max_col = 0;
    for (int i = 0; i < ARRAY_LEN_INT(piece->squares); ++i) {
        if (piece->squares[i][1] > max_col) {
            max_col = piece->squares[i][1];
        }
    }
    return max_col;
}

void Piece_rotate(Piece* piece, float direction, Game* game)
{
    const float degree = direction * PI / 2.0f;
    const Square* origin = &piece->squares[1];

    Square rotated_points[4] = { 0 };

    for (int i = 0; i < ARRAY_LEN_INT(piece->squares); ++i) {
        const float x0 = (float)piece->squares[i][0] - (float)(*origin)[0];
        const float y0 = (float)piece->squares[i][1] - (float)(*origin)[1];

        int x1 = (int)(roundf(x0 * cosf(degree) - y0 * sinf(degree)));
        int y1 = (int)(roundf(x0 * sinf(degree) + y0 * cosf(degree)));
        x1 += (float)(*origin)[0];
        y1 += (float)(*origin)[1];

        Square points = { x1, y1 };
        memcpy(rotated_points[i], points, sizeof(points));

        if (x1 < 0 || x1 >= TOTAL_ROWS || y1 < 0 || y1 >= COLS || Game_touch_other_square(game, points)) {
            return;
        }
    }

    memcpy(piece->squares, rotated_points, sizeof(rotated_points));
}

Piece spawn_piece(void)
{
    PieceKind piece_kind_to_spawn = PieceKind_get_random();
    Piece piece_to_spawn = { 0 };
    piece_to_spawn.kind = piece_kind_to_spawn;

    switch (piece_kind_to_spawn) {
    case T:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 3, 5 },
                                           { 2, 5 },
                                           { 2, 4 },
                                           { 2, 6 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case I:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 2, 4 },
                                           { 2, 5 },
                                           { 2, 6 },
                                           { 2, 7 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case J:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 3, 4 },
                                           { 3, 5 },
                                           { 3, 6 },
                                           { 2, 6 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case Z:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 2, 6 },
                                           { 2, 5 },
                                           { 3, 4 },
                                           { 3, 5 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case O:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 3, 4 },
                                           { 3, 5 },
                                           { 2, 4 },
                                           { 2, 5 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case S:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 3, 6 },
                                           { 3, 5 },
                                           { 2, 5 },
                                           { 2, 4 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case L:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 3, 4 },
                                           { 3, 5 },
                                           { 3, 6 },
                                           { 2, 4 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case Empty:
        printf("ERROR: Trying to spawn an EMPTY PIECE\n");
        assert(false);
        break;
    }

    return piece_to_spawn;
}

Game Game_init(int level)
{
    Board board = { 0 };
    for (int row = 0; row < ARRAY_LEN_INT(board); ++row) {
        for (int col = 0; col < ARRAY_LEN_INT(board[col]); ++col) {
            board[row][col].active = false;
            board[row][col].type = Empty;
        }
    }

    Game game = (Game) {
        .active_piece = spawn_piece(),
        .next_piece = spawn_piece(),
        .destroyed_lines = 0,
        .score = 0,
        .best_score = 0,
        .current_level = level
    };
    memcpy(game.board, board, sizeof(board));

    return game;
}

void Game_reset(Game* game, int start_level)
{
    Game to_copy = Game_init(start_level);
    to_copy.best_score = game->best_score;
    to_copy.score = game->score;
    memcpy(game, &to_copy, sizeof(to_copy));

    if (game->score > game->best_score) {
        game->best_score = game->score;
    }
    game->score = 0;
}

bool Game_touch_other_square(const Game* game, Square square)
{
    if (game->board[square[0]][square[1]].active == true) {
        return true;
    }
    return false;
}

bool Game_active_piece_can_go_right(Game* game)
{
    int active_right = Piece_right_square(&game->active_piece);
    if (active_right == COLS - 1) {
        return false;
    }

    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        Square next = { game->active_piece.squares[i][0], game->active_piece.squares[i][1] + 1 };
        if (Game_touch_other_square(game, next)) {
            return false;
        }
    }

    return true;
}

bool Game_active_piece_can_go_left(Game* game)
{
    int active_left = Piece_left_square(&game->active_piece);
    if (active_left == 0)
        return false;

    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        Square next = { game->active_piece.squares[i][0], game->active_piece.squares[i][1] - 1 };
        if (Game_touch_other_square(game, next)) {
            return false;
        }
    }

    return true;
}

/*
    Return true if Touched else false
*/
bool Game_gravity_active_piece(Game* game)
{
    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        Square next = { game->active_piece.squares[i][0] + 1, game->active_piece.squares[i][1] };
        if (next[0] == TOTAL_ROWS) {
            return true;
        }

        if (Game_touch_other_square(game, next)) {
            return true;
        }
    }

    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        game->active_piece.squares[i][0] += 1;
    }

    return false;
}

void Game_release_active_piece(Game* game)
{
    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        Square* curr_square = &game->active_piece.squares[i];
        memcpy(
            (void*)&game->board[(*curr_square)[0]][(*curr_square)[1]],
            &(Slot) { .active = true, .type = game->active_piece.kind },
            sizeof(Slot));
    }

    game->active_piece = game->next_piece;
    game->next_piece = spawn_piece();
}

void Game_move_active_piece(Game* game, Direction direction)
{
    switch (direction) {
    case Left:
        if (Game_active_piece_can_go_left(game) == true) {
            for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
                game->active_piece.squares[i][1] -= 1;
            }
        }
        break;
    case Right:
        if (Game_active_piece_can_go_right(game) == true) {
            for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
                game->active_piece.squares[i][1] += 1;
            }
        }
        break;
    }
}

void Game_rotate_active_piece(Game* game, Direction direction)
{
    switch (direction) {
    case Left:
        Piece_rotate(&game->active_piece, -1.0f, game);
        break;
    case Right:
        Piece_rotate(&game->active_piece, 1.0f, game);
        break;
    }
}

void Game_update_score(Game* game, int lines)
{
    int score = 0;
    switch (lines) {
    case 1:
        score = 40 * (game->current_level + 1);
        break;
    case 2:
        score = 100 * (game->current_level + 1);
        break;
    case 3:
        score = 300 * (game->current_level + 1);
        break;
    case 4:
        score = 1200 * (game->current_level + 1);
        break;
    }

    game->score += score;
}

int Game_delete_full_rows_if_exists(Game* game)
{
    int deleted_rows = 0;

    for (int row = 0; row < ARRAY_LEN_INT(game->board); ++row) {
        bool full_row = true;

        // Check if the row is full
        for (int col = 0; col < ARRAY_LEN_INT(game->board[row]); ++col) {
            if (game->board[row][col].active == false) {
                full_row = false;
                break;
            }
        }

        // Delete the row
        if (full_row == true) {
            deleted_rows += 1;

            int row_start = row;

            while (row_start > 0) {
                for (int col = 0; col < ARRAY_LEN_INT(game->board[row]); ++col) {
                    memcpy(&game->board[row_start][col], &game->board[row_start - 1][col], sizeof(Slot));
                }
                row_start -= 1;
            }
        }
    }
    game->destroyed_lines += deleted_rows;
    return deleted_rows;
}

bool Game_check_game_over(Game* game)
{
    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        if (game->board[game->active_piece.squares[i][0]][game->active_piece.squares[i][1]].active == true) {
            return true;
        }
    }

    return false;
}

void Game_draw_on_window(const Game* game, int starting_x, Shader shader, float delta_time)
{
    int shader_loc = GetShaderLocation(shader, "time");
    SetShaderValue(shader, shader_loc, &delta_time, SHADER_UNIFORM_FLOAT);

    for (int row = 0; row < ARRAY_LEN_INT(game->board); ++row) {
        if (row < HIDDEN_ROWS) {
            continue;
        }

        for (int col = 0; col < ARRAY_LEN_INT(game->board[row]); ++col) {
            const Slot* curr_square = &game->board[row][col];
            if (curr_square->active == true) {
                Rectangle to_draw = {
                    .x = (float)(col * SQUARE_SIZE + starting_x),
                    .y = (float)(row * SQUARE_SIZE - HIDDEN_ROWS * SQUARE_SIZE),
                    .width = (float)SQUARE_SIZE,
                    .height = (float)SQUARE_SIZE
                };

                BeginShaderMode(shader);

                DrawRectangleRec(to_draw, ColorFromPiece(game->board[row][col].type));

                // TODO: Maybe drag down rectangle lines ex
                DrawRectangleLinesEx(to_draw, LINE_THICKNESS, BLACK);

                EndShaderMode();
            }
        }
    }

    for (int i = 0; i < ARRAY_LEN_INT(game->active_piece.squares); ++i) {
        Rectangle rect = {
            .x = (float)(game->active_piece.squares[i][1] * SQUARE_SIZE + starting_x),
            .y = (float)(game->active_piece.squares[i][0] * SQUARE_SIZE - HIDDEN_ROWS * SQUARE_SIZE),
            .width = SQUARE_SIZE,
            .height = SQUARE_SIZE,
        };

        BeginShaderMode(shader);
        DrawRectangleRec(rect, ColorFromPiece(game->active_piece.kind));
        EndShaderMode();

        DrawRectangleLinesEx(rect, LINE_THICKNESS, BLACK);
    }
}
