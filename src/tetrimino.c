//
//  tetrimino.c
//  Cetris
//
//  Created by David Devant on 14/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

void init_tetri(struct game_t *game)
{
  uint8_t i = 0;
  for (i = 0; i < NB_TETRI_COMMING; i++) {
    use_next_tetri(game);
  }
}

uint8_t rotate90_tetri(struct board_t *board, struct tetri_t *tetri)
{
  uint8_t i = 0;
  
  transpose_shape(tetri);
  reverse_column(tetri);
  
  if (check_tetri_colision(board, tetri, COLI_SUPER)) { // If not possible, come back
    reverse_column(tetri);
    transpose_shape(tetri);
    return 1;
  }
  
  // Move left if colision on right border
  while (check_tetri_colision(board, tetri, COLI_BORDER) && (i < 4)) {
    move_tetri_left(board, tetri);
    i++;
  }
  return 0;
}

void move_tetri_left(struct board_t *board, struct tetri_t *tetri)
{
  if (tetri->x == 0) {
    if (board->no_wall_mode) {
      tetri->x = board->w - 1;
    } else {
      return;
    }
  } else {
    tetri->x--;
  }
  
  if (check_tetri_colision(board, tetri, COLI_SUPER)) { // If not possible, cancel mouvement
    if (tetri->x >= (board->w - tetri->w)) {
      tetri->x = 0;
    } else {
      tetri->x++;
    }
  }
}

void move_tetri_right(struct board_t *board, struct tetri_t *tetri)
{
  if (board->no_wall_mode) {
    if (tetri->x >= (board->w - 1)) {
      tetri->x = 0;
    } else {
      tetri->x++;
    }
  } else {
    if (tetri->x >= (board->w - tetri->w)) {
      return;
    } else {
      tetri->x++;
    }
  }
  
  if (check_tetri_colision(board, tetri, COLI_SUPER)) { // If not possible, cancel mouvement
    if (tetri->x == 0) {
      tetri->x = board->w;
    } else {
      tetri->x--;
    }
  }
}

void move_tetri_down(struct board_t *board, struct tetri_t *tetri)
{ 
  if (tetri->y >= (board->h - tetri->h)) {
    return;
  }
  tetri->y++;
}

bool test_fall_speed(struct game_t *game)
{
  struct tetri_t *tetri = &game->tetri_comming[ON_BOARD]; // Shortcut
  uint8_t speed_counter;
  
  speed_counter = ((LEVEL_MAX - game->stats.level) * 10); // Level set the speed
  if ((!tetri->is_dropping) && (++tetri->div_clk >= speed_counter))
  {
    tetri->div_clk = 0;
    return MOVE_NOW;
  }
  return MOVE_LATER;
}

bool test_drop_speed(struct game_t *game)
{
  struct tetri_t *tetri = &game->tetri_comming[ON_BOARD]; // Shortcut
  
  if (!tetri->is_dropping) {
    return MOVE_LATER;
  }
  
  if (++tetri->div_clk >= DROP_SPEED) {
    tetri->div_clk = 0;
    return MOVE_NOW;
  }
  
  return MOVE_LATER;
}

/* Check colision between board and tetri falling
 * Select which colision you want with filter (See COLI_xxxx in .h)
 */
bool check_tetri_colision(struct board_t *board, struct tetri_t *tetri,  uint8_t filter)
{
  bool test = false;
  uint16_t index = 0;
  uint8_t xt = 0, yt = 0;
  
  if (filter == 0) { // No colision if filter is empty
    return (test = 0);
  }
  if (board->no_wall_mode == true) {
    filter &= ~COLI_VERTI_BORDER;
  }
  
  test |= ((tetri->y + tetri->h) >= board->h) && ((filter & COLI_HORIZ_BORDER) != 0);
  test |= ((tetri->x + tetri->w) >  board->w) && ((filter & COLI_VERTI_BORDER) != 0);
  
  while ((yt < tetri->h) && !test) {
    xt = 0;
    while ((xt < tetri->w) && !test) {
      if (tetri->shape[yt * tetri->w + xt] != FREE) {     // Test the tetri's box
        index = (tetri->y + yt) * board->w + (tetri->x + xt) % board->w;
        test |= (board->table[index] != FREE) && ((filter & COLI_SUPER) != 0);
        index = (tetri->y + yt + 1) * board->w + (tetri->x + xt) % board->w;
        test |= (board->table[index] != FREE) && ((filter & COLI_UNDER) != 0);
      }
      ++xt;
    }
    ++yt;
  }
  return test;
}

