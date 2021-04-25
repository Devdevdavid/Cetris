//
//  user_cmd.c
//  Cetris
//
//  Created by David Devant on 14/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

void init(struct game_t *game)
{
  srand((unsigned) time(NULL));
  game->quit = 0;
  reset(game);
  init_block(game->blocks);
  init_file(game);
  log_debug("=== CETRIS STARTED ===");
  log_debug("=== %s ===", VERSION);
}

void reset(struct game_t *game)
{
  init_board(&game->board);
  init_stats(&game->stats);
  init_timer(&game->timer);
  init_tetri(game);
  ai_init(&game->ai_brain);
  game->ended = false;
  game->in_game = false;
  game->pause = false;
  game->board.no_wall_mode = false;
  update_speed_counter(game);
}

void start(struct game_t *game)
{
  reset(game);
  check_login(&game->display);
  start_timer(&game->timer);
  game->in_game = true;
  clear_display(&game->display);
}

void stop(struct game_t *game)
{
  game->ended = true;
  stop_timer(&game->timer);
  save_stats(game);
}

void quit(struct game_t *game)
{
  stop(game);
  game->quit = true;
  log_debug("=== CETRIS END ===");
}

void return_menu(struct game_t *game)
{
  log_debug("Game aborted by user");
  stop(game);
  game->in_game = false;
  clear_display(&game->display);
}

void check_user_input(struct game_t *game)
{
  if (!game->in_game) {
    check_user_input_start(game);
  } else {
    check_user_input_game(game);
  }
}

// Internal Functions

void check_user_input_start(struct game_t *game)
{
  uint16_t key = wgetch(game->display.start_win);
  switch(key)
  {
    case KEY_UP:
      if (game->display.select > 0) {
        game->display.select--;
      }
      break;
    case KEY_DOWN:
      if (game->display.select < (NB_MENU_ENTRIES - 1)) {
        game->display.select++;
      }
      break;
    case ALT_ENTER:
      process_choice(game);
      break;
    default:
      if (game->display.select == TITLE_LOGIN) {
        edit_login(game, key);
      } else if (key == 'q') {
        log_debug("%s quit the game", game->display.login);
        quit(game);
      }
      break;
  }
}

void check_user_input_game(struct game_t *game)
{
  uint16_t ch = wgetch(game->display.board);
  
  if (ch == 'q') {
    log_debug("User quit the game");
    quit(game);
  } else if (ch == 'p') {
    toggle_pause(game);
  } else if (ch == 'i') {
    toggle_ai(game);
  } else if (ch == 'h') {
    toggle_high_speed(game);
  } else if ((ch == ALT_ENTER && game->ended) || (ch == 'm')) {
    return_menu(game);
  } else if (ch == 'a') {
    add_column_board(game, 0);
    clear_display(&game->display);
  } else if (ch == 'z') {
    remove_column_board(game, game->board.w - 1);
    clear_display(&game->display);
  }
  // Move tetri
  if (!game->pause && !game->ai_brain.enable && !game->tetri_comming[0].is_dropping) {
    if (ch == KEY_LEFT) {
      move_tetri_left(&game->board, &game->tetri_comming[0]);
    } else if (ch == KEY_RIGHT) {
      move_tetri_right(&game->board, &game->tetri_comming[0]);
    } else if (ch == KEY_UP) {
      rotate90_tetri(&game->board, &game->tetri_comming[0]);
    } else if (ch == KEY_DOWN) {
      set_drop_flag_tetri(&game->tetri_comming[0]);
    }
  }
}

void edit_login(struct game_t *game, uint16_t key)
{
  uint8_t len = strlen(game->display.login);
  
  if (key == ALT_BACKSPACE || key == KEY_BACKSPACE) {
    if (len > 0) {
      game->display.login[len - 1] = '\0';
    }
  } else if(len < LOGIN_LEN - 1) {
    if (((key >= '0') && (key <= '9')) ||
        ((key >= 'a') && (key <= 'z')) ||
        ((key >= 'A') && (key <= 'Z'))) {
      game->display.login[len] = key;
      to_upper(game->display.login);
    }
  }
}

void process_choice(struct game_t *game)
{
  switch (game->display.select) {
    case TITLE_LOGIN:
      break;
    case TITLE_CLASSIC:
      log_debug("%s choose \"Classic Mode\"", game->display.login);
      start(game);
      break;
    case TITLE_B_MODE:
      log_debug("%s choose \"Mode B\"", game->display.login);
      start(game);
      random_fill_board(game);
      break;
    case TITLE_NO_WALL:
      log_debug("%s choose \"No Wall Mode\"", game->display.login);
      start(game);
      game->board.no_wall_mode = true;
      break;
    case TITLE_QUIT:
    default:
      log_debug("%s quit the game", game->display.login);
      quit(game);
      break;
  }
}

void toggle_pause(struct game_t *game)
{
  if (!(game->in_game && !game->ended)) {
    return;
  }
    
  if (game->pause) {
    log_debug("%s unpaused the game", game->display.login);
    game->pause = false;
    start_timer(&game->timer);
  } else {
    log_debug("%s paused the game", game->display.login);
    game->pause = true;
    stop_timer(&game->timer);
  }
}

void check_login(struct display_t *display)
{
  if (strlen(display->login) == 0) {
    strcpy(display->login, "NONAME");
  }
}

void to_upper(char s[])
{
  uint16_t i, len;
  
  len = strlen(s);
  for (i = 0; i < len; i++) {
    if ((s[i] >= 'a') && (s[i] <= 'z')) {
      s[i] -= 32;
    }
  }
}


