namespace std {
template <class T,  int D>
struct array {
  array();
  T at(int i) const;
  int size() const;
};
}

%extend std::array<int, 2> {
  %pythoncode %{
     def __getitem__(self, d):
       if d >= self.size() or d < 0:
         raise IndexError()
       return self.at(d)

     def __len__(self):
       return self.size()
     %}
}
%template(IntRange) std::array<int,2>;
