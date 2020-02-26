#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
	int n;
	if (args == NULL) {
		n = 1;
	} else {
		char * str_n = strtok(args, " ");
		n = atoi(str_n);
	 }
	cpu_exec(n);
	return 0;

}

static int cmd_p(char *args) {
	bool success = true;
	uint32_t result = expr(args, &success);
    if (success){
		printf("result: 0x%-8x  |  %d\n", result, result);
	} else {
		printf("Bad Expression!!!\n");
	}
	return 0;
}

static int cmd_x(char* args) {
	char * args_end = args + strlen(args);
	char * str_n = strtok(args, " ");
	int n = atoi(str_n);
	args = str_n + strlen(str_n) + 1;
	if (args >= args_end) {
		printf("lack argument!!!\n");
		return 0;
	}
	bool success = true;
	uint32_t result = expr(args, &success);
	if (success) {
		for(int i = 0; i < n; i++) {
			uint32_t mem = vaddr_read(result + 4 * i, 4);
			printf("0x%-8x  ~  0x%-8x : 0x%-2x 0x%-2x 0x%-2x 0x%-2x\n",result+4*i, result+4*i+4, mem & 0xff, (mem & 0xff00) >> 8, (mem & 0xff0000) >> 16, (mem & 0xff000000) >> 24);
		}
	}
	return 0;
}

static int cmd_w(char * args) {
	bool success = true;
	uint32_t temp = expr(args, &success);
	if (!success) {
		Log("The expression to set up watchpoint is wrong.");
		return 0;
	}
	WP* p = new_wp();
	p->expr = (char *)malloc(strlen(args)+1);
	strcpy(p->expr, args);
	p->original_value = temp;
	printf("Watchpoint %d : %s\n", p->NO, p->expr);
	return 0;
}	

static int cmd_d(char * args) {
	char * NO_str = strtok(args, " ");
	bool success = free_wp(atoi(NO_str));
	if (success) {
		printf("Delete watchpoint %s\n", NO_str);
	} else {
		printf("There is no watchpoint %s in use.\n", NO_str);
	}
	return 0;
}

static int cmd_info(char *args) {
	char * kind = strtok(args, " ");
	if (strcmp("r", kind) == 0) {
		for(int i = R_EAX; i <= R_EDI; i++) {
			printf("%s : %-8x\t%s : %-4x", regsl[i], reg_l(i), regsw[i], reg_w(i));
			if (i <= R_EBX) {
				printf("\t%s : %-2x\t%s : %-2x\n",regsb[i+4], reg_b((i+4)), regsb[i], reg_b(i));
			} else {
				printf("\n");
			}
		}
		printf("eip : %x\n", cpu.eip);
		printf("CF : %d OF : %d SF : %d ZF : %d IF : %d\n",cpu.eflags.CF, cpu.eflags.OF, cpu.eflags.SF, cpu.eflags.ZF, cpu.eflags.IF);
	} else if (strcmp("w", kind) == 0) {
		list_wp();
	} else {
		printf("Info command doesn't support %s option.\n", kind);
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "si", "Execute N instructions.", cmd_si },
  { "x", "x N EXPR. Print consecutive N 4 bytes in hex rom the result of EXPR.", cmd_x },
  { "p", "Calculate the value of the expression EXPR", cmd_p},
  { "w", "Use the expression to set the watchpoint, pause the program when the value of expr changes.", cmd_w},
  { "d", "Delete the watchpoint of the NO. N", cmd_d},
  { "info", "Print register status or watchpoint infomation", cmd_info },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
