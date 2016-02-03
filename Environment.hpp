class Environment {
private:
  vector<token_t *> vars;
  token_t * parent;
public:
  Environment(token_t * parent)
  : vars(parent == nullptr ? vector<token_t *>() : parent->vars),
    parent(parent) {};

  Environment * extend() {
    return new Environment(parent);
  }
  token_t * lookup(string name) {
    token_t * scope = parent, * tok;
    while (scope != nullptr) {
      if ((tok = find_token(scope->vars, name)) != nullptr)
        return scope;
      scope = scope->parent;
    }
    return nullptr;
  }
  token_t * get(string name) {
    token_t * tok;
    if ((tok = find_token(vars, name)) != nullptr)
      return tok;
    cout << "Undefined variable " + name << endl;
    exit(0);
    return nullptr;
    //throw new Error("Undefined variable " + name);
  }
  token_t * set(string name, string value) {
    // returns scope in which name exists
    token_t * scope = lookup(name), * tok;
    if (scope != nullptr && parent != nullptr) {
      cout << "Undefined variable " + name << endl;
      exit(0);
    }
      //throw new Error("Undefined variable " + name);
    scope = scope == nullptr ? parent : scope;

    tok = find_token(scope->vars, name);
    tok->value = value;
    return tok;
  }
  token_t * let(string name, string value) {
    token_t * tok = new token_t;
    tok->type = "var";
    tok->value = name;
    vars.push_back(tok);
    return tok;
  }

  token_t * let(string name, lambda func) {
    token_t * tok = new token_t;
    tok->type = "var";
    tok->value = name;
    tok->exe = func;
    vars.push_back(tok);
    return tok;
  }
};
