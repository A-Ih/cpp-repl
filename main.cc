#include "glue.hh"

#include "utils.hh"

#include <iostream>
#include <memory>
#include <cstring>


template<typename F>
void LogTime(F f, int repetitions = 1) {
  auto [avg, min, max] = MeasureTime(std::move(f), repetitions);
  std::cout << utils::MyFormat("% launches: avg=% min=% max=%", repetitions, avg, min, max) << std::endl;
}

int main() {
  auto loader = std::make_unique<module_loader>();
  std::clog << "created loader" << std::endl;
  loader->load_module({
      .name = "add",
      .func_pointer_signature = "int(*)(int, int)",
  });
  std::clog << "loaded module" << std::endl;

  call_info c = {
    .func_name = "add",
    .argnum = 2,
    .args = { "12", "42" },
    .result = { '\0' },
    .is_error = 0,
  };
  int code = loader->make_call(&c);
  std::clog << "made the call" << std::endl;
  assert(std::strcmp(c.result, "54") == 0);
  switch (code) {
    case SUCCESS:
      std::cout << "Success: " << c.result << std::endl;
      break;
    default:
      std::cout << utils::MyFormat("Failed with code %, message: %", code, c.result) << std::endl;
  }
}
