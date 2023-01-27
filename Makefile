SRCS=\
	 caller.cc

CC=g++
CFLAGS=\
	   -Wall \
	   -fsanitize=address,leak,undefined \
	   -O2 \
	   -g \
	   -std=c++20

main: FORCE
	$(CC) $(CFLAGS) -o main main.cc glue.cc

format: FORCE
	clang-format --verbose -i --style=Google $(SRCS)

FORCE:
