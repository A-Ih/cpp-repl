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
  if ((ss >> res).fail() || ss.peek() != std::stringstream::traits_type::eof()) {
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
    if (c->argnum != sizeof...(Args)) {
      throw std::runtime_error(utils::MyFormat("arity mismatch: expected %, got %", sizeof...(Args), c->argnum));
    }
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

inline void fill_buffer(char* buf, const int cap, const char* src) {
  auto src_len = std::strlen(src);
  auto size = std::min(src_len, std::size_t(cap - 1));
  std::memcpy(buf, src, size);
  buf[size] = '\0';
}

struct func_container {
  func_container() = default;
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
      fill_buffer(c->result, RESULT_SIZE, res_str.c_str());
    };
  }

  void make_call(call_info* c) noexcept {
    if (auto it = funcs.find(c->func_name); it != funcs.end()) {
      auto& [name, f] = *it;
      try {
        f(c);
        return;
      } catch (...) {
        c->is_error = 1;
        fill_buffer(c->result, RESULT_SIZE, "encountered exception outside of function");
        return;
      }
    }
    c->is_error = 1;
    fill_buffer(c->result, RESULT_SIZE, "no function with such name is loaded");
  }

  std::unordered_map<std::string, std::function<void(call_info*)>> funcs;
};
