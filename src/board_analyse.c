//
//  board_analyse.c
//  Cetris
//
//  Created by David Devant on 14/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

void init_block(struct block_t *block)
{
  // Tetri I
  block[0].color = COLOR_CYAN;
  block[0].shape = 0x3C;
  block[0].w = 4;
  block[0].h = 1;
  block[0].name = 'I';
  // Tetri O
  block[1].color = COLOR_YELLOW;
  block[1].shape = 0x3C;
  block[1].w = 2;
  block[1].h = 2;
  block[1].name = 'O';
  // Tetri T
  block[2].color = COLOR_MAGENTA;
  block[2].shape = 0x3A;
  block[2].w = 3;
  block[2].h = 2;
  block[2].name = 'T';
  // Tetri L
  block[3].color = COLOR_WHITE;
  block[3].shape = 0x3C;
  block[3].w = 3;
  block[3].h = 2;
  block[3].name = 'L';
  // Tetri J
  block[4].color = COLOR_BLUE;
  block[4].shape = 0x39;
  block[4].w = 3;
  block[4].h = 2;
  block[4].name = 'J';
  // Tetri Z
  block[5].color = COLOR_RED;
  block[5].shape = 0x33;
  block[5].w = 3;
  block[5].h = 2;
  block[5].name = 'Z';
  // Tetri S
  block[6].color = COLOR_GREEN;
  block[6].shape = 0x1E;
  block[6].w = 3;
  block[6].h = 2;
  block[6].name = 'S';
}

void init_board(struct board_t *board)
{
  board->h = BOARD_HEIGHT;
  board->w = BOARD_WIDTH;
  clear_board(board);
  board->no_wall_mode = 0;
}

void erase_line(struct board_t *board, uint8_t y)
{
  uint8_t x;
  
  for (x = 0; x < board->w; x++) {
    board->table[y * board->w + x] = 0;
  }

  while (y > 0) {
    swap_line(board, y, y - 1);
    y--;
  }
}

uint8_t remove_complete_line(struct board_t *board)
{
  uint8_t y, cnt = 0;
  
  for (y = 0; y < board->h; y++) {
    if (is_line_complete(board, y)) {
      erase_line(board, y);
      ++cnt;
    }
  }
  return cnt;
}

void check_complete_lines(struct game_t *game)
{
  uint8_t count;
  
  count = remove_complete_line(&game->board);
  add_score(game, SCORE_COMPLETE_ROW * count);
  
  if (count > 0) {
    if (is_line_free(&game->board, game->board.h - 1)) {
      set_cleaner_bonus(game);
    }
  }
  if (count > 1) {
    set_multiply(game, count, BONUS_DELAY_MULTIPLY);
  }
}

/* Write tetri in board
 * "[...] % board->w;" -> no_wall_mode
 */
void write_tetri(struct board_t *board, struct tetri_t *tetri)
{
  uint16_t index;
  uint8_t y, x;
  
  for (y = 0; y < tetri->h; y++) {
    for (x = 0; x < tetri->w; x++) {
      if (tetri->shape[y * tetri->w + x] != FREE) {
        index = (tetri->y + y) * board->w + (tetri->x + x) % board->w;
        board->table[index] = tetri->shape[y * tetri->w + x];
      }
    }
  }
}

void erase_tetri(struct board_t *board, struct tetri_t *tetri)
{
  uint16_t index;
  uint8_t y, x;
  
  for (y = 0; y < tetri->h; y++) {
    for (x = 0; x < tetri->w; x++) {
      if (tetri->shape[y * tetri->w + x] != FREE) {
        index = (tetri->y + y) * board->w + (tetri->x + x) % board->w;
        board->table[index] = FREE;
      }
    }
  }
}

void add_row_board(struct game_t *game, uint8_t row)
{
  struct board_t *board = &game->board; // Shortcut
  int16_t y; // Negative needed
  uint8_t x;
  
  if (!board_size_is_valid(board->w, board->h + 1) || (row > board->h)) {
    return;
  }
  
  y = game->board.h + 1;
  while (y >= row) {
    for (x = 0; x < board->w; x++) {
      if (y == row) {
        board->table[y * board->w + x] = 0;
      } else {
        board->table[y * board->w + x] = board->table[(y - 1) * board->w + x];
      }
    }
    y--;
  }
  ++board->h;
}

