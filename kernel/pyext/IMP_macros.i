/* Ignore shared object import/export stuff */
#define IMPDLLEXPORT
#define IMPDLLLOCAL
#define IMP_COMPARISONS_1(f) bool operator==(const This &o) const ; bool operator<(const This &o) const; bool operator>(const This &o) const; bool operator!=(const This &o) const; bool operator<=(const This &o) const; bool operator>=(const This &o) const;
#define IMP_OUTPUT_OPERATOR_1(f)
#define IMP_OUTPUT_OPERATOR(f)
#define IMP_RESTRAINT(a,b) \
   virtual Float evaluate(DerivativeAccumulator *accum);\
   virtual void show(std::ostream &out=std::cout) const;\
   virtual std::string version() const {return std::string(a);}\
   virtual std::string last_modified_by() const {return std::string(b);}
#define IMP_OPTIMIZER(a, b)                       \
  virtual Float optimize(unsigned int max_steps);\
  virtual std::string version() const {return std::string(a);};\
  virtual std::string last_modified_by() const {return std::string(b);};

#define IMP_DECORATOR_GET(a,b,c,d)
#define IMP_DECORATOR_GET_SET(n,a,t,r) r get_##n() const;\
                                        void set_##n(t);
#define IMP_DECORATOR_GET_SET_OPT(n,a,t,r,d) r get_##n() const;\
                                        void set_##n(t);
#define IMP_DECORATOR(Name, Parent, a,b) \
         public:\
         typedef Name This;\
                 public: Name(); \
                 static Name create(::IMP::Particle *p);\
                 static Name cast(::IMP::Particle *p);\
                 void show(std::ostream &out=std::cout, std::string pre="")
#define IMP_DECORATOR_ARRAY_DECL(a, b)
#define IMP_CONTAINER(Ucname, lcname, IndexType)                      \
  public:                                                              \
  IndexType add_##lcname(Ucname *obj);                                 \
  Ucname *get_##lcname(IndexType i) const ;                            \
  unsigned int number_of_##lcname##s() const {return lcname##_vector_.size();} \
const std::vector<Ucname*> &get_##lcname##s() const {\
      return lcname##_vector_;}\

