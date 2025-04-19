#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <raylib.h>

// #define COLS 10
// #define ROWS 20
// #define HIDDEN_ROWS 2
// #define TOTAL_ROWS (ROWS + HIDDEN_ROWS)
// #define SQUARE_SIZE 40
// #define GUI_SIZE 300
// #define LINE_THICKNESS 2.0f

#define COLS 10
#define ROWS 20
#define HIDDEN_ROWS 2
#define TOTAL_ROWS (ROWS + HIDDEN_ROWS)
#define SQUARE_SIZE 50
#define GUI_SIZE 400
#define LINE_THICKNESS 2.0f

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

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
} Game;

int Piece_left_square(Piece* piece);
int Piece_right_square(Piece* piece);
void Piece_rotate(Piece* piece, float direction, Game* game);

Piece spawn_piece(void);

Game Game_init(void);
void Game_reset(Game* game);
bool Game_touch_other_square(const Game* game, Square square);
bool Game_active_piece_can_go_right(Game* game);
bool Game_active_piece_can_go_left(Game* game);
bool Game_gravity_active_piece(Game* game);
void Game_release_active_piece(Game* game);
void Game_move_active_piece(Game* game, Direction direction);
void Game_rotate_active_piece(Game* game, Direction direction);
void Game_update_score(Game* game, int lines, int level);
int Game_delete_full_rows_if_exists(Game* game);
bool Game_check_game_over(Game* game);
void Game_draw_on_window(const Game* game, int starting_x, Shader shader, float delta_time);

