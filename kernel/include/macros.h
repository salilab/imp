/**
 *  \file IMP/macros.h    \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_MACROS_H
#define IMP_MACROS_H

//! Implement comparison in a class using a compare function
/** The macro requires that This be defined as the type of the current class.
    The compare function should take a const This & and return -1, 0, 1 as
    appropriate.
 */
#define IMP_COMPARISONS                                                 \
  /** */ bool operator==(const This &o) const {                         \
    return (compare(o) == 0);                                           \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (compare(o) != 0);                                           \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (compare(o) <0);                                             \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (compare(o) > 0);                                            \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return !(compare(o) < 0);                                           \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return !(compare(o) > 0);                                           \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_1(field)                                        \
  /** */ bool operator==(const This &o) const {                         \
    return (field== o.field);                                           \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (field!= o.field);                                           \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field< o.field);                                            \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field> o.field);                                            \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field>= o.field);                                           \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return (field<= o.field);                                           \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_2(f0, f1)                                       \
  /** */ bool operator==(const This &o) const {                         \
    return (f0== o.f0 && f1==o.f1);                                     \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (f0!= o.f0 || f1 != o.f1);                                   \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    else return f1 < o.f1;                                              \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    else return f1 > o.f1;                                              \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator<(o) || operator==(o);                               \
  }

//! Implement comparison in a class using field as the variable to compare
/** The macro requires that This be defined as the type of the current class.
 */
#define IMP_COMPARISONS_3(f0, f1, f2)                                   \
  /** */ bool operator==(const This &o) const {                         \
    return (f0== o.f0 && f1==o.f1 && f2 == o.f2);                       \
  }                                                                     \
  /** */ bool operator!=(const This &o) const {                         \
    return (f0!= o.f0 || f1 != o.f1 || f2 != o.f2);                     \
  }                                                                     \
  /** */ bool operator<(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0< o.f0) return true;                                          \
    else if (f0 > o.f0) return false;                                   \
    if (f1< o.f1) return true;                                          \
    else if (f1 > o.f1) return false;                                   \
    else return f2 < o.f2;                                              \
  }                                                                     \
  /** */ bool operator>(const This &o) const {                          \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    if (f0 > o.f0) return true;                                         \
    else if (f0 < o.f0) return false;                                   \
    if (f1 > o.f1) return true;                                         \
    else if (f1 < o.f1) return false;                                   \
    else return f2 > o.f2;                                              \
  }                                                                     \
  /** */ bool operator>=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator>(o) || operator==(o);                               \
  }                                                                     \
  /** */ bool operator<=(const This &o) const {                         \
    IMP_assert(!is_default() && !o.is_default(),                        \
               "Ordering with uninitialized index is undefined");       \
    return operator<(o) || operator==(o);                               \
  }

//! Implement operator<< on class name, assuming it has one template argument
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_1(name) /** write to a stream*/             \
template <class L>                                                      \
 inline std::ostream& operator<<(std::ostream &out, const name<L> &i)   \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }

//! Implement operator<< on class name, assuming it has two template arguments
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR_2(name) /** write to a stream*/             \
  template <class L, class M>                                           \
  inline std::ostream& operator<<(std::ostream &out, const name<L, M> &i) \
  {                                                                     \
    i.show(out);                                                        \
    return out;                                                         \
  }

//! Implement operator<< on class name
/** class name should also define the method std::ostream &show(std::ostream&)
 */
#define IMP_OUTPUT_OPERATOR(name)   /** write to a stream*/             \
  inline std::ostream &operator<<(std::ostream &out, const name &i)     \
  {                                                                     \
    i.show(out);                                                 \
    return out;                                                  \
  }

//! Define the basic things you need for a Restraint.
/** These are: show, evaluate, get_version_info
    \param[in] version_info The version info object to return.
*/
#define IMP_RESTRAINT(version_info)                       \
  /** evaluate the restraint*/                                          \
  virtual Float evaluate(DerivativeAccumulator *accum);                 \
  /** write information about the restraint to the stream*/             \
  virtual void show(std::ostream &out=std::cout) const;                 \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Define the basic things you need for an optimizer.
/** These are: optimize, get_version_info
    \param[in] version_info The version info object to return.
*/
#define IMP_OPTIMIZER(version_info)                                     \
  /** \short Optimize the model.
      \param[in] max_steps The maximum number of steps to take.
      \return The final score.
   */                                                                   \
  virtual Float optimize(unsigned int max_steps);                       \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }


//! Define the basics needed for an OptimizerState
/** This macro declares the required functions
    - void update()
    - void show(std::ostream &out) const
    and defines the function
    - get_version_info

    \param[in] version_info The version info object to return.
*/
#define IMP_OPTIMIZER_STATE(version_info)                               \
  /** update the state*/                                                \
  virtual void update();                                                \
  /** write information about the state to the stream*/                 \
  virtual void show(std::ostream &out=std::cout) const;                 \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Define the basics needed for a ScoreState
/** This macro declares the required functions
    - void do_before_evaluate()
    - void show(std::ostream &out) const
    and defines the function
    - get_version_info

    \param[in] version_info The version info object to return.
*/
#define IMP_SCORE_STATE(version_info)                                   \
protected:                                                              \
  /** update the state*/                                                \
 virtual void do_before_evaluate();                                     \
public:                                                                 \
 /** write information about the state to the stream */                 \
 virtual void show(std::ostream &out=std::cout) const;                  \
  /** \return version and authorship information */                     \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Define the basics needed for a particle refiner
/** This macro declares the following functions
    - bool can_refine(Particle*) const;
    - void cleanup(Particle *a, Particles &b,
                   DerivativeAccumulator *da);
    - void show(std::ostream &out) const;
    - Particles refine(Particle *) const;

    \param[in] version_info The version info object to return

 */
#define IMP_PARTICLE_REFINER(version_info)                              \
  public:                                                               \
  /** Return if the particle can be refined*/                           \
  virtual bool get_can_refine(Particle*) const;                         \
  /** Write information about the state to the stream */                \
  virtual void show(std::ostream &out) const;                           \
  /** Destroy any created particles and propagate derivatives */        \
  virtual void cleanup_refined(Particle *a, Particles &b,               \
                               DerivativeAccumulator *da=0) const;      \
  /** Return a list of particles which refines the passed particle.*/   \
  virtual Particles get_refined(Particle *) const;                      \
  virtual IMP::VersionInfo get_version_info() const { return version_info; }

//! Use the swap_with member function to swap two objects
#define IMP_SWAP(name) \
  inline void swap(name &a, name &b) {          \
    a.swap_with(b);                             \
  }

//! swap two member variables assuming the other object is called o
#define IMP_SWAP_MEMBER(var_name) \
  std::swap(var_name, o.var_name);



//! use a copy_from method to create a copy constructor and operator=
#define IMP_COPY_CONSTRUCTOR(TC) TC(const TC &o){copy_from(o);}  \
  TC& operator=(const TC &o) {copy_from(o); return *this;}



#ifdef _MSC_VER
// VC doesn't understand friends properly
#define IMP_REF_COUNTED_DESTRUCTOR(Classname)                   \
public:                                                         \
virtual ~Classname();
#else

#ifdef SWIG
// SWIG doesn't do friends right either, but we don't care as much
#define IMP_REF_COUNTED_DESTRUCTOR(Classname)                   \
protected:                                                      \
    virtual ~Classname();
#else
//! Declare a protected destructor and get the friends right
/** The destructor is unprotected for SWIG since if it is protected
    SWIG does not wrap the python proxy distruction and so does not
    dereference the ref counted pointer. Swig also gets confused
    on template friends.
 */
#define IMP_REF_COUNTED_DESTRUCTOR(Classname)                   \
  protected:                                                    \
  template <class T> friend void IMP::internal::disown(T*);     \
  friend class IMP::internal::UnRef<true>;                      \
  virtual ~Classname();
#endif // SWIG
#endif // _MSC_VER

#endif  /* IMP_MACROS_H */
