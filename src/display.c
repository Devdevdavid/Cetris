//
//  display.c
//  Cetris
//
//  Created by David Devant on 14/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

bool init_display(struct game_t *game)
{
  initscr();        // Initialize the window
  noecho();         // Don't echo any keypresses
  curs_set(false);  // Don't display a cursor
  cbreak();
  set_escdelay(0);
  init_display_color();
  
  // Start
  game->display.start_win = newwin(TERM_HEIGHT, TERM_WIDTH, 0, 0);
  // Game
  game->display.board =     newwin(game->board.h + 2, 2 * game->board.w + 2, 1, 1);
  game->display.next_tetri= newwin(4 , 16 + (NB_TETRI_COMMING-2)*10, 0, 0);
  game->display.stats =     newwin(9 , 16, 0, 0);
  game->display.status =    newwin(5 , 16, 0, 0);
  game->display.topten =    newwin(14 , 34, 0, 0);
  game->display.hotkey =    newwin(4 , 50, 0, 0);
  
  keypad(game->display.start_win, TRUE);
  keypad(game->display.board, TRUE);
  wtimeout(game->display.start_win, 1);
  wtimeout(game->display.board, 1);
  
  game->display.select = 1;
  if (getlogin() == NULL) {
    strcpy(game->display.login, "UNKNOWN");
  } else {
    strcpy(game->display.login, getlogin());
    to_upper(game->display.login);
  }
  return 0;
}

/* Update position when change board size
 */
void update_windows_position(struct game_t *game)
{
  wresize(game->display.board, game->board.h + 2, 2 * game->board.w + 2);
  mvwin(game->display.next_tetri, 1, 2 * game->board.w + 3);
  mvwin(game->display.stats,      5, 2 * game->board.w + 3);
  mvwin(game->display.topten,     5, 2 * game->board.w + 19);
  mvwin(game->display.status,    14, 2 * game->board.w + 3);
  mvwin(game->display.hotkey,    19, 2 * game->board.w + 3);
}

void clear_display(void)
{
  clear();
  refresh();
}

void print_start_win(struct display_t *display)
{
  uint8_t xoff = TERM_WIDTH / 2 - 53 / 2;
  uint8_t yoff = 1;
  
  mvwprintw(display->start_win, yoff  , xoff," _______  _______ _________ _______ _________ _______");
  mvwprintw(display->start_win, yoff+1, xoff,"|  ____ \\|  ____ \\\\__   __/|  ____ |\\__   __/|  ____ \\");
  mvwprintw(display->start_win, yoff+2, xoff,"| |    \\/| |    \\/   | |   | |    ||   | |   | |    \\/");
  mvwprintw(display->start_win, yoff+3, xoff,"| |      | |__       | |   | |____||   | |   | |_____ ");
  mvwprintw(display->start_win, yoff+4, xoff,"| |      |  __|      | |   |     __|   | |   |_____  |");
  mvwprintw(display->start_win, yoff+5, xoff,"| |      | |         | |   | |\\ |      | |         | |");
  mvwprintw(display->start_win, yoff+6, xoff,"| |____/\\| |____/\\   | |   | | \\ \\_____| |___/\\____| |");
  mvwprintw(display->start_win, yoff+7, xoff,"|_______/|_______/   |_|   |/   \\__/\\_______/\\_______|");
  mvwprintw(display->start_win, TERM_HEIGHT - 1, TERM_WIDTH - 16, "By DevDev Studio");
  mvwprintw(display->start_win, TERM_HEIGHT - 1, 0, VERSION);
#ifdef AI_SPY_MODE
  mvwprintw(display->start_win, TERM_HEIGHT - 3, 0, "AI spy mode enable");
#endif
#ifdef DEBUG
  mvwprintw(display->start_win, TERM_HEIGHT - 2, 0, "Debug version");
#endif
  
  print_play_menu(display);
  wrefresh(display->start_win);
}

void print_play_menu(struct display_t *display)
{
  char choices[][10] = {"", "PLAY    ", "B MODE  ", "NO WALL ", "QUIT    "};
  char tips[][24] = {"   Edit your name !    ", " Play in classic mode  ", " Start with difficulty ", "No collision with board", "    Quit cetris :(     "};
  uint8_t i, xoff = TERM_WIDTH / 2 - 8, yoff = TERM_HEIGHT / 2 - 1;
  char temp_log[LOGIN_LEN] = "";
  
  add_spaces_after_login(display->login, temp_log);
  
  wmove(display->start_win, yoff++, xoff);
  whline(display->start_win, 0 , 12);
  for (i = 0; i < NB_MENU_ENTRIES; ++i) {
    if (display->select == i) {
      wattron(display->start_win, A_REVERSE);
    }
    if (i == TITLE_LOGIN) {
      mvwprintw(display->start_win, yoff, xoff, " %s", temp_log);
    } else {
      mvwprintw(display->start_win, yoff, xoff, " %s   ", choices[i]);
    }
    if (display->select == i) {
      wattroff(display->start_win, A_REVERSE);
    }
    yoff++;
  }
  wmove(display->start_win, yoff++, xoff);
  whline(display->start_win, 0, 12);
  
  mvwprintw(display->start_win, ++yoff, xoff + 6 - 23 / 2, "%s", tips[display->select]);
}

void print_game_win(struct game_t *game)
{
  update_windows_position(game);
  print_board(game);
  print_next_tetri(game);
  print_stats(game);
  print_hotkey(&game->display);
  print_topten(game);
  print_status(game);
}

// Window print

void print_board(struct game_t *game)
{
  struct display_t *display = &game->display;      // Shortcut
#ifdef AI_SPY_MODE
  struct board_t *board = &game->ai_brain.board[LAST_TETRI];
#else
  struct board_t *board = &game->board;
#endif
  uint8_t y = 0, x = 0;
  uint8_t color;
  
  for (y = 0; y < board->h; y++) {
    for (x = 0; x < board->w; x++) {
      color = board->table[y * board->w + x];
      print_pixel(display->board, color, x, y);
    }
  }
  print_tetri(game);
  if (game->stats.level <= POS_GUIDE_LVL_MAX) {
    print_position_guide(game);
  }
  
  switch(game->display.select) {
    case TITLE_CLASSIC: print_box(display->board, "Classic mode"); break;
    case TITLE_B_MODE: print_box(display->board, "B mode"); break;
    case TITLE_NO_WALL: print_box(display->board, "No wall mode"); break;
  }
  wrefresh(display->board);
}

void print_tetri(struct game_t *game)
{
  struct tetri_t *tetri = &game->tetri_comming[ON_BOARD]; // Shortcut
  uint8_t x, y, color;
  
  for (y = 0; y < tetri->h; y++) {
    for (x = 0; x < tetri->w; x++) {
      if (tetri->shape[y * tetri->w + x] != FREE) {
        color = tetri->shape[y * tetri->w + x];
        print_pixel(game->display.board, color, (tetri->x + x) % game->board.w, tetri->y + y);
      }
    }
  }
}

void print_position_guide(struct game_t *game)
{
  struct tetri_t *tetri = &game->tetri_comming[ON_BOARD]; // Shortcut
  uint8_t x, y, i;

  x = tetri->x;
  for (i = 0; i < 2; i++) {
    for (y = (tetri->y + tetri->h); y < game->board.h; y++) {
      if (game->board.table[y * game->board.w + x] == FREE) {
        mvwprintw(game->display.board, 1 + y, 1 + i + 2 * x,".");
      }
    }
    x = (tetri->x + tetri->w - 1) % game->board.w;
  }
}

void print_next_tetri(struct game_t *game)
{
  struct display_t *display = &game->display; // Shortcut
  uint8_t i,y, x, color;
  
  werase(display->next_tetri);
  for (i = 1; i < NB_TETRI_COMMING; i++) {
    for (y = 0; y < game->tetri_comming[i].h; y++) {
      for (x = 0; x < game->tetri_comming[i].w; x++) {
        color = game->tetri_comming[i].shape[y * game->tetri_comming[i].w + x];
        print_pixel(game->display.next_tetri, color, 2 + x + (i - 1) * 5, y);
      }
    }
  }
  print_box(display->next_tetri, "Next");
  wrefresh(display->next_tetri);
}

void print_stats(struct game_t *game)
{
  struct display_t *display = &game->display; // Shortcut
  struct stats_t *stats = &game->stats; // Shortcut
  struct tm timeinfo;
  char buffer[LOGIN_LEN] = "";
  
  timeinfo = *get_timer(&game->timer);
  
  mvwprintw(display->stats, 1, 2,"%s", add_spaces_after_login(display->login, buffer));
  mvwprintw(display->stats, 2, 2,"Timer: %s", format_time(timeinfo, buffer));
  mvwprintw(display->stats, 3, 2,"Score:%s", format_score(stats->score, buffer));
  mvwprintw(display->stats, 4, 2,"Tetri:%s", format_score(stats->tetri_placed, buffer));
  mvwprintw(display->stats, 5, 2,"Level: %5d", stats->level);
  mvwprintw(display->stats, 6, 2,"Multi: %5d", stats->multiply);
  mvwprintw(display->stats, 7, 2,"Speed:%3dt/s", stats->speed);

  print_box(display->stats, "Scoreboards");
  wrefresh(display->stats);
}

