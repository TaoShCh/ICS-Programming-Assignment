#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char * expr;
  uint32_t original_value;


} WP;

bool free_wp(int NO);
bool check_wp();
WP * new_wp();
void list_wp();


#endif
