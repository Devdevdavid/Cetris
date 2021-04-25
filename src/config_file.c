//
//  config_file.c
//  Cetris
//
//  Created by David Devant on 23/11/2017.
//  Copyright © 2017 David Devant. All rights reserved.
//

#include "cetris.h"

void init_file(struct game_t *game)
{
  if (get_file_len(game->stats.conf_path) == 0) {
    write_file_stats(&game->stats); // Create file and write init value
  } else {
    read_file_stats(&game->stats);
  }
}

void init_log(struct game_t *game)
{
  char str[PATH_MAX_LEN];
  sprintf(str, "%s/%s", game->cetris_path, LOG_FILE_NAME);
  log_set_fp(str);
}

void save_stats(struct game_t *game)
{
  read_file_stats(&game->stats); // Re-read file for potential updates
  game->stats.top_position = 10;
  update_topten(game); // Re-calculate position
  write_file_stats(&game->stats);
}

void read_file_stats(struct stats_t *stats)
{
  char buffer[31] = "";
  FILE *fp = NULL;
  uint8_t i;
  
  fp = fopen(stats->conf_path, "r");
  
  if (fp != NULL) {
    for (i = 0; i < 10; i++) {
      fgets(buffer, 31, fp);
      strncpy(stats->topten[i].login, get_string(buffer), LOGIN_LEN - 1);
      fgets(buffer, 31, fp);
      stats->topten[i].score = get_value(buffer);
      fgets(buffer, 31, fp);
      stats->topten[i].tetri = get_value(buffer);
    }
    fclose(fp);
  } else {
    log_error("Failed to open %s (r)", stats->conf_path);
  }
}

void write_file_stats(struct stats_t *stats)
{
  uint8_t i;
  FILE *fp = NULL;
  
  fp = fopen(stats->conf_path, "w");
  
  if (fp != NULL) {
    for (i = 0; i < 10; i++) {
      fprintf(fp, "login_%d=%s\n", i, stats->topten[i].login);
      fprintf(fp, "score_%d=%d\n", i, stats->topten[i].score);
      fprintf(fp, "tetri_%d=%d\n", i, stats->topten[i].tetri);
    }
    fclose(fp);
    log_debug("Stats saved");
  } else {
    log_error("Failed to open %s (w)", stats->conf_path);
  }
}

void get_cetris_path(struct game_t *game, const char * first_arg)
{
  const char *pos = 0;
  
  // Find the exec path
  if (first_arg[0] == '/') { // Absolute path
    sprintf(game->cetris_path, "%s", first_arg);
  } else if (first_arg[0] == '.') { // Relative path
    getcwd(game->cetris_path, PATH_MAX_LEN - 1);
    sprintf(game->cetris_path, "%s%s", game->cetris_path, &first_arg[1]);
  } else { // Like a command
	strcpy(game->cetris_path, "~/Cetris");
  }
  // Remove exec name
  pos = strrchr(game->cetris_path, '/');
  game->cetris_path[pos - game->cetris_path] = '\0';
  // Build config file's path
  sprintf(game->stats.conf_path, "%s/%s", game->cetris_path, CONF_FILE_NAME);
}

// Internal function

void get_label(char buffer[], char label[])
{
  while ((*buffer != '=') && *buffer) {
    buffer++;
  }
  strcpy(label, ++buffer);
}

char * get_string(char buffer[])
{
  // Find the '='
  while ((*buffer != '=') && *buffer) {
    buffer++;
  }
  if (buffer[strlen(buffer) - 1] == '\n') {
    buffer[strlen(buffer) - 1] = '\0';
  }
  return ++buffer;
}

uint32_t get_value(char buffer[])
{
  uint32_t value = 0;
  
  buffer = get_string(buffer);
  
  if ((*buffer) != '\0') {
    value = atoi(buffer);
  }
  
  return value;
}

uint16_t get_file_len(char path[])
{
  FILE *fp = NULL;
  uint16_t length = 0;
  
  fp = fopen(path, "r");
  
  if (fp != NULL) {
    while (fgetc(fp) != EOF) {
      length++;
    }
    fclose(fp);
  }
  return length;
}




