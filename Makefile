SRCS=\
	 caller.cc \
	 repl.cc

CC=g++
CFLAGS=\
	   -Wall \
	   -O2 \
	   -g \
	   -std=c++20

main: FORCE
	$(CC) $(CFLAGS) -o repl repl.cc glue.cc

format: FORCE
	clang-format --verbose -i --style=Google $(SRCS)

FORCE:
