/**
 *  \file IMP/optimizer_state_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OPTIMIZER_STATE_MACROS_H
#define IMPKERNEL_OPTIMIZER_STATE_MACROS_H
#include "kernel_config.h"
#include <IMP/base/value_macros.h>
#include "OptimizerState.h"


//! Define the basics needed for an OptimizerState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::OptimizerState::update()
*/
#define IMP_OPTIMIZER_STATE(Name)               \
  virtual void update();                        \
  IMP_OBJECT(Name)

//! Define the basics needed for an OptimizerState which acts every n steps
/** In addition to the methods done by all the macros, it declares
    - do_update(unsigned int call_number) where step number
    is the number of the optimization step, and call_number is the number
    of the call to do_update.
    It also defines
    - void set_periodicity(unsigned int)
    - void reset() which resets all state (like the current frame number)

    If you use this macro, you should also include IMP/internal/utility.h.
*/
#define IMP_PERIODIC_OPTIMIZER_STATE(Name)                              \
  virtual void update() {                                               \
    IMP_OBJECT_LOG;                                                     \
    ++call_number_;                                                     \
    if (call_number_%(skip_+1) ==0) {                                   \
      do_update(update_number_);                                        \
      ++update_number_;                                                 \
    }                                                                   \
  }                                                                     \
  void do_update(unsigned int call_number);                             \
  void set_is_optimizing(bool tf) {                                     \
    if (!tf) {                                                          \
      do_update(update_number_);                                        \
      ++update_number_;                                                 \
    }                                                                   \
    else call_number_=0;                                                \
  }                                                                     \
  IMP_NO_DOXYGEN(void set_skip_steps(unsigned int k) {set_period(k+1);}); \
  void set_period(unsigned int p) {                                     \
    IMP_USAGE_CHECK(p>0, "Period must be positive.");                   \
    skip_=p-1; call_number_=0;                                          \
  }                                                                     \
  void reset() {                                                        \
    call_number_=0;                                                     \
  }                                                                     \
  IMP_OBJECT(Name);                                                     \
  private:                                                              \
  ::IMP::internal::Counter skip_, call_number_, update_number_          \




//! Define a pair of classes to handle saving the model
/** This macro defines two classes:
 - NameOptimizerState
 - NameFailureHandler
 to handling saving the model in the specified way during
 optimization and on failures, respectively.
 \param[in] Name The name to prefix the class names
 \param[in] args The parentesized arguments to the constructor. The
   last one should be a string called file_name.
 \param[in] vars The needed member variables.
 \param[in] constr The body of the constructor.
 \param[in] functs Any functions (declaration and definition) to
            go in the class bodies.
 \param[in] save_action The action to take to perform the save. The
            name to save to is know as file_name

 The headers "IMP/OptimizerState.h", "IMP/FailureHandler.h", "boost/format.hpp"
 and "IMP/internal/utility.h" must be included.
 */
#define IMP_MODEL_SAVE(Name, args, vars, constr, functs, save_action)   \
  class Name##OptimizerState: public OptimizerState {                   \
    ::IMP::internal::Counter skip_steps_, call_number_, update_number_; \
    std::string file_name_;                                             \
    vars                                                                \
    IMP_IMPLEMENT_INLINE(virtual void update(), {                       \
      if (call_number_%(skip_steps_+1) ==0) {                           \
        std::ostringstream oss;                                         \
        bool formatted=false;                                           \
        try {                                                           \
          oss << boost::format(file_name_) % update_number_;            \
          formatted=true;                                               \
        } catch(...){                                                   \
        }                                                               \
        if (formatted) {                                                \
          write(oss.str(), false);                                      \
        } else {                                                        \
          write(file_name_, update_number_!=0);                         \
        }                                                               \
        ++update_number_;                                               \
      }                                                                 \
      ++call_number_;                                                   \
      });                                                               \
  public:                                                               \
/** Write to a file generated from the passed filename every
skip_steps steps. The file_name constructor argument should contain
"%1%" if you don't want to write the same file each time.
*/                                                                      \
    Name##OptimizerState args :                                         \
    OptimizerState(std::string("Writer to ")+file_name),                \
      file_name_(file_name) {constr}                                    \
    functs                                                              \
    IMP_NO_DOXYGEN(void set_skip_steps(unsigned int k) {set_period(k+1);}); \
    void set_period(unsigned int p) {                                   \
      skip_steps_=p-1;                                                  \
      call_number_=0;                                                   \
    }                                                                   \
void write(std::string file_name, unsigned int call=0,                  \
           bool append=false) const {                                   \
  IMP_UNUSED(call); IMP_UNUSED(append);                                 \
  save_action                                                           \
    }                                                                   \
  private:                                                              \
IMP_IMPLEMENT(void do_update(unsigned int call_number));                \
    IMP_OBJECT_INLINE(Name##OptimizerState,                             \
                      out << "Write to " << file_name_ << std::endl;,); \
  };                                                                    \
IMP_OBJECTS(Name##OptimizerState, Name##OptimizerStates);               \
/** Write to a file when a failure occurs.*/                            \
class Name##FailureHandler: public base::FailureHandler {               \
  std::string file_name_;                                               \
  vars                                                                  \
  public:                                                               \
  Name##FailureHandler args :                                           \
  base::FailureHandler(std::string("Writer to ")+file_name),            \
    file_name_(file_name) {                                             \
    constr}                                                             \
  functs                                                                \
  IMP_IMPLEMENT_INLINE(void handle_failure(), {                         \
    const std::string file_name=file_name_;                             \
    bool append=false; unsigned int call=0;                             \
    IMP_UNUSED(append); IMP_UNUSED(call);                               \
    save_action                                                         \
      });                                                               \
  IMP_OBJECT_INLINE(Name##FailureHandler,                               \
                    out << "Write to " << file_name_ << std::endl;,);   \
};                                                                      \
IMP_OBJECTS(Name##FailureHandler, Name##FailureHandlers);


#endif  /* IMPKERNEL_OPTIMIZER_STATE_MACROS_H */
