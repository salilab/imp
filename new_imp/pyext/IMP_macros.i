/* Ignore shared object import/export stuff */
#define IMPDLLEXPORT
#define IMPDLLLOCAL
#define IMP_COMPARISONS_1(f) bool operator==(const This &o) const ; bool operator<(const This &o) const; bool operator>(const This &o) const; bool operator!=(const This &o) const; bool operator<=(const This &o) const; bool operator>=(const This &o) const;
#define IMP_OUTPUT_OPERATOR_1(f)
#define IMP_OUTPUT_OPERATOR(f)
#define IMP_RESTRAINT(a,b) \
   virtual Float evaluate(DerivativeAccumulator *accum);\
   virtual void show(std::ostream &out) const;\
   virtual std::string version() const {return std::string(version_string);}\
   virtual std::string last_modified_by() const {return std::string(lmb_string);}
#define IMP_DECORATOR_GET(a,b,c,d)
#define IMP_DECORATOR_GET_SET(n,a,t,r) r get_##n() const;\
                                        void set_##n(t);
#define IMP_DECORATOR_GET_SET_OPT(n,a,t,r,d) r get_##n() const;\
                                        void set_##n(t);
#define IMP_DECORATOR(Name, Parent, a,b) \
         public:\
         typedef Name This;\
                 public: Name(); \
                 static Name create(Particle *p);\
                 static Name cast(Particle *p);\
                 void show(std::ostream &=std::cout, std::string pre="")
#define IMP_DECORATOR_ARRAY_DECL(a, b)
