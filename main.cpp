#include <iostream>
#include <fstream>
#include <map>
#include <regex>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#include "./AlbertTypes.hpp"
#include "./AlbertUtils.hpp"
#include "./InputStream.hpp"
#include "./TokenStream.hpp"
#include "./Parser.hpp"
#include "./Environment.hpp"

token_t * evaluate(token_t *, Environment *);

token_t * apply_op(string op, string a, string b) {
  token_t * tok = new token_t;
  tok->type = "num";
  if (op == "+")       tok->value = to_string(stoi(a) + stoi(b));
  else if (op == "-")  tok->value = to_string(stoi(a) - stoi(b));
  else if (op == "*")  tok->value = to_string(stoi(a) * stoi(b));
  else if (op == "/")  tok->value = to_string(stoi(a) / stoi(b));
  else if (op == "%")  tok->value = to_string(stoi(a) % stoi(b));
  else if (op == "<")  tok->value = to_string(stoi(a) < stoi(b));
  else if (op == ">")  tok->value = to_string(stoi(a) > stoi(b));
  else if (op == "<=") tok->value = to_string(stoi(a) <= stoi(b));
  else if (op == ">=") tok->value = to_string(stoi(a) >= stoi(b));
  else if (op == "==") tok->value = to_string(a == b);
  else if (op == "!=") tok->value = to_string(a != b);
  else tok->type = "bool";

  if (tok->type == "bool") {
    if (op == "&&")      tok->value = a != "false" && b != "false" ? "true" : "false";
    else if (op == "||") tok->value = a != "false" || b != "false" ? "true" : "false";
    else {
        cout << "Error: can't apply operator " << op << endl;
        exit(0);
    }
  }

  return tok;
  //throw new Error("Can't apply operator " + op);
}

token_t * make_lambda(token_t * exp, Environment * env) {
  token_t * tok = new token_t;
  tok->type = "var";
  tok->exe = [&](vector<string> args) {
    cout << "actually called" << endl;
    vector<token_t *> names = exp->vars;
    Environment * scope = env->extend();
    for (int i = 0; i < names.size(); ++i)
      scope->let(names.at(i)->value, i < args.size() ? args.at(i) : "false");
    return evaluate(exp->body, scope);
  };
  return tok;
}

token_t * evaluate(token_t * exp, Environment * env) {
  string t = exp->type;
  if (t == "num" || t == "str" || t == "bool") {
    return exp;
  }
  else if (t == "var") {
    return env->get(exp->value);
  }
  else if (t == "assign") {
    if (exp->left->type != "var") {
      cout << "Cannot assign to " << exp->left->value << endl;
      exit(0);
    }
      //throw new Error("Cannot assign to " + JSON.stringify(exp.left));
    return env->let(exp->left->value, evaluate(exp->right, env)->value);
  }
  else if (t == "binary") {
    return apply_op(exp->op, evaluate(exp->left, env)->value, evaluate(exp->right, env)->value);
  }
  else if (t == "lambda") {
    return make_lambda(exp, env);
  }
  else if (t == "if") {
    token_t * cond = evaluate(exp->cond, env);
    if (cond != nullptr) return evaluate(exp->then, env);
    else if (exp->else_cond != nullptr) return evaluate(exp->else_cond, env);
    return nullptr;
  }
  else if (t == "prog") {
    token_t * val = nullptr;
    for (token_t * tok : exp->prog)
      val = evaluate(tok, env);
    return val;
  }
  else if (t == "call") {
    cout << "maybe " << exp->func->type << endl;
    token_t * func = evaluate(exp->func, env);
    cout << "calling " << func->value << endl;
    vector<string> args;
    for (token_t * arg : exp->args)
      args.push_back(evaluate(arg, env)->value);
    for (auto arg : args) cout << arg << endl;

    func->exe(args);
    return func;
  }
  cout << "Error: can't evaluate " << exp->type << endl;
  return nullptr;
}

int main(int argc, char * argv[]) {
  string line, lines;
  char input[100];
  ifstream bert_file (argv[1]);
  while(getline(bert_file, line)) lines += line;
  strcpy(input, lines.c_str());
  Parser p = Parser(TokenStream(InputStream(input)));
  token_t * ast = p.parse_toplevel();
  Environment * e = new Environment(ast);
  e->let("println", [](vector<string> args) {
    for (string arg : args) cout << arg;
    cout << endl;
  });
  evaluate(ast, e);
}
