#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
	if (free_ == NULL) {
		Assert(0, "the watchpoints from wp_pool are all in use.\n");
	}
	WP * p = free_;
	free_ = free_->next;
	p->next = head;
	head = p;	
	return p;
}

bool free_wp(int NO) {
	if (head->NO == NO) {
		WP *p =head;
		head = head->next;
		p->next = free_;
		free_ = p;
		return true;
	} 
	for (WP *prev = head, *p = head->next; p ; prev = p, p = p->next) {
		if (p->NO == NO) {
			prev->next = p->next;
			p->next = free_;
			free_ = p;
			return true;
	 	}
	} 
	return false;
}

bool check_wp() {
	bool flag = false;
	for (WP * p =  head; p; p = p -> next) {
		bool success = true;
		uint32_t new_value = expr(p -> expr, &success);
		if (success) {
			if (new_value != p -> original_value) {
				printf("\nWatchpoint %d : %s\n", p -> NO, p -> expr);
				printf("\nOld value = 0x%-8x  |  %d\n", p -> original_value, p -> original_value);
				printf("New value = 0x%-8x  |  %d\n", new_value, new_value);
				p -> original_value = new_value;
				flag = true;
			}
		}

	}
	return flag;
}

void list_wp() {
	printf("NUM\tExpr\n");
	for(WP * p = head; p; p = p -> next) {
		printf("%-3d\t%s\n", p -> NO, p -> expr);
	}
}
