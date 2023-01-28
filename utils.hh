#pragma once

#include <sstream>
#include <string_view>
#include <chrono>
#include <vector>
#include <numeric>
#include <cassert>
#include <algorithm>

namespace utils {
template <typename T>
std::string ToString(const T& x) {
  std::ostringstream os;
  os << x;
  return os.str();
}

class MakeString {
public:
    template<class T>
    MakeString& operator<< (const T& arg) {
        ss << arg;
        return *this;
    }
    operator std::string() const {
        return ss.str();
    }
private:
    std::stringstream ss;
};

template <typename... Args>
std::string MyFormat(std::string_view formatStr, Args&&... args) {
  if constexpr (sizeof...(Args) == 0) {
    return std::string{formatStr};
  }
  if (std::count(formatStr.begin(), formatStr.end(), '%') != sizeof...(Args)) {
    throw std::runtime_error(
        "Number of arguments doesn't match number of replacement spots");
  }
  auto replacements = {ToString(args)...};
  auto currentReplacement = replacements.begin();
  std::ostringstream result;
  for (auto c : formatStr) {
    if (c == '%') {
      result << *currentReplacement;
      currentReplacement++;
    } else {
      result << c;
    }
  }
  return result.str();
}

template<typename T>
constexpr inline const char* TypeStr = "unknown";

#define DECL_TYPESTR(type) \
  template<> \
  constexpr inline const char* TypeStr<type> = #type;

DECL_TYPESTR(bool)
DECL_TYPESTR(char)
DECL_TYPESTR(unsigned char)
DECL_TYPESTR(signed char)
DECL_TYPESTR(short)
DECL_TYPESTR(unsigned short)
DECL_TYPESTR(int)
DECL_TYPESTR(unsigned int)
DECL_TYPESTR(long)
DECL_TYPESTR(unsigned long)
DECL_TYPESTR(long long)
DECL_TYPESTR(unsigned long long)
DECL_TYPESTR(float)
DECL_TYPESTR(double)
DECL_TYPESTR(long double)

#undef DECL_TYPESTR

struct TimeStat {
  double avg;
  double min;
  double max;
};

// Measure statistics of execution time of pure procedure (in microseconds)
template<typename F>
TimeStat MeasureTime(F f, int repetitions = 10) {
  assert(repetitions > 0);
  std::vector<double> times(repetitions);
  for (int i = 0; i < repetitions; i++) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    times[i] = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
  }
  return {
    .avg = std::accumulate(times.begin(), times.end(), 0.0) / repetitions,
    .min = *std::min_element(times.begin(), times.end()),
    .max = *std::max_element(times.begin(), times.end()),
  };
}

} // namespace utils
