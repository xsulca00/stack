STACK=dynamic_stack
LEX=scanner
PARSER=op-parser
PROGRAM=parser

all:
	gcc -std=c11 -Wall -g -Wextra -pedantic utils/${STACK}.c -c
	gcc -std=c11 -Wall -g -Wextra -pedantic scanner/${LEX}.c -c	
	gcc -std=c11 -Wall -g -Wextra -pedantic ${PARSER}.c -c
	gcc -std=c11 -o ${PROGRAM} ${STACK}.o ${LEX}.o ${PARSER}.o

clean:
	rm *.o  ${PROGRAM}