void remove_row_board(struct game_t *game, uint8_t row)
{
  struct board_t *board = &game->board; // Shortcut
  int16_t y; // Negative needed
  uint8_t x;
  
  if ((row >= board->h) || (board->h <= MIN_BOARD_ROW)) {
    return;
  }
  
  y = row;
  while (y < game->board.h) {
    for (x = 0; x < board->w; x++) {
      board->table[y * board->w + x] = board->table[(y + 1) * board->w + x];
    }
    y++;
  }
  --board->h;
}

void add_column_board(struct game_t *game, uint8_t column)
{
  struct board_t *board = &game->board; // Shortcut
  uint8_t tab[BOARD_TABLE_SIZE] = {0};
  uint8_t x = 0, y;
  
  if (!board_size_is_valid(board->w + 1, board->h) || (column > board->w)) {
    return;
  }
  
  memcpy(tab, board->table, board->w * board->h);
  while (x <= board->w) {
    for (y = 0; y < board->h; y++) {
      if (x < column) {
        board->table[y * (board->w + 1) + x] = tab[y * board->w + x];
      } else if (x == column) {
        board->table[y * (board->w + 1) + column] = 0;
      } else {
        board->table[y * (board->w + 1) + x] = tab[y * board->w + (x - 1)];
      }
    }
    ++x;
  }
  ++board->w;
}

void remove_column_board(struct game_t *game, uint8_t column)
{
  struct board_t *board = &game->board; // Shortcut
  uint8_t x = 0, y;
  uint8_t tab[BOARD_TABLE_SIZE] = {0};
  
  if ((column >= board->w) || (board->w <= MIN_BOARD_COLUMN)) {
    return;
  }
  
  memcpy(tab, board->table, board->w * board->h);
  while (x < (board->w - 1)) {
    for (y = 0; y < board->h; y++) {
      if (x < column) {
        board->table[y * (board->w - 1) + x] = tab[y * board->w + x];
      } else {
        board->table[y * (board->w - 1) + x] = tab[y * board->w + (x + 1)];
      }
    }
    ++x;
  }
  --board->w;
  if ((game->tetri_comming[ON_BOARD].x + game->tetri_comming[ON_BOARD].w) > board->w) {
    --game->tetri_comming[ON_BOARD].x;
  }
}

/* Fill the half-bottom part of board with
 * random color and a 50% proportion
 */
void random_fill_board(struct game_t *game)
{
  uint8_t colors[7] = {COLOR_CYAN, COLOR_YELLOW, COLOR_MAGENTA, COLOR_WHITE, COLOR_BLUE, COLOR_RED, COLOR_GREEN};
  uint8_t x, y;
  uint8_t nb_line = game->board.h / 2;
  
  for (x = 0; x < game->board.w; x++) {
    for (y = game->board.h - 1; y > (game->board.h - nb_line - 1); y--) {
      if (rand() % 2) {
        game->board.table[y * game->board.w + x] = colors[rand() % 7];
      }
    }
  }
}

bool check_end_game(struct game_t *game)
{
  bool coli;
  
  coli = check_tetri_colision(&game->board, &game->tetri_comming[ON_BOARD], COLI_ALL);
  game->ended = !is_line_free(&game->board, 0) || coli;
  
  if (game->ended) {
    log_debug("> GAME OVER <");
    stop(game);
  }
  return game->ended;
}

// Internal Functions

void clear_board(struct board_t *board)
{
  memset(board->table, 0, board->w * board->h);
}

bool is_line_free(struct board_t *board, uint8_t y)
{
  bool test = true;
  uint8_t x = 0;
  
  while (test && (x < board->w)) {
    test &= (board->table[y * board->w + x] == FREE);
    x++;
  }
  return test;
}

bool is_line_complete(struct board_t *board, uint8_t y)
{
  bool test = true;
  uint8_t x = 0;
  
  while (test && (x < board->w)) {
    test &= (board->table[y * board->w + x] != FREE);
    x++;
  }
  
  return test;
}

void swap_line(struct board_t *board, uint8_t y1, uint8_t y2)
{
  uint8_t x = 0;
  
  for (x = 0; x < board->w; x++) {
    swap_u8(&board->table[y1 * board->w + x], &board->table[y2 * board->w + x]);
  }
}

void copy_tab(uint8_t *tab_dest, uint8_t *tab_src, uint16_t length)
{
  uint16_t i = 0;

  for (i = 0; i < length; i++) {
    tab_dest[i] = tab_src[i];
  }
}

bool board_size_is_valid(uint8_t w, uint8_t h)
{
  return (w * h <= BOARD_TABLE_SIZE);
}



