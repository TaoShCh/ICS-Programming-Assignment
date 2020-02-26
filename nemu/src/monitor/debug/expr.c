#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

#define OPERNUM 10
enum {
  TK_NOTYPE = 256, TK_EQ, TK_UNEQ, TK_DECNUM, TK_HEXNUM, TK_REG, TK_AND, TK_OR, TK_DEREF

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"0x[0-9A-Fa-f]+", TK_HEXNUM},   // hexadecimal number
  {"[0-9]+", TK_DECNUM},   // decimal number
  {"\\$(eip|eax|ax|ah|al|ebx|bx|bh|bl|ecx|cx|ch|cl|edx|dx|dh|dl|esp|ebp|esi|edi|sp|bp|si|di)", TK_REG},
  {"\\+", '+'},         // plus
  {"\\*", '*'},         // mul
  {"-", '-'},           // minus
  {"/", '/'},           // div
  {"\\(", '('},           // left parenthesis
  {"\\)", ')'},           // right parenthesis
  {"==", TK_EQ},         // equal
  {"!=", TK_UNEQ},         // unequal
  {"&&", TK_AND},          // and
  {"\\|\\|", TK_OR},        //or
  {"!", '!'}            // not
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

//        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
//            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
			case '+':
			case '-':
			case '*':
			case '/':
			case '(':
			case ')':
			case '!':
			case TK_EQ:
			case TK_UNEQ:
			case TK_AND:
			case TK_OR:
				tokens[nr_token++].type = rules[i].token_type;
				break;
			case TK_NOTYPE:
				break;
			case TK_DECNUM:
				tokens[nr_token].type = TK_DECNUM;
				if (substr_len >= 32) {
					printf("The length of the number is too long.\n");
					assert(0);
				}
				strncpy(tokens[nr_token].str, substr_start, substr_len);
				nr_token++;
				break;
			case TK_HEXNUM:
				tokens[nr_token].type = TK_HEXNUM;
				if (substr_len >= 32) {
					printf("The length of the hex number is too long.\n");
					assert(0);
				}
				strncpy(tokens[nr_token].str, substr_start, substr_len);
				nr_token++;
				break;
			case TK_REG:
				tokens[nr_token].type = TK_REG;
				strncpy(tokens[nr_token].str, substr_start + 1,substr_len - 1);
				tokens[nr_token].str[substr_len - 1] = '\0';
				//printf("tokens[%d].str = %s\n", nr_token, tokens[nr_token].str);
				nr_token++;
				break;
			


          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p, int q){
	if (tokens[p].type == '(' && tokens[q].type == ')'){
		char stack[32];
		int top = 0;
		for(int i=p; i<=q; i++) {
			if (tokens[i].type == '('){
				stack[top++] = '(';
			} else if (tokens[i].type == ')') {
				if (stack[top-1] == '(') {
					top--;
					if (top == 0 && i != q) {
						return false;
					}
				} else {
					stack[top++] = '(';
				}
			}
		}	
		if (top == 0) {
			return true;
		} else {
			return false;
		}
	}
	return false;
}

static int operators[]={'!', '+', '-', '*', '/', TK_EQ, TK_UNEQ, TK_AND, TK_OR, TK_DEREF};

static int get_operator_index(int oper){
	for(int i = 0; i < OPERNUM; i++){
		if (oper == operators[i]){
			return i;
	 	}
	} 
	printf("Wrong Operator! %d\n", oper);
	assert(0);
}

int operators_priority[][OPERNUM] = {
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 0, 0, 1, 1, 1, 1, 0},
	{0, 1, 1, 0, 0, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
	{0, 0, 0, 0, 0, 1, 1, 1, 1, 0},
	{0, 0, 0, 0, 0, 1, 1, 1, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
	{0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
	{0, 1, 1, 1, 1, 1, 1, 1, 1, 0}
};

static int dominant_operator(int p, int q) {
	int dopr = '@';
	int dopr_index = -1;
	bool init_flag = false, parenthethese_flag = false;
	for(int i = p; i <= q; i++) {
	 	switch (tokens[i].type){
			case TK_DECNUM:
			case TK_HEXNUM:
			case TK_REG:
				break;
			case '+': case '-':
			case '*': case '/':
			case '!': case TK_DEREF:
			case TK_EQ: case TK_UNEQ: 
			case TK_AND: case TK_OR:
	 			if (!parenthethese_flag) {
	 				if (init_flag) {
	 					if (operators_priority[get_operator_index(dopr)][get_operator_index(tokens[i].type)]){
							dopr = tokens[i].type;
							dopr_index = i;
						}
					} else {
						dopr = tokens[i].type;
						dopr_index = i;
						init_flag = true;
					}
				}
				break;
			case '(':
				parenthethese_flag = true;
				break;
			case ')':
				parenthethese_flag = false;
				break;

		}
	}
	return dopr_index;
}

uint32_t eval(int p, int q) {
  if (p > q) {
    /* Bad expression */
	printf("Bad Expression!\n");
	assert(0);
  } 
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
      */
	if (tokens[p].type == TK_DECNUM) {
		return atoi(tokens[p].str);
	}else if(tokens[p].type == TK_HEXNUM) {
		return strtol(tokens[p].str, NULL, 16);
	}else if(tokens[p].type == TK_REG) {
		//printf("reg: %s\n", tokens[p].str);
		for (int i = R_EAX; i <= R_EDI; i++) {
			if (strcmp(regsl[i], tokens[p].str) == 0) {
				return reg_l(i);
			} else if (strcmp(regsw[i], tokens[p].str) == 0) {
				return reg_w(i);
			} else if (strcmp(regsb[i], tokens[p].str) == 0)	{
				return reg_b(i);
			}
		}
		if (strcmp(tokens[p].str, "eip")) {
			return cpu.eip;
		}
	}
  } 
  else if (check_parentheses(p, q) == true) {
    /*  The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    int op = dominant_operator(p, q);
	uint32_t val1 = 0;
	if (tokens[op].type != '!' && tokens[op].type != TK_DEREF){
		val1 = eval(p, op - 1);
	}
    uint32_t val2 = eval(op + 1, q);

	//printf("%d %d %d =\n", val1, tokens[op].type, val2);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
	  case '!': return !val2;
	  case TK_EQ: return val1 == val2;
	  case TK_UNEQ: return val1 != val2;
	  case TK_AND: return val1 && val2;
	  case TK_OR: return val1 || val2;
	  case TK_DEREF: return vaddr_read(val2, 4);
      default: assert(0);
    }
  } 
  assert(0);
}

uint32_t expr(char *e, bool *success) {
  if ( !make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i ++) {
	if (tokens[i].type == '*'){
		if (i == 0) {
		  tokens[i].type = TK_DEREF;
		} else {
			int type = tokens[i-1].type;
			if (type == '+' || type == '-' || type == '*' || type == '/' || type == TK_EQ || type == TK_UNEQ || type == TK_AND || type == TK_OR || type == '!') {
			  tokens[i].type = TK_DEREF;
			}
		}
    }
  }

  return eval(0, nr_token - 1);

}
