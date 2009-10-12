%extend IMP::Decorator {
  bool __eq__(Decorator o) {
    return *self == o;
  }
  bool __eq__(Particle* o) {
    return *self == o;
  }
}

%ignore IMP::Decorators::operator[];
%extend IMP::Decorators {
  Decorator __getitem__(int index) const {
    if (index < 0) index= index+self->size();
    if (index >= static_cast<int>(self->size())) {
       IMP_FAILURE("Index out of range in getitem" << index
       << self->size(), IMP::IndexException);
    }
    return self->operator[](index);
  }
  void __setitem__(int index, Decorator val) {
    if (index < 0) index= index+self->size();
    if (index >= static_cast<int>(self->size())) {
       IMP_FAILURE("Index out of range in setitem" << index
       << self->size(), IMP::IndexException);
    }
    self->set(index, val);
  }
  int __len__() const {
    return self->size();
  }
  void append(Decorator d) {
    self->push_back(d);
  }
  Decorators< Decorator, ParentDecorators >
   __add__(const Decorators< Decorator, ParentDecorators > &o) {
    IMP::Decorators< Decorator, ParentDecorators > ret(*self);
    ret.insert(ret.end(), o.begin(), o.end());
    return ret;
  }
  std::vector<Decorator> __list__() const {
    std::vector<Decorator> ret(self->begin(), self->end());
    return ret;
  }
  Decorators< Decorator, ParentDecorators > __getslice__(int b, int e) const {
    if (e < 0) e= self->size()+e;
    if (b < 0) b= self->size()+b;
    IMP::Decorators< Decorator, ParentDecorators > ret;
    for ( int c=b; c!= e; ++c) {
       ret.push_back(self->operator[](c));
    }
    return ret;
  }
}

%ignore IMP::DecoratorsWithTraits::operator[];
%extend IMP::DecoratorsWithTraits {
  Decorator __getitem__(int index) const {
    if (index < 0) index=self->size()+index;
    if (index >= static_cast<int>(self->size())) {
       IMP_FAILURE("Index out of range in getitem" << index
       << self->size(), IMP::IndexException);
    }
    return self->operator[](index);
  }
  std::vector<Decorator> __list__() const {
    std::vector<Decorator> ret(self->begin(), self->end());
    return ret;
  }
  void __setitem__(int index, Decorator val) {
    if (index < 0) index=self->size()+index;
    if (index >= static_cast<int>(self->size())) {
       IMP_FAILURE("Index out of range in setitem" << index
       << self->size(), IMP::IndexException);
    }
    self->set(index, val);
  }
  int __len__() const {
    return self->size();
  }
  void append(Decorator d) {
    self->push_back(d);
  }
  DecoratorsWithTraits< Decorator, ParentDecorators, Traits >
  __add__(const DecoratorsWithTraits<Decorator, ParentDecorators, Traits > &o) {
    IMP::DecoratorsWithTraits< Decorator, ParentDecorators, Traits > ret(*self);
    ret.insert(ret.end(), o.begin(), o.end());
    return ret;
  }
  DecoratorsWithTraits< Decorator, ParentDecorators, Traits >
  __getslice__(int b, int e) const {
    if (e < 0) e= self->size()+e;
    if (b < 0) b= self->size()+b;
    IMP::DecoratorsWithTraits< Decorator, ParentDecorators, Traits > ret;
    for ( int c=b; c!= e; ++c) {
       ret.push_back(self->operator[](c));
    }
    return ret;
  }
}

%define IMP_DECORATORS(Name, PluralName, Parent)
%template(PluralName) ::IMP::Decorators< Name, Parent>;
%template(Name##Vector) ::std::vector<Name>;
%implicitconv Name ;
%enddef
%implicitconv Decorator;

