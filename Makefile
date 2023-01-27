SRCS=\
	 repl.cc \
	 caller.hh \
	 defs.h \
	 glue.cc \
	 glue.hh \
	 main.cc \
	 utils.hh

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
