#pragma once

#include <sstream>

#define RESULT_SIZE 256
#define MAX_ARGS 32


// TODO: replace bool with something like int, because bool isn't stable in c
// and c++ ABI's.
struct call_info {
  const char* func_name;
  int argnum;
  const char* args[MAX_ARGS];
  char result[RESULT_SIZE];
  int is_error;
};

struct import_info {
  int argnum;
  char* args[MAX_ARGS];
  char result[RESULT_SIZE];
};

#define SUCCESS 0
#define BAD_FUNC -1
#define BAD_ARG -2
#define BAD_FUNC_EVAL -3
#define CRITICAL_ERROR -4


// Why `extern "C"`? Check page 88 of AC
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// return SUCCESS on successful call
// return BAD_FUNC on non-existent function
// return BAD_ARG on argument mismatch
// return BAD_FUNC_EVAL on exceptions during function evaluation
// return CRITICAL_ERROR on exceptions in the library code
int make_call(call_info*);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