void print_hotkey(struct display_t *display)
{
  mvwprintw(display->hotkey, 1, 2,"P: Pause  DOWN: Drop   M: Menu    I: Toggle AI");
  mvwprintw(display->hotkey, 2, 2,"Q: Quit    < >: Move  UP: Rotate  H: HighSpeed");

  print_box(display->hotkey, "Hotkey");
  wrefresh(display->hotkey);
}

void print_topten(struct game_t *game)
{
  uint8_t i;
  char buffer[LOGIN_LEN] = "";
  wattron(game->display.topten, A_BOLD);
  mvwprintw(game->display.topten, 1, 2,"   Login      | Score | Tetri");
  wattroff(game->display.topten, A_BOLD);
  for (i = 0; i < 10; i++) {
    mvwprintw(game->display.topten, i + 2,  2,"%2d %s", i + 1, add_spaces_after_login(game->stats.topten[i].login, buffer));
    mvwprintw(game->display.topten, i + 2, 18,"%s", format_score(game->stats.topten[i].score, buffer));
    mvwprintw(game->display.topten, i + 2, 26,"%s", format_score(game->stats.topten[i].tetri, buffer));
  }
  
  print_box(game->display.topten, "Top 10");
  wrefresh(game->display.topten);
}

void print_status(struct game_t *game)
{
  struct display_t *display = &game->display; // Shortcut
  
  werase(display->status);
  box(display->status, 0, 0);
  if (game->ended) {
    mvwprintw(display->status, 1, 1,"   GAME OVER  ");
    mvwprintw(display->status, 3, 1,"  PRESS ENTER ");
  } else if (game->pause) {
    mvwprintw(display->status, 2, 1,"     PAUSE    ");
  } else if (game->stats.cleaner_msg_enable) {
    mvwprintw(display->status, 1, 1,"   CLEANER !  ");
    mvwprintw(display->status, 3, 1,"   + %4dpts  ", SCORE_CLEANER);
  } else if (game->ai_brain.high_speed) {
    mvwprintw(display->status, 1, 1,"  AI ENABLED  ");
    mvwprintw(display->status, 3, 1," HIGHSPEED AI ");
  } else if (game->ai_brain.enable) {
    mvwprintw(display->status, 2, 1,"  AI ENABLED  ");
  } else {
    werase(display->status);
  }
  wrefresh(display->status);
}

// Internal Functions

void init_display_color(void)
{
  if (has_colors() == FALSE) {
    endwin();
    log_fatal("Your terminal doesn't support colors");
    exit(1);
  }
  start_color();
  use_default_colors();
  
  init_pair(1, COLOR_BLACK, COLOR_RED);
  init_pair(2, COLOR_BLACK, COLOR_GREEN);
  init_pair(3, COLOR_BLACK, COLOR_YELLOW);
  init_pair(4, COLOR_BLACK, COLOR_BLUE);
  init_pair(5, COLOR_BLACK, COLOR_MAGENTA);
  init_pair(6, COLOR_BLACK, COLOR_CYAN);
  init_pair(7, COLOR_BLACK, COLOR_WHITE);
}

void print_pixel(WINDOW *window, uint8_t color, uint8_t x, uint8_t y)
{
  wattron(window, COLOR_PAIR(color));
  mvwprintw(window, 1 + y, 1 + 2 * x,"  ");
  wattroff(window, COLOR_PAIR(color));
}

void print_box(WINDOW *window, char title[])
{
  box(window, 0, 0);
  wattron(window, A_BOLD);
  mvwprintw(window, 0, 1,title);
  wattroff(window, A_BOLD);
}

char * format_score(uint32_t score, char *buffer)
{
  if (score <= 9999) {
    sprintf(buffer, "%6d", score); // 0 to 9,999
  } else if (score <= 999999) {
    sprintf(buffer, "%5dk", score/1000); // 10k to 999k
  } else if (score <= 999999999) {
    sprintf(buffer, "%5dM", score/1000000); // 1M to 999M
  } else {
    sprintf(buffer, "%5dG", score/1000000000); // 1G to 999G
  }
  return buffer;
}

char * format_time(struct tm timeinfo, char *buffer)
{
  if (timeinfo.tm_yday > 0) {
    sprintf(buffer, "%02dd%02dh", timeinfo.tm_yday, timeinfo.tm_hour);
  } else if (timeinfo.tm_hour > 0) {
    sprintf(buffer, "%02dh%02d", timeinfo.tm_hour, timeinfo.tm_min);
  } else {
    sprintf(buffer, "%02d:%02d", timeinfo.tm_min, timeinfo.tm_sec);
  }
  return buffer;
}

char * add_spaces_after_login(char src[], char dest[])
{
  uint8_t i, nb_space_to_add;
  
  nb_space_to_add = LOGIN_LEN - strlen(src) - 1;
  strcpy(dest, src);
  for (i = 0 ; i < nb_space_to_add ; i++) {
    strcat(dest, " ");
  }
  return dest;
}


