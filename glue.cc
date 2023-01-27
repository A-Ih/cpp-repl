#include "glue.hh"
#include "utils.hh"

#include "dlfcn.h"

#include <regex>
#include <fstream>
#include <string>


std::string TEMPLATE = R"(
#include "defs.h"
#include "utils.hh"
#include "caller.hh"

//includes//

//signatures//

std::unique_ptr<func_container> call_site;
#if defined (__GNUC__)
void __attribute__((constructor)) my_init(void) {
  call_site = std::unique_ptr<func_container>();
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
)";

module_loader::module_loader() {
  load_modules();
}

void module_loader::load_module(module_info m) {
  loaded[m.name] = m.func_pointer_signature;
  load_modules();
}

void module_loader::unload_module(std::string name) {
  loaded.erase(name);
  load_modules();
}

int module_loader::make_call(call_info* c) {
  return f_make_call(c);
}

void module_loader::load_modules() {
  if (handle != nullptr) {
    dlclose(handle);
    handle = nullptr;
  }
  std::string includes;
  std::string signatures;
  std::string constructor;
  for (const auto& [name, signature] : loaded) {
    includes.append(utils::MyFormat("#include \"%\"\n", name));
    signatures.append(utils::MyFormat("using T%=%;\n", name, signature));
    constructor.append(utils::MyFormat(R"(
    T% %_ptr = %;
    assert(%_ptr != nullptr);
    call_site->addFunc<T%>("%", %_ptr);
    )", name, name, name, name, name, name, name));
  }
  auto result = std::regex_replace(TEMPLATE, std::regex("//includes//"), includes);
  result = std::regex_replace(result, std::regex("//signatures//"), signatures);
  result = std::regex_replace(result, std::regex("//constructor//"), constructor);
  {
    std::ofstream out("caller.cc");
    out << result << std::endl;
  }
  std::string compileCommand = "g++ -std=c++20 -shared -fPIC -g -Wall -o caller.plugin caller.cc";
  if (auto code = std::system(compileCommand.c_str()); code != 0) {
    throw std::runtime_error(utils::MakeString() << "compilation failed with code " << code);
  }
  handle = dlopen("./caller.plugin", RTLD_LAZY); // this is configurable, see https://man7.org/linux/man-pages/man3/dlopen.3.html
  if (dlerror() != nullptr) {
    throw std::runtime_error(utils::MakeString() << "dlopen failed: " << dlerror());
  }
  f_make_call = reinterpret_cast<TMakeCall>(dlsym(handle, "make_call"));
  if (dlerror() != nullptr) {
    throw std::runtime_error(utils::MakeString() << "couldn't load `make_call`, dlerror: " << dlerror());
  }
}
