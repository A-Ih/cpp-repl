#include "defs.h"
#include "utils.hh"
#include "caller.hh"

//includes//

// TODO: there goes the code-gen part. Let's add a couple of examples

using f1T = int (*)(long, char*);
using f2T = int (*)(long);

//signatures//

// type signatures here

std::unique_ptr<func_container> call_site;
#if defined (__GNUC__)
void __attribute__((constructor)) my_init(void) {
  call_site = std::unique_ptr<func_container>();
  // TODO: codegen here: we put all the pairs <name, func_ptr> into `call_site`
  f1T kek = nullptr;
  f2T kek2 = nullptr;
  call_site->addFunc("f1", kek);
  call_site->addFunc("f2", kek2);

  //constructor//
}

void __attribute__((destructor)) my_fini(void) {
  // maybe smarter code should be written here
  call_site.reset();
}
#else
  #error "Compilers other than GCC are not supported yet"
#endif

extern "C"
int make_call(call_info* c) {
  return call_site->make_call(c);
}
