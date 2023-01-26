SRCS=\
	 caller.cc

CC=gcc
CFLAGS=\
	   -Wall \
	   -fsanitize=address,leak,undefined \
	   -O2 \
	   -g \
	   -std=c++20

caller:
	$(CC) $(CFLAGS) -fPIC caller.cc -o libcaller.so

format: FORCE
	clang-format --verbose -i --style=Google $(SRCS)

FORCE:
