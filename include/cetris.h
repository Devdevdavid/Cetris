//
//  cetris.h
//  Cetris
//
//  Created by David Devant on 13/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#ifndef CETRIS_H
#define CETRIS_H

#define VERSION               "v1.7.0 - October 2019 - Marvel Version"
#define LEVEL_MAX             10
#define MIN_BOARD_ROW         6
#define MIN_BOARD_COLUMN      4
#define NB_TETRI_COMMING      2
#define DROP_SPEED            8
#define POS_GUIDE_LVL_MAX     2

#define TERM_WIDTH            80
#define TERM_HEIGHT           24
#ifndef BOARD_WIDTH
	#define BOARD_WIDTH       8
#endif
#ifndef BOARD_HEIGHT
	#define BOARD_HEIGHT      20
#endif
#define BOARD_TABLE_SIZE      (BOARD_WIDTH * BOARD_HEIGHT)
#define PATH_MAX_LEN          201
#define ALT_BACKSPACE         127
#define ALT_ENTER             10
#define LOGIN_LEN             12
#define NB_MENU_ENTRIES       5
#define NB_TETRI              7

#define COLI_SUPER            0x01
#define COLI_UNDER            0x02
#define COLI_HORIZ_BORDER     0x04
#define COLI_VERTI_BORDER     0x08
#define COLI_BORDER           (COLI_HORIZ_BORDER | COLI_VERTI_BORDER)
#define COLI_ALL              0x0F

#define TITLE_LOGIN           0
#define TITLE_CLASSIC         1
#define TITLE_B_MODE          2
#define TITLE_NO_WALL         3
#define TITLE_QUIT            4

#define SCORE_CLEANER         3000
#define SCORE_PER_LEVEL       400
#define SCORE_COMPLETE_ROW    100
#define SCORE_WRITE_TETRI     5
#define SCORE_DROP            1

#define AI_SCORE_COMPL        +0.760666
#define AI_SCORE_AGGRE        -0.510066
#define AI_SCORE_HOLES        -0.356630
#define AI_SCORE_BUMPI        -0.484483

#define CONF_FILE_NAME        "cetris.conf"
#define LOG_FILE_NAME         "cetris.log"

// In seconds
#define BONUS_DELAY_MULTIPLY  10

#define NB_TIMEOUT            3
#define ID_TIMEOUT_MULTIPLY   0
#define ID_TIMEOUT_CLEANER    1
#define ID_TIMEOUT_SPEED      2

#define FREE                  0
#define DISABLED              0
#define ON_BOARD              0
#define MOVE_LATER            0
#define MOVE_NOW              1
#define LAST_TETRI            NB_TETRI_COMMING - 1

// Data Pipe
#define MAX_PIPE_NAME_LEN 	  32

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <curses.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "log.h"

struct block_t {
  char color;                   // Color is independant of shape
  uint8_t shape;                // Use 6 bit only
  char w;
  char h;
  char name;
};

struct tetri_t {
  uint8_t shape[6];
  uint8_t x;
  uint8_t y;
  uint8_t w;
  uint8_t h;
  uint8_t div_clk;              // For falling speed
  bool is_dropping;
  char name;
};

struct timer_t {
  time_t timeout[NB_TIMEOUT];   // Scheduled event
  time_t start;                 // Time from last pause
  time_t chrono;                // Game time
  bool is_running;
};

struct topten_t {
  char     login[LOGIN_LEN];
  uint32_t score;
  uint32_t tetri;
};

struct stats_t {
  struct topten_t topten[10];
  uint8_t  top_position;        // User's position in TOP 10
  uint32_t score;
  uint32_t tetri_placed;
  uint32_t tetri_placed_old;    // Old value to compute placement speed
  uint16_t speed;
  uint8_t  level;
  uint8_t  multiply;
  bool     cleaner_msg_enable; // Flag for cleanner bonus message
  char     conf_path[PATH_MAX_LEN];
};

struct board_t {
  uint8_t w;
  uint8_t h;
  bool no_wall_mode;
  uint8_t table[BOARD_TABLE_SIZE];
};

