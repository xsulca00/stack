STACK=dynamic_stack
TEST=stack-test

all:
	gcc -std=c11 -Wall -g -Wextra -pedantic ${STACK}.c -c
	gcc -std=c11 -Wall -g -Wextra -pedantic ${TEST}.c -c
	gcc -std=c11 -o ${TEST} ${TEST}.o ${STACK}.o 

clean:
	rm *.o ${TEST}
