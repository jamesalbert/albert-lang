token_t * FALSE = new token_t;

bool contains(string arg1, string arg2) {
  return arg1.find(arg2) != string::npos;
}

token_t * find_token(vector<token_t *> vars, string name) {
  for (token_t * var : vars){
    if (var->value == name) return var;
  }
  return nullptr;
}