struct display_t {
  WINDOW *start_win;
  WINDOW *board;
  WINDOW *next_tetri;
  WINDOW *stats;
  WINDOW *hotkey;
  WINDOW *topten;
  WINDOW *status;
  WINDOW *goal_msg;
  uint8_t is_goal_msg_visible;
  uint8_t select;               // Menu selection
  char login[LOGIN_LEN];
};

struct node_t {
  uint8_t   x;
  uint8_t   y;
  uint8_t   rotate;
  uint8_t   complete_line;
};

struct branch_t {
  struct node_t nodes[NB_TETRI_COMMING];
  uint8_t   holes;              // number of holes
  uint8_t   complete_lines;     // 4 max
  uint16_t  aggregate_height;   // Sum of all column height
  uint16_t  bumpiness;          // Sum of height differences
  uint8_t   col_height[20];
  double    score;
};

struct ai_brain_t {
  struct branch_t branch;
  struct node_t best_first_node;
  struct board_t board[NB_TETRI_COMMING];
  double best_branch_score;
  uint16_t cur_tetri;
  uint16_t nb_passage;
  bool enable;
  bool high_speed;
};

struct goal_t {
	uint8_t event; /* 0: Goal disabled, > 0: Event ID */
	uint32_t score;
	bool isScoreReached;
	uint8_t level;
	bool isLevelReached;
};

struct data_pipe_t {
	char pipeName[MAX_PIPE_NAME_LEN];
	int fifoFd;
	uint8_t tableCpy[BOARD_TABLE_SIZE];				/* Copy of board->table with the tetri in it */
};

struct game_t {
  struct stats_t stats;
  struct timer_t timer;
  struct tetri_t tetri_comming[NB_TETRI_COMMING];
  struct block_t blocks[NB_TETRI];
  struct board_t board;
  struct display_t display;
  struct ai_brain_t ai_brain;
  struct goal_t goal;
  struct data_pipe_t data_pipe;
  char cetris_path[PATH_MAX_LEN];
  bool pause;
  bool in_game;
  bool ended;
  bool quit;
  uint64_t cur_time;
};

/* ====================== *
 *         MAIN           *
 * ====================== */
void game_sleep(void);
void update(struct game_t *);
void place_tetri(struct game_t *);
void render(struct game_t *);
void check_goal(struct game_t *);

/* ====================== *
 *      CONFIG_FILE       *
 * ====================== */
void init_file(struct game_t *);
void init_log(struct game_t *);
void save_stats(struct game_t *);
void read_file_stats(struct stats_t *);
void write_file_stats(struct stats_t *);
void get_cetris_path(struct game_t *, const char * first_arg);
void get_label(char buffer[], char label[]);
char * get_string(char buffer[]);
uint32_t get_value(char buffer[]);
uint16_t get_file_len(char path[]);

/* ====================== *
 *         STATS          *
 * ====================== */
void init_stats(struct stats_t *);
void set_score(struct game_t *, uint32_t new_score);
void add_tetri_placed(struct stats_t *);
void add_score(struct game_t *, uint16_t points);
void level_up(struct game_t *);
void set_cleaner_bonus(struct game_t *);
void set_multiply(struct game_t *, uint8_t multiply, uint16_t duration);
void update_topten(struct game_t *);
void bubble_sorting(struct topten_t *);
void swap_topten(struct topten_t *t1, struct topten_t *t2);
void update_speed_counter(struct game_t *);

/* ====================== *
 *         TIMER          *
 * ====================== */
void init_timer(struct timer_t *);
void start_timer(struct timer_t *);
void stop_timer(struct timer_t *);
struct tm* get_timer(struct timer_t *);
void set_timeout(struct timer_t *, uint8_t id, uint16_t duration);
void timeout_callback(struct game_t *);

/* ====================== *
 *     BOARD ANALYSE      *
 * ====================== */
void init_block(struct block_t *);
void init_board(struct board_t *);
void erase_line(struct board_t *, uint8_t y);
void check_complete_lines(struct game_t *);
void write_tetri(struct board_t *, struct tetri_t *);
void erase_tetri(struct board_t *, struct tetri_t *);
void add_row_board(struct game_t *, uint8_t row);
void remove_row_board(struct game_t *, uint8_t row);
void add_column_board(struct game_t *, uint8_t column);
void remove_column_board(struct game_t *, uint8_t column);
void random_fill_board(struct game_t *);
bool check_end_game(struct game_t *);
void clear_board(struct board_t *);
bool is_line_free(struct board_t *, uint8_t y);
bool is_line_complete(struct board_t *, uint8_t y);
void swap_line(struct board_t *, uint8_t y1, uint8_t y2);
void copy_tab(uint8_t *tab_dest, uint8_t *tab_src, uint16_t length);
bool board_size_is_valid(uint8_t w, uint8_t h);


