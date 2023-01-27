#include "glue.hh"

#include "utils.hh"

#include <iostream>
#include <memory>


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

  std::string arg1 = "12";
  std::string arg2 = "42";
  call_info c = {
    .func_name = "add",
    .argnum = 2,
    .args = { arg1.c_str(), arg2.c_str() },
    .result = { '\0' },
    .is_error = 0,
  };
  int code = loader->make_call(&c);
  std::clog << "made the call" << std::endl;
  switch (code) {
    case SUCCESS:
      std::cout << "Success: " << c.result << std::endl;
      break;
    default:
      std::cout << utils::MyFormat("Failed with code %, message: %", code, c.result) << std::endl;
  }
}
