class Parser {
  TokenStream input;
  precedence_t PRECEDENCE;
public:
  Parser(TokenStream input) : input(input) {
    FALSE->type = "bool";
    FALSE->value = "false";
    PRECEDENCE["="] = 1;
    PRECEDENCE["||"] = 2;
    PRECEDENCE["&&"] = 3;
    PRECEDENCE["<"] = 7;
    PRECEDENCE[">"] = 7;
    PRECEDENCE["<="] = 7;
    PRECEDENCE[">="] = 7;
    PRECEDENCE["=="] = 7;
    PRECEDENCE["!="] = 7;
    PRECEDENCE["+"] = 10;
    PRECEDENCE["-"] = 10;
    PRECEDENCE["*"] = 20;
    PRECEDENCE["/"] = 20;
    PRECEDENCE["%"] = 20;
  }
  token_t * is_punc(char ch='\0') {
    token_t * tok = input.peek();
    if (tok != nullptr &&
       tok->type == "punc" &&
       (ch == '\0' || tok->value == string(1, ch)))
      return tok;
    else
      return nullptr;
  }
  token_t * is_kw(string kw) {
    token_t * tok = input.peek();
    if (tok != nullptr &&
       tok->type == "kw" &&
       (kw == " " || tok->value == kw))
      return tok;
    else
      return nullptr;
  }
  token_t * is_op(char op='\0') {
    token_t * tok = input.peek();
    if (tok != nullptr &&
       tok->type == "op" &&
       (op == '\0' || tok->value == string(1, op)))
      return tok;
    else
      return nullptr;
  }
  void skip_punc(char ch) {
    if (is_punc(ch) != nullptr) input.next();
    else input.croak("Expecting punctuation: \"" + string(1, ch) + "\"");
  }
  void skip_kw(string kw) {
    if (is_kw(kw) != nullptr) input.next();
    else input.croak("Expecting keyword: \"" + kw + "\"");
  }
  void skip_op(char op) {
    if (is_op(op) != nullptr) input.next();
    else input.croak("Expecting operator: \"" + string(1, op) + "\"");
  }
  void unexpected() {
    input.croak("Unexpected token: " + input.peek()->value);
  }
  token_t * maybe_binary(token_t * left, int my_prec) {
    token_t * tok = is_op();
    if (tok != nullptr) {
      int his_prec = PRECEDENCE[tok->value];
      if (his_prec > my_prec) {
        input.next();
        token_t * ret = new token_t;
        ret->type = tok->value == "=" ? "assign" : "binary";
        ret->op = tok->value;
        ret->left = left;
        ret->right = maybe_binary(parse_atom(), his_prec);
        return maybe_binary(ret, my_prec);
      }
    }
    return left;
  }
  vector<token_t *> delimited(char start, char stop, char separator, base_callback<token_t *, Parser *> * parser) { //string (*parser)()) {
    vector<token_t *> a;
    bool first = true;
    skip_punc(start);
    while (!input.eof()) {
      if (is_punc(stop)) break;
      if (first) first = false;
      else skip_punc(separator);
      if (is_punc(stop)) break;
      a.push_back(parser->call(this));
    }
    skip_punc(stop);
    return a;
  }
  token_t * parse_call(token_t * func) {
    struct parse_call_callback : base_callback<token_t *, Parser *> {
      virtual token_t * call(Parser * self) {
        return self->parse_expression();
      }
    } pcc;
    token_t * ret = new token_t;
    ret->type = "call";
    ret->func = func;
    ret->args = delimited('(', ')', ',', &pcc);
    return ret;
  }
  token_t * parse_varname() {
    token_t * name = input.next();
    if (name->type != "var") input.croak("Expecting variable name");
    return name;
  }
  token_t * parse_if() {
    skip_kw("if");
    token_t * cond = parse_expression();
    if (is_punc('{') == nullptr)
      skip_kw("then");
    token_t * then = parse_expression();
    token_t * ret = new token_t;
    ret->type = "if";
    ret->cond = cond;
    ret->then = then;
    if (is_kw("else") != nullptr) {
      input.next();
      ret->else_cond = parse_expression();
    }
    return ret;
  }
  token_t * parse_lambda() {
    struct parse_lambda_callback : base_callback<token_t *, Parser *> {
      virtual token_t * call(Parser * self) {
        return self->parse_varname();
      }
    } plc;
    token_t * ret = new token_t;
    ret->type = "lambda";
    ret->vars = delimited('(', ')', ',', &plc);
    ret->body = parse_expression();
    return ret;
  }
  token_t * parse_bool() {
    token_t * ret = new token_t;
    ret->type = "bool";
    ret->value = input.next()->value == "true";
    return ret;
  }
  token_t * maybe_call(base_callback<token_t *, Parser *> * _expr) {
    token_t * expr = _expr->call(this);
    return is_punc('(') ? parse_call(expr) : expr;
  }
  token_t * parse_atom() {
    struct parse_atom_callback : base_callback<token_t *, Parser *> {
      virtual token_t * call(Parser * self) {
        if (self->is_punc('(')) {
          self->input.next();
          token_t * exp = self->parse_expression();
          self->skip_punc(')');
          return exp;
        }
        if (self->is_punc('{')) return self->parse_prog();
        if (self->is_kw("if")) return self->parse_if();
        if (self->is_kw("true") || self->is_kw("false")) return self->parse_bool();
        if (self->is_kw("lambda") || self->is_kw("λ")) {
          self->input.next();
          return self->parse_lambda();
        }
        token_t * tok = self->input.next();
        if (tok->type == "var" || tok->type == "num" || tok->type == "str")
          return tok;
        self->unexpected();
        return nullptr;
      }
    } pac;
    return maybe_call(&pac);
  }
  token_t * parse_toplevel() {
    token_t * ret = new token_t;
    vector<token_t *> prog;
    while (!input.eof()) {
      prog.push_back(parse_expression());
      if (!input.eof()) skip_punc(';');
    }
    ret->type = "prog";
    ret->prog = prog;
    return ret;
  }
  token_t * parse_prog() {
    struct parse_prog_callback : base_callback<token_t *, Parser *> {
      virtual token_t * call(Parser * self) {
        return self->parse_expression();
      }
    } ppc;
    vector<token_t *> prog = delimited('{', '}', ';', &ppc);
    if (prog.size() == 0) return FALSE;
    if (prog.size() == 1) return prog[0];
    token_t * ret = new token_t;
    ret->type = "prog";
    ret->prog = prog;
    return ret;
  }
  token_t * parse_expression() {
    struct parse_expression_callback : base_callback<token_t *, Parser *> {
      virtual token_t * call(Parser * self) {
        return self->maybe_binary(self->parse_atom(), 0);
      }
    } pec;
    return maybe_call(&pec);
  }
};
