#include "cetris.h"

void ai_init(struct ai_brain_t *ai_brain)
{
#ifdef AI_SPY_MODE
  ai_brain->enable = true;
#else
  ai_brain->enable = false;
#endif
  
  ai_brain->high_speed = false;
}

void toggle_ai(struct game_t *game)
{
  if (!game->in_game || game->ended) {
    return;
  }
  
  strcpy(game->display.login, "JARVIS");
  
  if (game->ai_brain.enable) {
    log_debug("%s make sleep Jarvis", game->display.login);
    game->ai_brain.enable = false;
    game->ai_brain.high_speed = false;
  } else {
    log_debug("%s wake up Jarvis", game->display.login);
    game->ai_brain.enable = true;
  }
}

void toggle_high_speed(struct game_t *game)
{
  if (!(game->in_game && !game->ended && game->ai_brain.enable)) {
    return;
  }
  
  if (game->ai_brain.high_speed) {
    log_debug("%s slow down Jarvis", game->display.login);
    game->ai_brain.high_speed = false;
  } else {
    log_debug("%s speed up Jarvis", game->display.login);
    game->ai_brain.high_speed = true;
    game->tetri_comming[ON_BOARD].is_dropping = false;
  }
}

void ai_move(struct game_t *game)
{
  struct ai_brain_t *ai_brain = &game->ai_brain; // Shortcut
  uint8_t i;
  
  if (!game->in_game || !ai_brain->enable || game->pause || game->ended || game->tetri_comming[ON_BOARD].is_dropping) {
    return;
  }
  
  // Init all boards of AI
  for (i = 0; i < NB_TETRI_COMMING; i++) {
    ai_brain->board[i].h = game->board.h;
    ai_brain->board[i].w = game->board.w;
    ai_brain->board[i].no_wall_mode = game->board.no_wall_mode;
  }
  
  ai_brain->nb_passage = 0;
  ai_brain->cur_tetri = 0;
  ai_brain->best_branch_score = -1000;
  ai_compute_node(game);
  apply_node(game, &ai_brain->best_first_node);
}

// Internals Functions

/* Recursive function which explore possibilities
 * by moving and rotating tetris.
 * It computes the total score when it placed all tetris.
 * Next, it tries an other solution
 */
uint16_t ai_compute_node(struct game_t *game)
{
  struct ai_brain_t *ai_brain = &game->ai_brain; // Shortcut
  struct board_t *board = &game->ai_brain.board[ai_brain->cur_tetri]; // Shortcut
  struct board_t *board_prev = &game->ai_brain.board[ai_brain->cur_tetri - 1]; // Shortcut
  struct tetri_t *tetri = &game->tetri_comming[ai_brain->cur_tetri]; // Shortcut
  struct node_t *node = &ai_brain->branch.nodes[ai_brain->cur_tetri]; // Shortcut
  uint8_t r, pos_in_width;
  uint16_t x;
  
  if (ai_brain->cur_tetri < NB_TETRI_COMMING) {
    for (r = 0; r < 4; r++) {
	    pos_in_width = (board->no_wall_mode) ? (board->w) : (board->w - tetri->w + 1);
      for (x = 0; x < pos_in_width; x++) {
        ++ai_brain->nb_passage; // Just for fun
        if(ai_brain->cur_tetri == 0) {
          memcpy(board->table, game->board.table, board->h * board->w);
        } else {
          memcpy(board->table, board_prev->table, board->h * board->w);
        }
        tetri->x = x;
        tetri->y = 0;
        drop_tetri(board, tetri);
        node->x = tetri->x;
        node->y = tetri->y;
        node->rotate = r;
        write_tetri(board, tetri);
        node->complete_line = remove_complete_line(board);
#ifdef AI_SPY_MODE
        render(game);
        usleep(500);
#endif
        ai_brain->cur_tetri++; // Jump to next tetri
        ai_compute_node(game); // Recurcive here
        ai_brain->cur_tetri--; // Go back to previous
      }
      tetri->y = 0; // Needed for rotation
      tetri->x = 0;
      rotate90_tetri(board, tetri);
      improve_speed(tetri, &r);
    }
	  tetri->x = board->w / 2 - tetri->w / 2;
    tetri->y = 0;
  } else {
    // Compute score for the branch
    ai_get_column_height(&ai_brain->branch, &game->ai_brain.board[NB_TETRI_COMMING-1]);
    ai_get_holes(&ai_brain->branch, &game->ai_brain.board[NB_TETRI_COMMING-1]);
    ai_compute_score(&ai_brain->branch);
    // If branch is best, save the first node which interrests us
    if (ai_brain->best_branch_score <= ai_brain->branch.score) {
      ai_brain->best_first_node = ai_brain->branch.nodes[ON_BOARD];
      ai_brain->best_branch_score = ai_brain->branch.score;
#ifdef AI_SPY_MODE
      log_ia_boards(ai_brain);
      log_best_score(ai_brain);
#endif
    }
  }
  return 0;
}

