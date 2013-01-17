// Typemaps to allow Python director class methods to write to C++ std::ostream
// objects as if they were Python file-like objects

%include "cstring.i"

%cstring_input_binary(const char *osa_buf, int osa_len);
namespace std {
  %rename(_ostream) ostream;
  class ostream {
  public:
    void write(const char *osa_buf, int osa_len);
  private:
    ostream() {}
    ~ostream() {}
  };
}
