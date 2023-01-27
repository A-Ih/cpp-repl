#include <cstring>
#include <iostream>
#include <memory>

#include "glue.hh"
#include "utils.hh"

#define END_REPL_INPUT "exit"
#define LOAD_MODULE_INPUT ":l"
#define UNLOAD_MODULE_INPUT ":u"
#define PARSE_ERROR -10

//#define string std::string
//$define vector std::vector

using namespace std;

struct operation {
  string op;
  vector<string> args;
  bool isFunc;
  string errorMessage;

  operation(string &op, bool isFunc) {
    this->op = op;
    this->isFunc = isFunc;
  }

  operation(string &errMessage) { this->errorMessage = errMessage; }
};

class Lexer {
 public:
  Lexer(string &input) { this->input = input; }

  string getNextToken() {
    string res;

    skipWhiteSpace();

    if (input[ind] == '\"') {
      return parseStringToken();
    }

    for (; ind < input.length() && !isWhiteSpace(input[ind]); ind++) {
      res += input[ind];
    }

    return res;
  }

  bool isInputEnd() {
    skipWhiteSpace();
    return ind == input.size();
  }

 private:
  int ind = 0;
  string input;

  string parseStringToken() {
    string res = "\"";
    ind++;

    for (; ind < input.size() && input[ind] != '\"'; ind++) {
      res += input[ind];
      if (input[ind] == '\\') {
        if (ind + 1 < input.size()) {
          res += input[++ind];
        } else {
          throw std::runtime_error(
              "Expected special symbol after \\ symbol. Found end of line.");
        }
      }
    }

    if (input[ind] == '\"') {
      res += '\"';
      ind++;
      return res;
    }

    throw std::runtime_error(
        "unexpected end of input line. Excepted closing quote (\"), found end "
        "of line.");
  }

  static bool isWhiteSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n';
  }

  void skipWhiteSpace() {
    while (isWhiteSpace(input[ind])) {
      ind++;
    }
  }
};

class Parser {
 public:
  explicit Parser(string &input) : lexer(input) {}

  operation parse() {
    string firstToken = lexer.getNextToken();

    if (firstToken == ":l") {
      return finishLoadModuleParse(firstToken);
    }

    if (firstToken == ":u") {
      return finishUnloadModuleParse(firstToken);
    }

    return finishFunctionInvocationParse(firstToken);
  }

 private:
  Lexer lexer;

  operation finishModuleParse(string &firstToken, string errMessage) {
    operation op = operation(firstToken, false);
    vector<string> args;
    args.push_back(lexer.getNextToken());
    op.args = args;
    if (lexer.isInputEnd()) {
      return op;
    }

    return {errMessage};
  }

  operation finishLoadModuleParse(string &firstToken) {
    return finishModuleParse(
        firstToken,
        "Unexpected amount of arguments. Expected :l <module_name>\n");
  }

  operation finishUnloadModuleParse(string &firstToken) {
    return finishModuleParse(
        firstToken,
        "Unexpected amount of arguments. Expected :u <module_name>\n");
  }

  operation finishFunctionInvocationParse(string &firstToken) {
    operation op = operation(firstToken, true);
    op.args = parseArgs();

    return op;
  }

  vector<string> parseArgs() {
    vector<string> args;

    while (!lexer.isInputEnd()) {
      string arg = lexer.getNextToken();
      args.push_back(arg);
    }

    return args;
  }
};

void printOperation(operation &op) {
  cout << "op: " << op.op << endl;
  cout << "args: ";
  for (auto &arg : op.args) {
    cout << "[" << arg << "] ";
  }
  cout << endl;
  cout << "isFunc: " << op.isFunc << endl;
  cout << "error message: " << op.errorMessage << endl;
}

string escapeSpecialCharacters(string &s) {
  string res;

  for (char c : s) {
    if (c == '\\' || c == '"' || c == '\'') {  // special char
      res += '\\';
    }
    res += c;
  }

  return res;
}

template <typename F>
void LogTime(F f, int repetitions = 1) {
  auto [avg, min, max] = MeasureTime(std::move(f), repetitions);
  std::cout << utils::MyFormat("% launches: avg=% min=% max=%", repetitions,
                               avg, min, max)
            << std::endl;
}

void repl(unique_ptr<module_loader> &loader);
bool doCommand(operation &op);

int main() {
  auto loader = std::make_unique<module_loader>();
  std::clog << "created loader" << std::endl;
  repl(loader);
  //  loader->load_module({
  //                          .name = "add",
  //                          .func_pointer_signature = "int(*)(int, int)",
  //                      });
  //  std::clog << "loaded module" << std::endl;
  //
  //  call_info c = {
  //      .func_name = "add",
  //      .argnum = 2,
  //      .args = {"12", "42"},
  //      .result = {'\0'},
  //      .is_error = 0,
  //  };
  //  int code = loader->make_call(&c);
  //  std::clog << "made the call" << std::endl;
  //  assert(std::strcmp(c.result, "54") == 0);
  //  switch (code) {
  //    case SUCCESS:std::cout << "Success: " << c.result << std::endl;
  //      break;
  //    default:std::cout << utils::MyFormat("Failed with code %, message: %",
  //    code, c.result) << std::endl;
  //  }
}

void repl(unique_ptr<module_loader> &loader) {
  string s1;
  getline(cin, s1, '\n');

  while (s1 != END_REPL_INPUT) {
    Parser p = Parser(s1);
    operation op = p.parse();
    doCommand(op, loader);
    getline(cin, s1, '\n');
  }
}

// returns is success or not
bool doCommand(operation &op, unique_ptr<module_loader> &loader) {
  if (op.errorMessage != "") {
    cout << op.errorMessage;
    return false;
  }

  if (op.isFunc) {
    call_info c = {.func_name = op.op.c_str(),
                   .argnum = (int)op.args.size(),
                   .result = {'\0'},
                   .is_error = 0};
    for (int i = 0; i < op.args.size(); i++) {
      c.args[i] = op.args[i].c_str();
    }
    int code = loader->make_call(&c);
    std::clog << "made the call" << std::endl;
    switch (code) {
      case SUCCESS:
        std::cout << "Success: " << c.result << std::endl;
        break;
      default:
        std::cout << utils::MyFormat("Failed with code %, message: %", code,
                                     c.result)
                  << std::endl;
    }
  } else {
    if (op.op == LOAD_MODULE_INPUT) {
      loader->load_module({
          .name = op.args[0],
          .func_pointer_signature = op.args[1],
      });
    } else if (op.op == UNLOAD_MODULE_INPUT) {
      loader->unload_module(op.args[0]);
    }
  }

  return true;
}
