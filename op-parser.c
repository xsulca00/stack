#include <stdio.h>

#include "scanner/scanner.h"
#include "utils/dynamic_stack.h"

const char op_table[][14] =
{
//       ==   !=    <	 >   <=   >=	+    -	  *    /   ID	 (    )	   $
/* == */{'>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>' },
/* != */{'>', '>', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>' },
/* <  */{'>', '>', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>' },
/* >  */{'>', '>', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>' },
/* <= */{'>', '>', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>' },
/* >= */{'>', '>', '>', '>', '>', '>', '<', '<', '<', '<', '<', '<', '>', '>' },
/* +  */{'>', '>', '>', '>', '>', '>', '>', '>', '<', '<', '<', '<', '>', '>' },
/* -  */{'>', '>', '>', '>', '>', '>', '>', '>', '<', '<', '<', '<', '>', '>' },
/* *  */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '<', '>', '>' },
/* /  */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '<', '>', '>' },
/* ID */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '_', '_', '>', '>' },
/* (  */{'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=', '_' },
/* )  */{'>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '_', '_', '>', '>' },
/* $  */{'<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '_', '=' },
};

#define BOTTOM  13 
static char compare_token(int a, int b)
{
    
//    if (a > TOK_RIGHT_PAR || a < 0)
//    {
//        fprintf(stderr, "Bad token '%c' !\n", a);
//        return -1;
//    }
//
//    if (b > TOK_RIGHT_PAR || b < 0)
//    {
//        fprintf(stderr, "Bad token '%c' !\n", b);
//        return -1;
//    }
//    fprintf(stderr,"TOK: %i\n", op_table[a][b]);
//      
    if (a == '$')
        a = BOTTOM;
    if (b == '$')
        b = BOTTOM;
	return op_table[a][b];
}

int a,b;
int result;
int temp;

const char *tokens[] = 
{
    "==", "!=", "<", ">", "<=", ">=", "+", "-", "*", "/", "ID", "(", ")", "$"
    
};

void parse_expression(void)
{
    stack_t pda = stack_ctor();
    scanner_openFile("input_test.txt");
    int c =  getToken(); 
    printf("TOKEN: %s\n", g_lastToken.data.string);
	stack_push(&pda, BOTTOM);
    do
	{
        int result = compare_token(stack_top(&pda), c);
		switch(result)
		{
			case '<':
			case '=':
                stack_push(&pda, c);
                c = getToken(); 
				break;
			case '>':
	//			printf("%s", tokens[stack_top(&pda)]);
                stack_pop(&pda);
				break; 
			default:
				fprintf(stderr, "SYNTAX ERROR BITCH! FUCK YOU! WTF IS THAT!? -> '%s' \n", tokens[stack_top(&pda)]); return; 
		}
	
	}
	while (1);
}

int main(void)
{
	parse_expression();
	putchar('\n');
	return 0;
}
