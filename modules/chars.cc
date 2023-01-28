#include <stdexcept>

char toUpper(char c) {
  if ('a' <= c && c <= 'z') {
    return 'A' + (c - 'a');
  }
  return c;
}

char toLower(char c) {
  if ('A' <= c && c <= 'Z') {
    return 'a' + (c - 'A');
  }
  return c;
}

int code(char c) {
  return c;
}

char toHex(int c) {
  if (0 <= c && c < 10) {
    return '0' + c;
  } else if (10 <= c && c < 16) {
    return 'a' + (c - 10);
  } else {
    throw std::runtime_error("can't convert int to hex digit");
  }
}

int fromHex(char c) {
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'f') {
    return c - 'a' + 10;
  } else {
    std::abort();
  }
}