void drop_tetri(struct board_t *board, struct tetri_t *tetri)
{
  while (!check_tetri_colision(board, tetri, COLI_ALL)) {
    move_tetri_down(board, tetri);
  }
}

void set_drop_flag_tetri(struct tetri_t *tetri)
{
  tetri->is_dropping = true;
  tetri->div_clk = 0;
}

void dropping_tetri(struct game_t *game)
{
  if(test_drop_speed(game) == MOVE_LATER) {
    return; // Drop is not for now
  }
  
  if (!check_tetri_colision(&game->board, &game->tetri_comming[ON_BOARD], COLI_ALL)) {
    move_tetri_down(&game->board, &game->tetri_comming[ON_BOARD]);
    add_score(game, SCORE_DROP);
  } else {
    game->tetri_comming[ON_BOARD].is_dropping = false;
    game->tetri_comming[ON_BOARD].div_clk = 254; // Magic happen here: avoid moving tetri after drop
  }
}

void use_next_tetri(struct game_t* game)
{
  uint8_t i;
  
  for (i = 0; i < (NB_TETRI_COMMING - 1); i++) {
    game->tetri_comming[i] = game->tetri_comming[i + 1];
  }
  choose_next_tetri(game);
}

// Internal Functions

void swap_u8(uint8_t *a, uint8_t *b)
{
  uint8_t tmp = (*a);
  (*a) = (*b);
  (*b) = tmp;
}

void transpose_shape(struct tetri_t *tetri)
{
  uint8_t temp[6];
  uint8_t x, y;
  
  memcpy(temp, tetri->shape, 6 * sizeof(uint8_t));
  
  for (y = 0; y < tetri->h; y++) {
    for (x = 0; x < tetri->w; x++) {
      tetri->shape[x * tetri->h + y] = temp[y * tetri->w + x];
    }
  }
  swap_u8(&tetri->w, &tetri->h);
}

void reverse_column(struct tetri_t *tetri)
{
  uint8_t *shape = tetri->shape; // Shortcut
  uint8_t y;

  for (y = 0; y < tetri->h; y++) {
    swap_u8(&shape[y * tetri->w], &shape[y * tetri->w + (tetri->w - 1)]);
  }
}

/* In blocks, shape and color are divided to save memory
 * so we merge the two for math with tetri's format
 */
void cpy_block_to_tetri(struct tetri_t *tetri, struct block_t *block)
{
  bool bit;
  uint8_t i = 0;
  
  for (i = 0; i < 6; i++) {
    bit = (block->shape >> (5 - i)) & 1;
    tetri->shape[i] = bit ? block->color : 0;
  }
}

void choose_next_tetri(struct game_t* game)
{
  uint8_t random_index;
  
  random_index = rand() % 7;
  
  cpy_block_to_tetri(&game->tetri_comming[LAST_TETRI], &game->blocks[random_index]);
  game->tetri_comming[LAST_TETRI].w = game->blocks[random_index].w;
  game->tetri_comming[LAST_TETRI].h = game->blocks[random_index].h;
  game->tetri_comming[LAST_TETRI].x = game->board.w / 2 - game->tetri_comming[LAST_TETRI].w / 2;
  game->tetri_comming[LAST_TETRI].y = 0;
  game->tetri_comming[LAST_TETRI].div_clk = 0;
  game->tetri_comming[LAST_TETRI].is_dropping = false;
  game->tetri_comming[LAST_TETRI].name = game->blocks[random_index].name;
}
