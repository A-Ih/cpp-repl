
#include "defs.h"
#include "utils.hh"
#include "caller.hh"





std::unique_ptr<func_container> call_site;
#if defined (__GNUC__)
void __attribute__((constructor)) my_init(void) {
  call_site = std::make_unique<func_container>();
  
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

