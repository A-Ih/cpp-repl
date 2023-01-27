#pragma once

#include "defs.h"
#include "utils.hh"

#include <type_traits>
#include <utility>
#include <variant>
#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <memory>

template<typename T>
T from_cstring(const char* str) {
  std::stringstream ss;
  ss << str;
  T res;
  if (!(ss >> res).good()) {
    throw std::runtime_error(utils::MakeString() << "Unable to convert string `" << str << "` to " << utils::TypeStr<T>);
  }
  return res;
}

template<typename R, typename ...Args>
constexpr auto get_arity(R (*)(Args...)) {
  return std::make_index_sequence<sizeof...(Args)>{};
}

template<typename R, typename ...Args, std::size_t ...I>
std::variant<R, std::string> call(call_info* c, R (*f)(Args...), std::index_sequence<I...>) {
  static_assert(sizeof...(Args) == sizeof...(I));
  try {
    return f(from_cstring<Args>(c->args[I])...);
  } catch(const std::runtime_error& e) {
    // No matter what exception is caught - we have no result
    return std::string{e.what()};
  }
}

template<typename T>
constexpr bool IsGoodArg = std::is_trivial_v<T> && !std::is_const_v<T> && !std::is_pointer_v<T>;

template<typename R, typename ...Args, typename = std::enable_if_t<(IsGoodArg<Args> && ...)>>
std::variant<R, std::string> call(call_info* c, R (*f)(Args...)) {
  return call(c, f, get_arity(f));
}

struct func_container {
  ~func_container() = default;

  template<typename TFuncPtr, typename = std::enable_if_t<std::is_pointer_v<TFuncPtr> && std::is_function_v<std::remove_pointer_t<TFuncPtr>>>>
  void addFunc(std::string name, TFuncPtr f) {
    if (f == nullptr) {
      std::cerr << "nullptr is passed for function " << name << std::endl;
      return;
    }
    if (auto it = funcs.find(name); it != funcs.end()) {
      std::cerr << "function " << name << " is already imported" << std::endl;
      return;
    }
    funcs[name] = [f](call_info* c) {
      std::stringstream ss;
      if (auto res = call(c, f); res.index() == 0) {
        ss << std::get<0>(res);
      } else {
        c->is_error = 1;
        ss << "call to " << c->func_name << " with arguments:";
        for (int i = 0; i < c->argnum; i++) {
          ss << " `" << c->args[i] << "`";
        }
        ss << " has failed with an exception: " << std::get<1>(res) << std::endl;
      }
      auto res_str = ss.str();
      auto size = std::min(res_str.size(), std::size_t(RESULT_SIZE - 1));
      std::memcpy(c->result, res_str.data(), size);
      c->result[size] = '\0';
      return c->is_error == 0 ? SUCCESS : BAD_FUNC_EVAL;
    };
  }

  int make_call(call_info* c) noexcept {
    if (auto it = funcs.find(c->func_name); it != funcs.end()) {
      auto& [name, f] = *it;
      try {
        return f(c);
      } catch (...) {
        return CRITICAL_ERROR;
      }
    }
    return BAD_FUNC;
  }

  std::unordered_map<std::string, std::function<int(call_info*)>> funcs;
};
