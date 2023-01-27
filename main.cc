#include "glue.hh"

int main() {
  auto loader = std::make_unique<module_loader>();
  loader->load_module({
      .name = "add",
      .func_pointer_signature = "int(*)(int, int)",
  });

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
  switch (code) {
    case SUCCESS:
      std::cout << "Success: " << c.result << std::endl;
      break;
    default:
      std::cout << utils::MyFormat("Failed with code %, message: %", code, c.result) << std::endl;
  }
}
