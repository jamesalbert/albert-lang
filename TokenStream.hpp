class TokenStream {
private:
  token_t * current = nullptr;
  string keywords = " if then else lambda λ true false";
  InputStream input;
  // static methods //
  static bool is_digit(char ch) {
    regex reg("[0-9]");
    return regex_match(string(1, ch), reg);
  }
  static bool is_id_start(char ch) {
    regex reg("[a-zA-zλ_]");
    return regex_match(string(1, ch), reg);
  }
  static bool is_op_char(char ch) {
    return contains("+-*/%=&|<>!", string(1, ch));
  }
  static bool is_punc(char ch) {
    return contains(",;(){}[]", string(1, ch));
  }
  static bool is_whitespace(char ch) {
    return contains(" \t\n", string(1, ch));
  }
  // non-static methods //
  /*
  bool is_id(char ch) {
    return is_id_start(ch) || contains("?!-<>=0123456789", string(1, ch));
  }
  */
  bool is_keyword(string x) {
    return contains(keywords, x);
  }
  string read_while(base_callback<bool, char> * predicate) {
    string str;
    while (!input.eof() && predicate->call(input.peek()))
      str += input.next();
    return str;
  }
  token_t * read_number() {
    struct read_number_predicate : base_callback<bool, char> {
      virtual bool call(char ch) {
        bool has_dot = false;
        if (ch == '.') {
          if (has_dot) return false;
          has_dot = true;
          return true;
        }
        return is_digit(ch);
      }
    } rnp;
    string number = read_while(&rnp);
    token_t * ret = new token_t;
    ret->type = "num";
    ret->value = number;
    return ret;
  }
  token_t * read_ident() {
    struct read_ident_predicate : base_callback<bool, char> {
      virtual bool call(char ch) {
        return is_id_start(ch) || contains("?!-<>=0123456789", string(1, ch));
      }
    } rip;
    string id = read_while(&rip);
    token_t * ret = new token_t;
    ret->type = is_keyword(id) ? "kw" : "var";
    ret->value = id;
    return ret;
  }
  string read_escaped(char end) {
    bool escaped = false;
    string str = "";
    input.next();
    while (!input.eof()) {
      char ch = input.next();
      if (escaped) {
        str += ch;
        escaped = false;
      } else if (ch == '\\') {
        escaped = true;
      } else if (ch == end) {
        break;
      } else {
        str += ch;
      }
    }
    return str;
  }
  token_t * read_string() {
    token_t * ret = new token_t;
    ret->type = "str";
    ret->value = read_escaped('"');
    return ret;
  }
  void skip_comment() {
    struct skip_comment_predicate : base_callback<bool, char> {
      virtual bool call(char ch) {
        return ch != '\n';
      }
    } scp;
    read_while(&scp);
    input.next();
  }
  token_t * read_next() {
    token_t * ret = new token_t;
    struct whitespace_predicate : base_callback<bool, char> {
      virtual bool call(char ch) {
        return is_whitespace(ch);
      }
    } wp;
    struct is_op_char_predicate : base_callback<bool, char> {
      virtual bool call(char ch) {
        return is_op_char(ch);
      }
    } ocp;
    read_while(&wp);
    if (input.eof()) return nullptr;
    char ch = input.peek();
    if (ch == '#') {
        skip_comment();
        return read_next();
    }
    if (ch == '"') return read_string();
    if (is_digit(ch)) return read_number();
    if (is_id_start(ch)) return read_ident();
    if (is_punc(ch)) {
      ret->type = "punc";
      ret->value = string(1, input.next());
      return ret;
    }
    if (is_op_char(ch)) {
      token_t * ret = new token_t;
      ret->type = "op";
      ret->value = read_while(&ocp);
      return ret;
    }
    input.croak("Can't handle character: " + string(1, ch));
    return nullptr;
  }
public:
  TokenStream(InputStream input) : input(input) {}
  token_t * peek() {
    return current != nullptr ? current : (current = read_next());
  }
  token_t * next() {
    token_t * tok = current;
    current = nullptr;
    return tok != nullptr ? tok : read_next();
  }
  bool eof() {
    return peek() == nullptr;
  }
  void croak(string msg) {
    input.croak(msg);
  }
  ~TokenStream() {
    //delete current;
  }
};
