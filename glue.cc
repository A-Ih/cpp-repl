#include "glue.hh"
#include "utils.hh"

#include "dlfcn.h"

#include <regex>
#include <fstream>
#include <string>
#include <iostream>


std::string TEMPLATE = R"(
#include "defs.h"
#include "utils.hh"
#include "caller.hh"

//includes//

std::unique_ptr<func_container> call_site;
#if defined (__GNUC__)
void __attribute__((constructor)) my_init(void) {
  call_site = std::make_unique<func_container>();
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
void make_call(call_info* c) {
  call_site->make_call(c);
}
)";

module_loader::module_loader() {
  load_modules();
}

void module_loader::load_module(module_info m) {
  // we forcefully load modules whenever asked
  loaded_modules[m.path] = m.funcs;
  load_modules();
}

void module_loader::unload_module(std::string path) {
  if (auto it = loaded_modules.find(path); it != loaded_modules.end()) {
    loaded_modules.erase(it);
    load_modules();
  }
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
  std::string constructor;
  for (const auto& [path, funcs] : loaded_modules) {
    includes.append(utils::MyFormat("#include \"%\"\n", path));
    constructor.append(utils::MyFormat("// module %\n", path));
    for (const auto& func : funcs) {
      constructor.append(utils::MyFormat("call_site->addFunc(\"%\", &%);\n", func, func));
    }
  }
  auto result = std::regex_replace(TEMPLATE, std::regex("//includes//"), includes);
  result = std::regex_replace(result, std::regex("//constructor//"), constructor);
  {
    std::ofstream out("caller.cc");
    out << result << std::endl;
  }
  std::string compileCommand = "g++ -std=c++20 -shared -fPIC -g -Wall -o caller.plugin caller.cc";
  if (auto code = std::system("rm -f caller.plugin"); code != 0) {
    throw std::runtime_error("couldn't delete previous version of plugin");
  }
  if (auto code = std::system(compileCommand.c_str()); code != 0) {
    throw std::runtime_error(utils::MakeString() << "compilation failed with code " << code);
  }
  handle = dlmopen(LM_ID_NEWLM, "./caller.plugin", RTLD_NOW); // this is configurable, see https://man7.org/linux/man-pages/man3/dlopen.3.html
  if (dlerror() != nullptr) {
    throw std::runtime_error(utils::MakeString() << "dlopen failed: " << dlerror());
  }
  f_make_call = reinterpret_cast<TMakeCall>(dlsym(handle, "make_call"));
  if (dlerror() != nullptr) {
    throw std::runtime_error(utils::MakeString() << "couldn't load `make_call`, dlerror: " << dlerror());
  }
}
