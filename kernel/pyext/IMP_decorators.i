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
    if (index < 0) index=self->size()+index;
    return self->operator[](index);
  }
  void __setitem__(int index, Decorator val) {
    if (index < 0) index=self->size()+index;
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
    return self->operator[](index);
  }
  std::vector<Decorator> __list__() const {
    std::vector<D> ret(self->begin(), self->end());
    return ret;
  }
  void __setitem__(int index, Decorator val) {
    if (index < 0) index=self->size()+index;
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
    IMP::DecoratorsWithTraits< D, P, Traits > ret(*self);
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

/*%typemap(in) IMP::Particle* {
  void *argp1=NULL;
  int res1 = SWIG_ConvertPtr($input, &argp1,SWIGTYPE_p_IMP__Particle, 0 |  0 );
  if (!SWIG_IsOK(res1)) {
    IMP::Decorator *d=0;
    int res2 = SWIG_ConvertPtr($input, &argp1, SWIGTYPE_p_IMP__Decorator, 0 | 0 );
    if (!SWIG_IsOK(res2)) {
       SWIG_exception_fail(SWIG_ArgError(res1), "in method '" "new_XYZR" "', argument " "1"" of type '" "IMP::Particle *""'");
    } else {
      d= reinterpret_cast<IMP::Decorator*>(argp1);
      $1= d->get_particle();
    }
  } else {
    $1=reinterpret_cast< IMP::Particle * >(argp1);
  }
}*/
