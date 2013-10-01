/**
 *  \file IMP/kernel/optimizer_state_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OPTIMIZER_STATE_MACROS_H
#define IMPKERNEL_OPTIMIZER_STATE_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/value_macros.h>
#include <IMP/base/utility_macros.h>
#include "internal/utility.h"
#include "OptimizerState.h"

/** \deprecated_at{2.1} Declare the methods directly.
*/
#define IMP_OPTIMIZER_STATE(Name)                                  \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the methods youself."); \
  virtual void update() IMP_OVERRIDE;                              \
  IMP_OBJECT_NO_WARNING(Name)

/** \deprecated_at{2.1} Functionality is built into OptimizerState.
*/
#define IMP_PERIODIC_OPTIMIZER_STATE(Name)                                    \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Use IMP::core::PeriodicOptimizerState.");  \
  virtual void update() {                                                     \
    IMP_OBJECT_LOG;                                                           \
    ++call_number_;                                                           \
    if (call_number_ % (skip_ + 1) == 0) {                                    \
      do_update(update_number_);                                              \
      ++update_number_;                                                       \
    }                                                                         \
  }                                                                           \
  IMP_PROTECTED_METHOD(void, do_update, (unsigned int call_number), , );      \
                                                                              \
 public: /** Called when an optimization begins. It resets the current call   \
            number                                                            \
             as well as making sure that the last frame is written.*/         \
  void set_is_optimizing(bool tf) {                                           \
    if (!tf) {                                                                \
      do_update(update_number_);                                              \
      ++update_number_;                                                       \
    } else                                                                    \
      call_number_ = 0;                                                       \
  }                                                                           \
  IMP_NO_DOXYGEN(void set_skip_steps(unsigned int k) { set_period(k + 1); }); \
  void set_period(unsigned int p) {                                           \
    IMP_USAGE_CHECK(p > 0, "Period must be positive.");                       \
    skip_ = p - 1;                                                            \
    call_number_ = 0;                                                         \
  }                                                                           \
  unsigned int get_period() const { return skip_ + 1; }                       \
  void reset() {                                                              \
    call_number_ = 0;                                                         \
    update_number_ = 0;                                                       \
  }                                                                           \
  IMP_OBJECT_NO_WARNING(Name);                                                \
                                                                              \
 private:                                                                     \
  ::IMP::kernel::internal::Counter skip_, call_number_, update_number_

//! Define a pair of classes to handle saving the model
/** This macro defines a class:
 - NameOptimizerState
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
 */
#define IMP_MODEL_SAVE(Name, args, vars, constr, functs, save_action)          \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Just expand in place.");                    \
  class Name##OptimizerState : public OptimizerState {                         \
    std::string file_name_;                                                    \
    vars virtual void do_update(unsigned int update_number) IMP_OVERRIDE {     \
      std::ostringstream oss;                                                  \
      bool formatted = false;                                                  \
      try {                                                                    \
        oss << boost::format(file_name_) % update_number;                      \
        formatted = true;                                                      \
      }                                                                        \
      catch (...) {                                                            \
      }                                                                        \
      if (formatted) {                                                         \
        write(oss.str(), update_number, false);                                \
      } else {                                                                 \
        write(file_name_, update_number, update_number != 0);                  \
      }                                                                        \
    }                                                                          \
                                                                               \
   public: /** Write to a file generated from the passed filename every        \
           skip_steps steps. The file_name constructor argument should contain \
           "%1%" if you don't want to write the same file each time.           \
           */                                                                  \
    Name##OptimizerState args                                                  \
        : OptimizerState(std::string("Writer to ") + file_name),               \
          file_name_(file_name) {                                              \
      constr                                                                   \
    }                                                                          \
    functs void write(std::string file_name, unsigned int call = 0,            \
                      bool append = false) const {                             \
      IMP_UNUSED(call);                                                        \
      IMP_UNUSED(append);                                                      \
      save_action                                                              \
    }                                                                          \
                                                                               \
   private:                                                                    \
    IMP_OBJECT_METHODS(Name##OptimizerState);                                  \
  };                                                                           \
  IMP_OBJECTS(Name##OptimizerState, Name##OptimizerStates);

#endif /* IMPKERNEL_OPTIMIZER_STATE_MACROS_H */
