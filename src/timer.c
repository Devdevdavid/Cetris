//
//  timer.c
//  Cetris
//
//  Created by David Devant on 14/10/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

void init_timer(struct timer_t *timer)
{
  timer->chrono = 0;
  timer->is_running = false;
}

void start_timer(struct timer_t *timer)
{
  if (timer->is_running)
    return;
  
  timer->start = time(NULL);
  timer->is_running = true;
}

void stop_timer(struct timer_t *timer)
{
  if (!timer->is_running)
    return;
  
  timer->chrono += time(NULL) - timer->start;
  timer->is_running = false;
}

struct tm* get_timer(struct timer_t *timer)
{
  // Refresh timer->chrono
  if (timer->is_running) {
    timer->chrono += time(NULL) - timer->start;
    timer->start = time(NULL);
  }
  
  // Convert time_t to tm
  return gmtime(&timer->chrono);
}

/*
 * Schedule an action which will be execute in 'duration' seconds
 */
void set_timeout(struct timer_t *timer, uint8_t id, uint16_t duration)
{
  if (id >= NB_TIMEOUT) {
    return;
  }
  timer->timeout[id] = timer->chrono + duration;
}

/* Execute function when timeout is reached
 * Based on chrono clock
 */
void timeout_callback(struct game_t *game)
{
  struct timer_t *timer = &game->timer; // Shortcut
  time_t cur_time = timer->chrono;
  uint8_t i;
  
  for (i = 0; i < NB_TIMEOUT; i++) {
    if ((cur_time >= timer->timeout[i]) && (timer->timeout[i] != DISABLED)) {
      timer->timeout[i] = DISABLED;
      switch (i) {
        case ID_TIMEOUT_MULTIPLY:
          set_multiply(game, 1, 0); // Reset multiply to 1 for infinite duration (0)
          break;
        case ID_TIMEOUT_CLEANER:
          game->stats.cleaner_msg_enable = 0; // Hide cleaner bonus message
          break;
        case ID_TIMEOUT_SPEED:
          update_speed_counter(game);
          break;
      }
    }
  }
}