/* ====================== *
 *       TETRIMINO        *
 * ====================== */
void init_tetri(struct game_t *);
uint8_t rotate90_tetri(struct board_t *, struct tetri_t *);
void move_tetri_left(struct board_t *, struct tetri_t *);
void move_tetri_right(struct board_t *, struct tetri_t *);
void move_tetri_down(struct board_t *, struct tetri_t *);
bool test_fall_speed(struct game_t *);
bool test_drop_speed(struct game_t *);
bool check_tetri_colision(struct board_t *, struct tetri_t *,  uint8_t filter);
void drop_tetri(struct board_t *, struct tetri_t *);
void set_drop_flag_tetri(struct tetri_t *);
void dropping_tetri(struct game_t *);
void use_next_tetri(struct game_t*);

// Internal Functions
void swap_u8(uint8_t *a, uint8_t *b);
void transpose_shape(struct tetri_t *);
void reverse_column(struct tetri_t *);
void cpy_block_to_tetri(struct tetri_t *, struct block_t *);
void choose_next_tetri(struct game_t*);

/* ====================== *
 *       USER_CMD         *
 * ====================== */
void init(struct game_t *);
void reset(struct game_t *);
void start(struct game_t *);
void stop(struct game_t *);
void quit(struct game_t *);
void return_menu(struct game_t *);
void check_user_input(struct game_t *);

// Internal Functions
void check_user_input_start(struct game_t *);
void check_user_input_game(struct game_t *);
void edit_login(struct game_t *, uint16_t key);
void process_choice(struct game_t *);
void check_login(struct display_t *);
void toggle_pause(struct game_t *);
void to_upper(char s[]);

/* ====================== *
 *        DISPLAY         *
 * ====================== */
bool init_display(struct game_t *);
void update_windows_position(struct game_t *);
void clear_display(struct display_t *);
void print_start_win(struct display_t *);
void print_play_menu(struct display_t *);
void print_game_win(struct game_t *);

// Window print
void print_board(struct game_t *);
void print_tetri(struct game_t *);
void print_position_guide(struct game_t *);
void print_next_tetri(struct game_t *);
void print_stats(struct game_t *);
void print_hotkey(struct display_t *);
void print_topten(struct game_t *);
void print_status(struct game_t *);
void print_goal_msg(struct game_t *);
void apply_prints(struct game_t *);

// Internal Functions
void init_display_color(void);
void print_pixel(WINDOW *, uint8_t color, uint8_t x, uint8_t y);
void print_box(WINDOW *, char title[]);
char * format_score(uint32_t score, char buffer[]);
char * format_time(struct tm timeinfo, char *buffer);
char * add_spaces_after_login(char src[], char dest[]);

/* ====================== *
 *       AI_PLAYER        *
 * ====================== */
void ai_init(struct ai_brain_t *);
void toggle_ai(struct game_t *game);
void toggle_high_speed(struct game_t *);
void ai_move(struct game_t *);

// Internals Functions
uint16_t ai_compute_node(struct game_t *);
void ai_get_holes(struct branch_t *, struct board_t *);
void ai_get_column_height(struct branch_t *, struct board_t *);
void ai_compute_score(struct branch_t *);
void improve_speed(struct tetri_t *, uint8_t *r_index);
uint8_t remove_complete_line(struct board_t *);
void apply_node(struct game_t *, struct node_t *);
void log_best_score(struct ai_brain_t *);
void log_ia_boards(struct ai_brain_t *);

/* ====================== *
 *       DATA_PIPE        *
 * ====================== */
int init_data_pipe(struct data_pipe_t *data_pipe);
int data_pipe_close(struct data_pipe_t *data_pipe);
int data_pipe_write(struct game_t *game);

#endif /* CETRIS_H */







