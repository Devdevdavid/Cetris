//
//  main.c
//  Cetris
//
//  Created by David Devant on 12/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

// I was making the systeme of timeout according with multiply

#include "cetris.h"

struct game_t *p_game;

int main(int argc, const char * argv[]) {
  // DECLARATION
  struct game_t game;      // Data of the whole game
  uint64_t timer_3ms = 0;  // Smallest tick
  uint64_t timer_AI = 0;
  uint64_t timer_33ms = 0; // 30 img/sec
  
  // INITIALIZATION
  get_cetris_path(&game, argv[0]);
  init_log(&game);
  init_board(&game.board);
  init_display(&game);
  init(&game);
  
  // MAIN LOOP
  while(!game.quit)
  {
    // Execute every 3ms
    if (game.cur_time >= timer_3ms) {
      timer_3ms = game.cur_time + 3;
      update(&game);
    }
    
    // Execute every 33ms
    if (game.cur_time >= timer_33ms) {
      timer_33ms = game.cur_time + 33;
      timeout_callback(&game);
      check_user_input(&game);
      render(&game);
    }
    
    // Execute every 10ms
    if (game.cur_time >= timer_AI) {
      timer_AI = game.cur_time + 10 * !game.ai_brain.high_speed;
      ai_move(&game);
    }
    
    game_sleep();
    game.cur_time++;
  }
  
  // FREE
  endwin();
  return 0;
}

void game_sleep(void)
{
  static clock_t begin = 0;
  uint32_t work_time = 0; // In us
  
  work_time = (uint32_t) (clock() - begin) * 1000000 / CLOCKS_PER_SEC;
  if (work_time < 1000) {
    usleep(1000 - work_time);
  }
  begin = clock();
}

void update(struct game_t *game)
{
  if (!game->in_game || game->pause || game->ended || game->ai_brain.high_speed) {
    return;
  }
  dropping_tetri(game);
  if (test_fall_speed(game) == MOVE_NOW) {
    if (check_tetri_colision(&game->board, game->tetri_comming, COLI_ALL)) {
      place_tetri(game);
    } else if (!game->ai_brain.enable) {
      move_tetri_down(&game->board, &game->tetri_comming[ON_BOARD]);
    }
  }
}

void place_tetri(struct game_t *game)
{
  write_tetri(&game->board, &game->tetri_comming[ON_BOARD]);
  add_score(game, SCORE_WRITE_TETRI);
  add_tetri_placed(&game->stats);
  check_complete_lines(game);
  use_next_tetri(game);
  check_end_game(game);
}

void render(struct game_t *game)
{
  if (!game->in_game) {
    print_start_win(&game->display);
  }
  if (game->in_game) {
    print_game_win(game);
  }
}