/*
 * Refresh node->holes
 * Use     node->col_height[]
 */
void ai_get_holes(struct branch_t *branch, struct board_t *board)
{
  uint8_t x, y, cnt_free_on_line = 0;
  
  branch->holes = 0;
  branch->complete_lines = 0;
  y = board->h;
  
  while (((y--) > 1) && (cnt_free_on_line != board->w)) {
    cnt_free_on_line = 0;
    for (x = 0; x < board->w; x++) {
      if(board->table[y * board->w + x] == FREE) {
        if(y > (board->h - branch->col_height[x])) {
          ++branch->holes;
        }
      }
    }
  }
}

/*
 * Refresh node->aggregate_height
 *         node->bumpiness
 *         node->col_height[]
 */
void ai_get_column_height(struct branch_t *branch, struct board_t *board)
{
  uint8_t x, y;
  int16_t diff;
  
  branch->bumpiness = 0;
  branch->aggregate_height = 0;
  
  for (x = 0; x < board->w; x++) {
    y = 0;
    while((board->table[y * board->w + x] == FREE) && (y < board->h)) {
      ++y;
    }
    branch->col_height[x] = board->h - y;
    branch->aggregate_height += branch->col_height[x];
    if(x > 0) {
      diff = branch->col_height[x] - branch->col_height[x - 1];
      branch->bumpiness += ((diff < 0) ? (- diff) : (diff));
    }
  }
}

void ai_compute_score(struct branch_t *branch)
{
  branch->complete_lines = 0;
  for (uint8_t i = 0; i < NB_TETRI_COMMING; i++) {
    if(i == 0) {
      branch->complete_lines += branch->nodes[i].complete_line * 2;
    } else {
      branch->complete_lines += branch->nodes[i].complete_line;
    }
  }
  
  branch->score = 0;
  branch->score += AI_SCORE_AGGRE * branch->aggregate_height;
  branch->score += AI_SCORE_COMPL * branch->complete_lines;
  branch->score += AI_SCORE_HOLES * branch->holes;
  branch->score += AI_SCORE_BUMPI * branch->bumpiness;
}

void improve_speed(struct tetri_t *tetri, uint8_t *r_index)
{
  if ((tetri->name == 'I') && ((*r_index) >= 1)) {
    (*r_index) = 4;
  }
  if ((tetri->name == 'O') && ((*r_index) >= 0)) {
    (*r_index) = 4;
  }
  // T no changed
  // L no changed
  // J no changed
  if ((tetri->name == 'Z') && ((*r_index) >= 1)) {
    (*r_index) = 4;
  }
  if ((tetri->name == 'S') && ((*r_index) >= 1)) {
    (*r_index) = 4;
  }
}

void apply_node(struct game_t *game, struct node_t *node)
{
  uint8_t i = 0;
  game->tetri_comming[ON_BOARD].x = 0;
  game->tetri_comming[ON_BOARD].y = 0;
  
  for (i = 0; i < node->rotate; i++) {
    rotate90_tetri(&game->board, &game->tetri_comming[ON_BOARD]);
  }
  game->tetri_comming[ON_BOARD].x = node->x;
  
  if(game->ai_brain.high_speed) {
    game->tetri_comming[ON_BOARD].y = node->y;
    place_tetri(game);
  } else {
    set_drop_flag_tetri(&game->tetri_comming[ON_BOARD]);
  }
}

void log_best_score(struct ai_brain_t *ai_brain)
{
  struct branch_t *branch = &ai_brain->branch; // Shortcut

  log_debug("Best branch score: [%2d] ho=%2d cl=%2d ag=%2d bu=%2d sc=%4f ",
            ai_brain->nb_passage, branch->holes,
            branch->complete_lines, branch->aggregate_height, branch->bumpiness,
            branch->score);
}

void log_ia_boards(struct ai_brain_t *ai_brain)
{
  uint8_t x, y, i, pos;
  char str[150] = "";
  log_debug("Boards :");
  for (y = 0; y < ai_brain->board[0].h; y++) {
    pos = 0;
    for (i = 0; i < NB_TETRI_COMMING; i++) {
      for (x = 0; x < ai_brain->board[0].w; x++) {
        if(ai_brain->board[i].table[y * ai_brain->board[0].w + x] == 0) str[pos] = '.';
        else str[pos] = 48 + ai_brain->board[i].table[y * ai_brain->board[0].w + x];
        pos++;
      }
      str[pos++] = '_';
    }
    str[++pos] = '\0';
    log_debug("%s", str);
  }
}


