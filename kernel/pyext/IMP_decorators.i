
%extend IMP::Decorator {
  bool __eq__(Decorator o) {
    return *self == o;
  }
  bool __eq__(Particle* o) {
    return *self == o;
  }
}


%feature("shadow") IMP::DecoratorsWithTraits::__iter__ %{
def __iter__(self):
    return IMP.DecoratorIterator(self)
%}
%feature("shadow") IMP::Decorators::__iter__ %{
def __iter__(self):
    return IMP.DecoratorIterator(self)
%}

%ignore IMP::Decorators::operator[];
%extend IMP::Decorators {
  WrappedDecorator __getitem__(int index) const {
    if (index < 0) index= index+self->size();
    if (index >= static_cast<int>(self->size())) {
       IMP_THROW("Index out of range in getitem" << index
       << self->size(), IMP::IndexException);
    }
    return self->operator[](index);
  }
  void __setitem__(int index, WrappedDecorator val) {
    if (index < 0) index= index+self->size();
    if (index >= static_cast<int>(self->size())) {
       IMP_THROW("Index out of range in setitem" << index
       << self->size(), IMP::IndexException);
    }
    self->set(index, val);
  }
  int __len__() const {
    return self->size();
  }
  void append(WrappedDecorator d) {
    self->push_back(d);
  }
  void __iter__() const{}
  Decorators< WrappedDecorator, ParentDecorators >
   __add__(const Decorators< WrappedDecorator, ParentDecorators > &o) {
    IMP::Decorators< WrappedDecorator, ParentDecorators > ret(*self);
    ret.insert(ret.end(), o.begin(), o.end());
    return ret;
  }
  std::vector<WrappedDecorator> __list__() const {
    std::vector<WrappedDecorator> ret(self->begin(), self->end());
    return ret;
  }
  Decorators< WrappedDecorator, ParentDecorators > __getslice__(int b, int e) const {
    if (e < 0) e= self->size()+e;
    if (b < 0) b= self->size()+b;
    IMP::Decorators< WrappedDecorator, ParentDecorators > ret;
    for ( int c=b; c!= e; ++c) {
       ret.push_back(self->operator[](c));
    }
    return ret;
  }
}

%ignore IMP::DecoratorsWithTraits::operator[];
%extend IMP::DecoratorsWithTraits {
  WrappedDecorator __getitem__(int index) const {
    if (index < 0) index=self->size()+index;
    if (index >= static_cast<int>(self->size())) {
       IMP_THROW("Index out of range in getitem" << index
       << self->size(), IMP::IndexException);
    }
    return self->operator[](index);
  }
  std::vector<WrappedDecorator> __list__() const {
    std::vector<WrappedDecorator> ret(self->begin(), self->end());
    return ret;
  }
  void __setitem__(int index, WrappedDecorator val) {
    if (index < 0) index=self->size()+index;
    if (index >= static_cast<int>(self->size())) {
       IMP_THROW("Index out of range in setitem" << index
       << self->size(), IMP::IndexException);
    }
    self->set(index, val);
  }
  int __len__() const {
    return self->size();
  }
  void append(WrappedDecorator d) {
    self->push_back(d);
  }
  void __iter__(){}
  DecoratorsWithTraits< WrappedDecorator, ParentDecorators, Traits >
  __add__(const DecoratorsWithTraits<WrappedDecorator, ParentDecorators, Traits > &o) {
    IMP::DecoratorsWithTraits< WrappedDecorator, ParentDecorators, Traits > ret(*self);
    ret.insert(ret.end(), o.begin(), o.end());
    return ret;
  }
  DecoratorsWithTraits< WrappedDecorator, ParentDecorators, Traits >
  __getslice__(int b, int e) const {
    if (e < 0) e= self->size()+e;
    if (b < 0) b= self->size()+b;
    IMP::DecoratorsWithTraits< WrappedDecorator, ParentDecorators, Traits > ret;
    for ( int c=b; c!= e; ++c) {
       ret.push_back(self->operator[](c));
    }
    return ret;
  }
}


%define IMP_SWIG_DECORATORS(Name, PluralName, Parent)
%template(PluralName) ::IMP::Decorators< Name, Parent>;
%template(PluralName##Temp) ::IMP::Decorators<Name, Parent##Temp>;
%implicitconv Name ;
%enddef
%implicitconv Decorator;

