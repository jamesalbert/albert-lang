class InputStream {
private:
  int pos  = 0,
      line = 1,
      col  = 0;
  char* input;
public:
  InputStream(char* input) : input(input) {}
  char next() {
    char ch = input[pos++];
    if (ch == '\n') {
      line++;
      col = 0;
    } else col++;
    return ch;
  }
  char peek() {
    return input[pos];
  }
  bool eof() {
    // this might not work
    return peek() == '\0';
  }
  void croak(string msg) {
    cout << "CROAK: " << msg << endl;
    exit(0);
    //throw std::runtime_error(msg);
  }
};
