//
//  main.c
//  Cetris
//
//  Created by David Devant on 12/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

struct game_t *p_game;

int init_goal(int argc, const char * argv[], struct goal_t *goal) {
	// Init values
	goal->isLevelReached = false;
	goal->isScoreReached = false;
	goal->level = 0;
	goal->score = 0;
	goal->event = 0;

	for (int index = 1; index < argc; ++index) {
		if (strcmp(argv[index], "-gl") == 0) {
			if ((index + 1) < argc) {
				goal->level = atoi(argv[index + 1]);
			} else {
				printf("Wrong -gl argument\n");
				return -1;
			}
		}
		if (strcmp(argv[index], "-gs") == 0) {
			if ((index + 1) < argc) {
				goal->score = atoi(argv[index + 1]);
			} else {
				printf("Wrong -gs argument\n");
				return -1;
			}
		}
		if (strcmp(argv[index], "-e") == 0) {
			if ((index + 1) < argc) {
				goal->event = atoi(argv[index + 1]);
			} else {
				printf("Wrong -e argument\n");
				return -1;
			}
		}
	}

	if (goal->level == 1) {
		printf("You need to specify a level highter than 1\n");
		return -1;
	}

	if (((goal->level > 0) || (goal->score > 0)) && (goal->event == 0)) {
		printf("You need to specify an event with -e option\n");
		return -1;
	}

	if ((goal->level == 0) && (goal->score == 0) && (goal->event > 0)) {
		printf("You need to specify a level and/or a score goal with -gl and -gs options\n");
		return -1;
	}

	// Specific Version fo Simon
	/*goal->level = 0;
	goal->score = 4000;
	goal->event = 2;*/

	return 0;
}

int main(int argc, const char * argv[]) {
  // DECLARATION
  struct game_t game;      // Data of the whole game
  uint64_t timer_3ms = 0;  // Smallest tick
  uint64_t timer_AI = 0;
  uint64_t timer_33ms = 0; // 30 img/sec
  uint64_t timer_pipe = 0;   // 1 img/sec
  
  // INITIALIZATION
  get_cetris_path(&game, argv[0]);
  init_log(&game);
  if (init_goal(argc, argv, &game.goal) != 0) {
	  return -1;
  }
  if (init_data_pipe(&game.data_pipe) != 0) {
	  return -1;
  }
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
      check_goal(&game);
      render(&game);
    }

    if (game.cur_time >= timer_pipe) {
      timer_pipe = game.cur_time + 100;
      if (game.in_game) {
    	  data_pipe_write(&game);
      }
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
  data_pipe_close(&game.data_pipe);
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
  } else {
    print_game_win(game);
  }
  apply_prints(game);
}

void check_goal(struct game_t *game)
{
	if (strcmp(game->display.login, "JARVIS") == 0) {
		return; // Do not accept Jarvis !
	}
	if (game->goal.isScoreReached) {
		switch (game->goal.event) {
		case 1:
		case 2:
			print_goal_msg(game);
			game->ended = true;
			break;
		default: break;
		}
	}
	if (game->goal.isLevelReached) {
		switch (game->goal.event) {
		case 1:
			break;
		default: break;
		}
	}
	if (game->goal.isLevelReached && game->goal.isScoreReached) {
		switch (game->goal.event) {
		case 1:
			break;
		default: break;
		}
	}
}








