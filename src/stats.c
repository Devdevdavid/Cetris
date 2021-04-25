//
//  stats.c
//  Cetris
//
//  Created by David Devant on 14/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

void init_stats(struct stats_t *stats)
{
  stats->top_position = 10; // Out of the topten (11th pos)
  stats->score = 0;
  stats->tetri_placed = 0;
  stats->tetri_placed_old = 0;
  stats->speed = 0;
  stats->level = 1;
  stats->multiply = 1;
  stats->cleaner_msg_enable = false;
}

void set_score(struct game_t *game, uint32_t new_score)
{
  struct stats_t *stats = &game->stats; // Shortcut
  stats->score = new_score;
  update_topten(game);
  if (stats->score > (stats->level * stats->level * SCORE_PER_LEVEL)) {
    level_up(game);
  }
  if (game->goal.event > 0) {
    if ((stats->score >= game->goal.score) && (game->goal.score > 0)) {
      game->goal.isScoreReached = true;
    }
  }
}

void add_score(struct game_t *game, uint16_t points)
{
  set_score(game, game->stats.score + points * game->stats.multiply);
}

void level_up(struct game_t *game)
{
  struct stats_t *stats = &game->stats; // Shortcut

  if (stats->level < LEVEL_MAX) {
    ++stats->level;
    if (game->goal.event > 0) {
      if ((stats->level >= game->goal.level) && (game->goal.level > 0)) {
        game->goal.isLevelReached = true;
      }
    }
  }
}

void add_tetri_placed(struct stats_t *stats)
{
  stats->tetri_placed++;
}

void set_cleaner_bonus(struct game_t *game)
{
  add_score(game, SCORE_CLEANER);
  game->stats.cleaner_msg_enable = 1;
  set_timeout(&game->timer, ID_TIMEOUT_CLEANER, 5);
}

void set_multiply(struct game_t *game, uint8_t multiply, uint16_t duration)
{
  if (multiply == 0) {
    return;
  }
  game->stats.multiply = multiply;
  
  if ((duration != 0) && (multiply != 1)) {
    set_timeout(&game->timer, ID_TIMEOUT_MULTIPLY, duration);
  }
}

void update_topten(struct game_t *game)
{
  struct stats_t *stats = &game->stats;
  
  if (stats->top_position == 10) {
    if (stats->score > stats->topten[9].score) { // If you have a better score than the TOP10 you take his place
      stats->top_position = 9;
    } else {
      return;
    }
  }
  // Refresh user score
  stats->topten[stats->top_position].score = stats->score;
  stats->topten[stats->top_position].tetri = stats->tetri_placed;
  strcpy(stats->topten[stats->top_position].login, game->display.login);
  
  if (stats->top_position == 0) {
    return; // GG, You are the best !
  }

  // Update user position
  while (stats->topten[stats->top_position].score > stats->topten[stats->top_position - 1].score) {
    swap_topten(&stats->topten[stats->top_position - 1], &stats->topten[stats->top_position]);
    if(--stats->top_position == 0) {
      break; // GG, You are the best !
    }
  }
}

/* NOT USED */
void bubble_sorting(struct topten_t *topten)
{
  uint8_t i, j;
  
  for(i = 9; i >= 1; i--) {
    for(j = 0; j <= i - 1; j++) {
      if(topten[j + 1].score < topten[j].score) {
        swap_topten(&topten[j], &topten[j + 1]);
      }
    }
  }
}

void swap_topten(struct topten_t *t1, struct topten_t *t2)
{
  struct topten_t temp;
  
  temp = (*t1);
  (*t1) = (*t2);
  (*t2) = temp;
}

void update_speed_counter(struct game_t *game)
{
  game->stats.speed = game->stats.tetri_placed - game->stats.tetri_placed_old;
  game->stats.tetri_placed_old = game->stats.tetri_placed;
  set_timeout(&game->timer, ID_TIMEOUT_SPEED, 1);
}



