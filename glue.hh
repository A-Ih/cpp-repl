#pragma once

#include "defs.h"

#include <unordered_map>
#include <string>
#include <vector>

struct module_info {
  std::string path;
  std::vector<std::string> funcs;
};

struct module_loader {

  module_loader();

  void load_module(module_info m);
  void unload_module(std::string name);
  int make_call(call_info* c);

private:
  using TMakeCall = int (*)(call_info*);

  void load_modules();

  void* handle{nullptr};
  TMakeCall f_make_call{nullptr};
  std::unordered_map<std::string, std::vector<std::string>> loaded_modules;
};
