struct token_t;

typedef map<string, int> precedence_t;
typedef function<void(vector<string>)> lambda;

struct token_t {
  string type;
  string value;
  string op;
  token_t * left;
  token_t * right;
  token_t * func;
  token_t * cond;
  token_t * then;
  token_t * else_cond;
  token_t * body;
  token_t * parent;
  vector<token_t *> vars;
  vector<token_t *> args;
  vector<token_t *> prog;
  map<string, lambda> builtins;
  lambda exe;
};

template <typename T, typename U>
struct base_callback {
  virtual T call(U) = 0;
};
