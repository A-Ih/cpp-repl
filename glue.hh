#pragma once

#include "defs.h"
#include "utils.hh"

#include "dlfcn.h"

#include <unordered_map>
#include <regex>
#include <fstream>

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

struct module_info {
  std::string name;
  std::string func_pointer_signature;
};

struct module_loader {

  module_loader() {
    load_modules();
  }

  void load_module(module_info m) {
    loaded[m.name] = m.func_pointer_signature;
    load_modules();
  }

  void unload_module(std::string name) {
    loaded.erase(name);
    load_modules();
  }

  int make_call(call_info* c) {
    return f_make_call(c);
  }

private:
  using TMakeCall = int (*)(call_info*);

  void load_modules() {
    assert(handle != nullptr);
    dlclose(handle);
    handle = nullptr;
    std::string includes;
    std::string declarations;
    std::string constructor;
    for (const auto& [name, signature] : loaded) {
      includes.append(utils::MyFormat("#include<%>\n", name));
      declarations.append(utils::MyFormat("using T%=%;\n", name, signature));
      constructor.append(utils::MyFormat(R"(
      T% %_ptr = dlsym(\"%\");
      assert(%_ptr != nullptr);
      call_site->addFunc<T%>(\"%\", %_ptr);
      )", name, name, name, name, name, name, name));
    }
    auto result = std::regex_replace(TEMPLATE, std::regex("//includes//"), includes);
    result = std::regex_replace(result, std::regex("//declaration//"), declarations);
    result = std::regex_replace(result, std::regex("//constructor//"), includes);
    {
      std::ofstream out("caller.cc.rendered");
      out << result << std::endl;
    }
    std::string compileCommand = "g++ -std=c++20 -shared -fPIC -g -WAll -o caller.plugin caller.cc.rendered";
    std::system(compileCommand.c_str());
    handle = dlopen("./caller.plugin", RTLD_LAZY); // this is configurable, see https://man7.org/linux/man-pages/man3/dlopen.3.html
    if (dlerror() != nullptr) {
      std::cerr << "aborting: dlopen failed, lmao" << std::endl;
      std::abort();
    }
    f_make_call = reinterpret_cast<TMakeCall>(dlsym(handle, "make_call"));
    if (dlerror() != nullptr) {
      std::cerr << "aborting, couldn't load `make_call`" << std::endl;
      std::abort();
    }
  }

  void* handle{nullptr};
  TMakeCall f_make_call{nullptr};
  std::unordered_map<std::string, std::string> loaded;
};