int main(void)
{
    srand(time(NULL)); // SEED

    constexpr int screen_width = COLS * SQUARE_SIZE + GUI_SIZE;
    constexpr int screen_height = ROWS * SQUARE_SIZE;
    constexpr int level = 7;
    constexpr int level_delta = (10 - level) * 5;

    bool game_over = false;
    bool music_paused = false;
    float move_delay = 0.2f;
    float move_timer = 0.0f;

    InitWindow(screen_width, screen_height, "Cetris");
    SetTargetFPS(60);

    InitAudioDevice();

    Sound line_clear_sound = LoadSound("resources/music/line_clear.mp3");
    Sound tetris_sound = LoadSound("resources/music/tetris.mp3");
    Sound theme = LoadSound("resources/music/theme_a_drill.ogg");
    PlaySound(theme);

    Shader square_shader = LoadShader(nullptr, "resources/shaders/liquid_square2.glsl");

    Game game = Game_init();
    float delta_time = 0.0f;
    int gravity_wait = level_delta;

    while (!WindowShouldClose()) {
        gravity_wait -= 1;
        move_timer += GetFrameTime();

        // Audio
        if (!IsSoundPlaying(theme) && !music_paused)
            PlaySound(theme);
        if (IsKeyDown(KEY_RIGHT) && move_timer >= move_delay) {
            Game_move_active_piece(&game, Right);
            move_timer = 0.0f;
        }
        if (IsKeyDown(KEY_LEFT) && move_timer >= move_delay) {
            Game_move_active_piece(&game, Left);
            move_timer = 0.0f;
        }
        if (IsKeyPressed(KEY_Z))
            Game_rotate_active_piece(&game, Left);
        if (IsKeyPressed(KEY_X))
            Game_rotate_active_piece(&game, Right);
        if (IsKeyPressed(KEY_M)) {
            if (IsSoundPlaying(theme)) {
                PauseSound(theme);
                music_paused = !music_paused;
            } else {
                ResumeSound(theme);
                music_paused = !music_paused;
            }
        }
        if (IsKeyDown(KEY_DOWN)) {
            if (gravity_wait > 1) {
                gravity_wait -= 2;
            }
        }
        if (IsKeyPressed(KEY_R)) {
            Game_reset(&game);
            game_over = false;
        }

        if (gravity_wait == 0) {
            gravity_wait = level_delta;
            if (Game_gravity_active_piece(&game) == true) {
                Game_release_active_piece(&game);
                int deleted_rows = Game_delete_full_rows_if_exists(&game);
                // SOUND
                switch (deleted_rows) {
                case 1:
                case 2:
                case 3:
                    PlaySound(line_clear_sound);
                    break;
                case 4:
                    PlaySound(tetris_sound);
                    break;
                }
                Game_update_score(&game, deleted_rows, level);
                game_over = Game_check_game_over(&game);
            }
        }

        BeginDrawing();
        if (game_over == false) {
            // ClearBackground(RAYWHITE);
            ClearBackground((Color) { 0x1E, 0x20, 0x1E, 0xFF });

            Game_draw_on_window(&game, GUI_SIZE, square_shader, delta_time);

            // GUI DRAWING
            {
                DrawRectangleLinesEx((Rectangle) { 0, 0, GUI_SIZE, screen_height }, 15, (Color) { 0x3C, 0x3D, 0x37, 0xFF });

                // Score Text and value
                DrawText("Score:", 25, 50, 25, LIGHTGRAY);
                char score_as_str[50] = { 0 };
                sprintf(score_as_str, "%d", game.score);
                DrawText(score_as_str, 110, 51, 25, SKYBLUE);

                // Best Score text and value
                DrawText("Best Score:", 25, 100, 25, LIGHTGRAY);
                char best_as_str[50] = { 0 };
                sprintf(best_as_str, "%d", game.best_score);
                DrawText(best_as_str, 175, 101, 25, SKYBLUE);

                // Deleted Lines text and value
                DrawText("Del. Lines:", 25, 150, 25, LIGHTGRAY);
                char del_lines_as_str[50] = { 0 };
                sprintf(del_lines_as_str, "%d", game.destroyed_lines);
                DrawText(del_lines_as_str, 150, 151, 25, SKYBLUE);

                // Level
                DrawText("Level:", 25, 200, 25, LIGHTGRAY);
                char level_as_str[50] = { 0 };
                sprintf(level_as_str, "%d", level);
                DrawText(level_as_str, 100, 201, 25, SKYBLUE);

                // Next Piece text and new piece
                DrawText("Next Piece", GUI_SIZE / 2 - 75, 420, 25, LIGHTGRAY);
                Color next_piece_color = ColorFromPiece(game.next_piece.kind);

                for (int i = 0; i < ARRAY_LEN(game.next_piece.squares); ++i) {
                    float new_x = 0.0f;
                    if (game.next_piece.kind == I) {
                        new_x = (float)(game.next_piece.squares[i][1] * SQUARE_SIZE - 85);
                    } else if (game.next_piece.kind == O) {
                        new_x = (float)(game.next_piece.squares[i][1] * SQUARE_SIZE - 50);
                    } else {
                        new_x = (float)(game.next_piece.squares[i][1] * SQUARE_SIZE - 70);
                    }

                    Rectangle rect = {
                        .x = new_x,
                        .y = (float)(game.next_piece.squares[i][0] * SQUARE_SIZE + 510),
                        .width = (float)SQUARE_SIZE,
                        .height = (float)SQUARE_SIZE
                    };

                    BeginShaderMode(square_shader);
                    DrawRectangleRec(rect, next_piece_color);
                    DrawRectangleLinesEx(rect, LINE_THICKNESS, BLACK);
                    EndShaderMode();
                }
            }
        } else {
            BeginDrawing();
            DrawText("Si pers fra :(", screen_width / 4, screen_height / 3, 30, DARKBLUE);
            DrawText("Schiaccj lu tast R per continua'", screen_width / 4, screen_height / 3 + 40, 25, LIME);
            EndDrawing();
            continue;
        }

        EndDrawing();
        delta_time += GetFrameTime();
    }

    // Frees
    UnloadShader(square_shader);
    UnloadSound(theme);
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
int Piece_left_square(Piece* piece)
{
    int min_col = 100;
    for (int i = 0; i < ARRAY_LEN(piece->squares); ++i) {
        if (piece->squares[i][1] < min_col) {
            min_col = piece->squares[i][1];
        }
    }
    return min_col;
}

int Piece_right_square(Piece* piece)
{
    int max_col = 0;
    for (int i = 0; i < ARRAY_LEN(piece->squares); ++i) {
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

    for (int i = 0; i < ARRAY_LEN(piece->squares); ++i) {
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
                                           { 1, 5 },
                                           { 0, 5 },
                                           { 0, 4 },
                                           { 0, 6 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case I:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 1, 4 },
                                           { 1, 5 },
                                           { 1, 6 },
                                           { 1, 7 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case J:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 1, 4 },
                                           { 1, 5 },
                                           { 1, 6 },
                                           { 0, 6 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case Z:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 0, 6 },
                                           { 0, 5 },
                                           { 1, 4 },
                                           { 1, 5 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case O:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 1, 4 },
                                           { 1, 5 },
                                           { 0, 4 },
                                           { 0, 5 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case S:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 1, 6 },
                                           { 1, 5 },
                                           { 0, 5 },
                                           { 0, 4 },
                                       },
            sizeof(piece_to_spawn.squares));
        break;
    case L:
        memcpy(piece_to_spawn.squares, (Square[4]) {
                                           { 1, 4 },
                                           { 1, 5 },
                                           { 1, 6 },
                                           { 0, 4 },
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

Game Game_init(void)
{
    Board board = { 0 };
    for (int row = 0; row < ARRAY_LEN(board); ++row) {
        for (int col = 0; col < ARRAY_LEN(board[col]); ++col) {
            board[row][col].active = false;
            board[row][col].type = Empty;
        }
    }

    Game game = (Game) {
        .active_piece = spawn_piece(),
        .next_piece = spawn_piece(),
        .destroyed_lines = 0,
        .score = 0,
        .best_score = 0
    };
    memcpy(game.board, board, sizeof(board));

    return game;
}

void Game_reset(Game* game)
{
    Game to_copy = Game_init();
    to_copy.best_score = game->best_score;
    to_copy.score = game->score;
    memcpy(game, &to_copy, sizeof(to_copy));

    if (game->score > game->best_score) {
        game->best_score = game->score;
    }
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

    for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
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

    for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
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
    for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
        Square next = { game->active_piece.squares[i][0] + 1, game->active_piece.squares[i][1] };
        if (next[0] == TOTAL_ROWS) {
            return true;
        }

        if (Game_touch_other_square(game, next)) {
            return true;
        }
    }

    for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
        game->active_piece.squares[i][0] += 1;
    }

    return false;
}

void Game_release_active_piece(Game* game)
{
    for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
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
            for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
                game->active_piece.squares[i][1] -= 1;
            }
        }
        break;
    case Right:
        if (Game_active_piece_can_go_right(game) == true) {
            for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
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

void Game_update_score(Game* game, int lines, int level)
{
    int score = 0;
    switch (lines) {
    case 1:
        score = 40 * (level + 1);
        break;
    case 2:
        score = 100 * (level + 1);
        break;
    case 3:
        score = 300 * (level + 1);
        break;
    case 4:
        score = 1200 * (level + 1);
        break;
    }

    game->score += score;
}

int Game_delete_full_rows_if_exists(Game* game)
{
    int deleted_rows = 0;

    for (int row = 0; row < ARRAY_LEN(game->board); ++row) {
        bool full_row = true;

        // Check if the row is full
        for (int col = 0; col < ARRAY_LEN(game->board[row]); ++col) {
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
                for (int col = 0; col < ARRAY_LEN(game->board[row]); ++col) {
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
    for (int i = 0; i < ARRAY_LEN(game->board[0]); ++i) {
        if (game->board[0][i].active == true) {
            return true;
        }
    }

    return false;
}

void Game_draw_on_window(const Game* game, int starting_x, Shader shader, float delta_time)
{
    int shader_loc = GetShaderLocation(shader, "time");
    SetShaderValue(shader, shader_loc, &delta_time, SHADER_UNIFORM_FLOAT);

    for (int row = 0; row < ARRAY_LEN(game->board); ++row) {
        if (row < HIDDEN_ROWS) {
            continue;
        }

        for (int col = 0; col < ARRAY_LEN(game->board[row]); ++col) {
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

    for (int i = 0; i < ARRAY_LEN(game->active_piece.squares); ++i) {
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
