/**
 *  \file ProteinLocalizationRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <cmath>
#include <algorithm>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/SingletonModifier.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/internal/StaticListContainer.h>
#include <IMP/npc/ProteinLocalizationRestraint.h>

IMPNPC_BEGIN_NAMESPACE

namespace {
IMP::internal::StaticListContainer<SingletonContainer> *get_list(
    SingletonContainer *sc) {
  IMP::internal::StaticListContainer<SingletonContainer> *ret =
      dynamic_cast<
          IMP::internal::StaticListContainer<SingletonContainer> *>(
          sc);
  if (!ret) {
    IMP_THROW("Can only use the set and add methods when no container"
                  << " was passed on construction of ConnectivityRestraint.",
              ValueException);
  }
  return ret;
}
}

/*#####################################################
# Restraints setup - Immuno-EM ZAxialPositionRestraint
# Supplementary Table 7. Upper and lower bounds on Z-axial restraints of C-terminal bead of nups
# NupType : (min Z value, max Z value) (in Angstrom)
#####################################################*/
ZAxialPositionRestraint::ZAxialPositionRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double lower_bound, double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "ZAxialPositionRestraint %1%")
  , lower_bound_(lower_bound)
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

ZAxialPositionRestraint::ZAxialPositionRestraint(Model *m,
    double lower_bound, double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "ZAxialPositionRestraint %1%")
  , lower_bound_(lower_bound)
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void ZAxialPositionRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Z axial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ZAxialPositionRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Z axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ZAxialPositionRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Z axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
ZAxialPositionRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0.0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i=0; i < all_particles.size(); ++i )
  {
    double r = consider_radius_ ? core::XYZR(all_particles[i]).get_radius() : 0.0;
    double z = core::XYZR(all_particles[i]).get_coordinate(2);
    double z_down = consider_radius_ ? z - r : z;
    double z_up   = consider_radius_ ? z + r : z;
    double z_diff = z_down - lower_bound_;
    // check lower_bound_
    if ( z_diff < 0 )
    {
      v += z_diff*z_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        dz[0] = dz[1] = 0.0;
        dz[2] = 2.0*z_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
    // check upper_bound_
    z_diff = z_up - upper_bound_;
    if ( z_diff > 0 )
    {
      v += z_diff*z_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        dz[0] = dz[1] = 0.0;
        dz[2] = 2.0*z_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp ZAxialPositionRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM ZAxialPositionLowerRestraint
# Supplementary Table 7. Upper and lower bounds on Z-axial restraints of C-terminal bead of nups
# NupType : (min Z value, max Z value) (in Angstrom)
#####################################################*/
ZAxialPositionLowerRestraint::ZAxialPositionLowerRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double lower_bound, bool consider_radius, double sigma)
  : Restraint(m, "ZAxialPositionLowerRestraint %1%")
  , lower_bound_(lower_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

ZAxialPositionLowerRestraint::ZAxialPositionLowerRestraint(Model *m,
    double lower_bound, bool consider_radius, double sigma)
  : Restraint(m, "ZAxialPositionLowerRestraint %1%")
  , lower_bound_(lower_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void ZAxialPositionLowerRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Z axial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ZAxialPositionLowerRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Z axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ZAxialPositionLowerRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Z axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
ZAxialPositionLowerRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    double r = core::XYZR(all_particles[i]).get_radius();
    double z = core::XYZR(all_particles[i]).get_coordinate(2);
    double z_down = consider_radius_ ? z - r : z;
    double z_diff = z_down - lower_bound_;
    if ( z_diff < 0 )
    {
      v += z_diff*z_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        dz[0] = dz[1] = 0;
        dz[2] = 2*z_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp ZAxialPositionLowerRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM ZAxialPositionUpperRestraint
# Supplementary Table 7. Upper and lower bounds on Z-axial restraints of C-terminal bead of nups
# NupType : (min Z value, max Z value) (in Angstrom)
#####################################################*/
ZAxialPositionUpperRestraint::ZAxialPositionUpperRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "ZAxialPositionUpperRestraint %1%")
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

ZAxialPositionUpperRestraint::ZAxialPositionUpperRestraint(Model *m,
    double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "ZAxialPositionUpperRestraint %1%")
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void ZAxialPositionUpperRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Z axial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ZAxialPositionUpperRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Z axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ZAxialPositionUpperRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Z axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
ZAxialPositionUpperRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    double r = consider_radius_ ? core::XYZR(all_particles[i]).get_radius() : 0;
    double z = core::XYZR(all_particles[i]).get_coordinate(2);
    double z_up = z + r;
    double z_diff = z_up - upper_bound_;
    if ( z_diff > 0 )
    {
      v += z_diff*z_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        dz[0] = dz[1] = 0;
        dz[2] = 2*z_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp ZAxialPositionUpperRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM YAxialPositionRestraint
# Supplementary Table 7. Upper and lower bounds on Y-axial restraints of C-terminal bead of nups
# NupType : (min Y value, max Y value) (in Angstrom)
#####################################################*/
YAxialPositionRestraint::YAxialPositionRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double lower_bound, double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "YAxialPositionRestraint %1%")
  , lower_bound_(lower_bound)
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

YAxialPositionRestraint::YAxialPositionRestraint(Model *m,
    double lower_bound, double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "YAxialPositionRestraint %1%")
  , lower_bound_(lower_bound)
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void YAxialPositionRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Y axial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void YAxialPositionRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Y axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void YAxialPositionRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Y axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
YAxialPositionRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0.0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i=0; i < all_particles.size(); ++i )
  {
    double r = consider_radius_ ? core::XYZR(all_particles[i]).get_radius() : 0.0;
    double y = core::XYZR(all_particles[i]).get_coordinate(1);
    double y_down = consider_radius_ ? y - r : y;
    double y_up   = consider_radius_ ? y + r : y;
    double y_diff = y_down - lower_bound_;
    // check lower_bound_
    if ( y_diff < 0 )
    {
      v += y_diff*y_diff;
      if ( accum )
      {
        algebra::Vector3D dy;
        dy[0] = dy[1] = 0.0;
        dy[2] = 2.0*y_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dy, *accum);
      }
    }
    // check upper_bound_
    y_diff = y_up - upper_bound_;
    if ( y_diff > 0 )
    {
      v += y_diff*y_diff;
      if ( accum )
      {
        algebra::Vector3D dy;
        dy[0] = dy[1] = 0.0;
        dy[2] = 2.0*y_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dy, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp YAxialPositionRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM YAxialPositionLowerRestraint
# Supplementary Table 7. Upper and lower bounds on Y-axial restraints of C-terminal bead of nups
# NupType : (min Y value, max Y value) (in Angstrom)
#####################################################*/
YAxialPositionLowerRestraint::YAxialPositionLowerRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double lower_bound, bool consider_radius, double sigma)
      : Restraint(m, "YAxialPositionLowerRestraint %1%")
  , lower_bound_(lower_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

YAxialPositionLowerRestraint::YAxialPositionLowerRestraint(Model *m,
    double lower_bound, bool consider_radius, double sigma)
  : Restraint(m, "YAxialPositionLowerRestraint %1%")
  , lower_bound_(lower_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void YAxialPositionLowerRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Y axial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void YAxialPositionLowerRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Y axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void YAxialPositionLowerRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Y axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
YAxialPositionLowerRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    double r = core::XYZR(all_particles[i]).get_radius();
    double y = core::XYZR(all_particles[i]).get_coordinate(1);
    double y_down = consider_radius_ ? y - r : y;
    double y_diff = y_down - lower_bound_;
    if ( y_diff < 0 )
    {
      v += y_diff*y_diff;
      if ( accum )
      {
        algebra::Vector3D dy;
        dy[0] = dy[1] = 0;
        dy[2] = 2*y_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dy, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp YAxialPositionLowerRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM YAxialPositionUpperRestraint
# Supplementary Table 7. Upper and lower bounds on Y-axial restraints of C-terminal bead of nups
# NupType : (min Y value, max Y value) (in Angstrom)
#####################################################*/
YAxialPositionUpperRestraint::YAxialPositionUpperRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "YAxialPositionUpperRestraint %1%")
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

YAxialPositionUpperRestraint::YAxialPositionUpperRestraint(Model *m,
    double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "YAxialPositionUpperRestraint %1%")
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void YAxialPositionUpperRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Y axial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void YAxialPositionUpperRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Y axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void YAxialPositionUpperRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Y axial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
YAxialPositionUpperRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    double r = consider_radius_ ? core::XYZR(all_particles[i]).get_radius() : 0;
    double y = core::XYZR(all_particles[i]).get_coordinate(1);
    double y_up = y + r;
    double y_diff = y_up - upper_bound_;
    if ( y_diff > 0 )
    {
      v += y_diff*y_diff;
      if ( accum )
      {
        algebra::Vector3D dy;
        dy[0] = dy[1] = 0;
        dy[2] = 2*y_diff/sigma_;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dy, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp YAxialPositionUpperRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM XYRadialPositionRestraint
# Supplementary Table 7. Upper and lower bounds on R-radial restraints of C-terminal bead of nups
# NupType : (min R value, max R value) (in Angstrom)
#####################################################*/
XYRadialPositionRestraint::XYRadialPositionRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double lower_bound, double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "XYRadialPositionRestraint %1%")
  , lower_bound_(lower_bound)
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

XYRadialPositionRestraint::XYRadialPositionRestraint(Model *m,
    double lower_bound, double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "XYRadialPositionRestraint %1%")
  , lower_bound_(lower_bound)
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void XYRadialPositionRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "XY Radial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void XYRadialPositionRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "XY Radial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void XYRadialPositionRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "XY Radial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
XYRadialPositionRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  const double eps = 1e-9;
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i=0; i < all_particles.size(); ++i )
  {
    core::XYZR xyzr = core::XYZR(all_particles[i]);
    double r = consider_radius_ ? xyzr.get_radius() : 0;
    double x = xyzr.get_coordinate(0);
    double y = xyzr.get_coordinate(1);
    double radial = std::sqrt(x*x + y*y);
    double r_down = consider_radius_ ? radial - r : radial;
    double r_up   = consider_radius_ ? radial + r : radial;
    double r_diff = r_down - lower_bound_;
    // check lower_bound_
    if ( r_diff < 0 )
    {
      v += r_diff*r_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        if ( radial > eps )
        {
          double coeff = 2.0*r_diff/(radial*sigma_);
          dz[0] = coeff * x;
          dz[1] = coeff * y;
        }
        else
        {
          dz[0] = dz[1] = 0.0;
        }
        dz[2] = 0.0;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
    // check upper_bound_
    r_diff = r_up - upper_bound_;
    if ( r_diff > 0 )
    {
      v += r_diff*r_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        if ( radial > eps )
        {
          double coeff = 2.0*r_diff/(radial*sigma_);
          dz[0] = coeff * x;
          dz[1] = coeff * y;
        }
        else
        {
          dz[0] = dz[1] = 0.0;
        }
        dz[2] = 0.0;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp XYRadialPositionRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM XYRadialPositionLowerRestraint
# Supplementary Table 7. Upper and lower bounds on R-radial restraints of C-terminal bead of nups
# NupType : (min R value, max R value) (in Angstrom)
#####################################################*/
XYRadialPositionLowerRestraint::XYRadialPositionLowerRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double lower_bound, bool consider_radius, double sigma)
  : Restraint(m, "XYRadialPositionLowerRestraint %1%")
  , lower_bound_(lower_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

XYRadialPositionLowerRestraint::XYRadialPositionLowerRestraint(Model *m,
    double lower_bound, bool consider_radius, double sigma)
  : Restraint(m, "XYRadialPositionLowerRestraint %1%")
  , lower_bound_(lower_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void XYRadialPositionLowerRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "XY radial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void XYRadialPositionLowerRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "XY radial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void XYRadialPositionLowerRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "XY radial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
XYRadialPositionLowerRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  const double eps = 1e-9;
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    core::XYZR xyzr = core::XYZR(all_particles[i]);
    double r = consider_radius_ ? xyzr.get_radius() : 0;
    double x = xyzr.get_coordinate(0);
    double y = xyzr.get_coordinate(1);
    double radial = std::sqrt(x*x + y*y);
    double z_diff = radial - r - lower_bound_;
    if ( z_diff < 0 )
    {
      v += z_diff*z_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        if ( radial > eps )
        {
          dz[0] = 2*z_diff*x/(radial*sigma_);
          dz[1] = 2*z_diff*y/(radial*sigma_);
        }
        else
        {
          dz[0] = dz[1] = 0;
        }
        dz[2] = 0;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp XYRadialPositionLowerRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - Immuno-EM XYRadialPositionUpperRestraint
# Supplementary Table 7. Upper and lower bounds on R-radial restraints of C-terminal bead of nups
# NupType : (min R value, max R value) (in Angstrom)
#####################################################*/
XYRadialPositionUpperRestraint::XYRadialPositionUpperRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "XYRadialPositionUpperRestraint %1%")
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

XYRadialPositionUpperRestraint::XYRadialPositionUpperRestraint(Model *m,
    double upper_bound, bool consider_radius, double sigma)
  : Restraint(m, "XYRadialPositionUpperRestraint %1%")
  , upper_bound_(upper_bound)
  , sigma_(sigma)
  , consider_radius_(consider_radius)
{
}

void XYRadialPositionUpperRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "XY Radial list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void XYRadialPositionUpperRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "XY Radial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void XYRadialPositionUpperRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "XY Radial list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
XYRadialPositionUpperRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  const double eps = 1e-9;
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    core::XYZR xyzr = core::XYZR(all_particles[i]);
    double r = consider_radius_ ? xyzr.get_radius() : 0;
    double x = xyzr.get_coordinate(0);
    double y = xyzr.get_coordinate(1);
    double radial = std::sqrt(x*x + y*y);
    double z_diff = radial + r - upper_bound_;
    if ( z_diff > 0 )
    {
      v += z_diff*z_diff;
      if ( accum )
      {
        algebra::Vector3D dz;
        if ( radial > eps )
        {
          dz[0] = 2*z_diff*x/(radial*sigma_);
          dz[1] = 2*z_diff*y/(radial*sigma_);
        }
        else
        {
          dz[0] = dz[1] = 0;
        }
        dz[2] = 0;
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]), dz, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp XYRadialPositionUpperRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - ProteinContactRestraint
#####################################################*/
ProteinContactRestraint::ProteinContactRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double tolerance_factor, double sigma)
  : Restraint(m, "ProteinContactRestraint %1%")
  , tolerance_factor_(tolerance_factor)
  , sigma_(sigma)
{
  sc_ = sc;
}

ProteinContactRestraint::ProteinContactRestraint(Model *m,
    double tolerance_factor, double sigma)
  : Restraint(m, "ProteinContactRestraint %1%")
  , tolerance_factor_(tolerance_factor)
  , sigma_(sigma)
{
}

void ProteinContactRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein contact list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ProteinContactRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein contact list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ProteinContactRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Protein contact list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
ProteinContactRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  std::vector<IMP::algebra::Vector3D> der(all_particles.size());
  for ( unsigned int i = 0; i < der.size(); ++i )
  {
    IMP::algebra::Vector3D &v = der[i];
    v[0] = v[1] = v[2] = 0;
  }
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    core::XYZR i_xyz = core::XYZR(all_particles[i]);
    for ( unsigned int j = i + 1; j < all_particles.size(); ++j )
    {
      core::XYZR j_xyz = core::XYZR(all_particles[i]);
      double lower_bound = tolerance_factor_*(i_xyz.get_radius() + j_xyz.get_radius());
      double dist = std::sqrt((i_xyz.get_coordinates() - j_xyz.get_coordinates()).get_squared_magnitude());
      double diff = dist - lower_bound;
      if ( diff > 0 )
      {
        v += diff*diff;
        double dx = 2*diff*(j_xyz.get_coordinate(0) - i_xyz.get_coordinate(0))/dist;
        double dy = 2*diff*(j_xyz.get_coordinate(1) - i_xyz.get_coordinate(1))/dist;
        double dz = 2*diff*(j_xyz.get_coordinate(2) - i_xyz.get_coordinate(2))/dist;
        der[j] += IMP::algebra::Vector3D(dx/sigma_, dy/sigma_, dz/sigma_);
      }
    }
  }
  if ( accum )
  {
    for ( unsigned int i = 0; i < all_particles.size(); ++i )
    {
      all_particles[i]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[i]), der[i], *accum);
    }
  }
  return v/sigma_;
}

ModelObjectsTemp ProteinContactRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - ProteinChainRestraint
#####################################################*/
ProteinChainRestraint::ProteinChainRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double sigma)
  : Restraint(m, "ProteinChainRestraint %1%")
  , sigma_(sigma)
{
  sc_ = sc;
}

ProteinChainRestraint::ProteinChainRestraint(Model *m,
    double sigma)
  : Restraint(m, "ProteinChainRestraint %1%")
  , sigma_(sigma)
{
}

void ProteinChainRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein chain list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ProteinChainRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein chain list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ProteinChainRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Protein chain list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
ProteinChainRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  std::vector<IMP::algebra::Vector3D> der(all_particles.size());
  for ( unsigned int i = 0; i < der.size(); ++i )
  {
    IMP::algebra::Vector3D &v = der[i];
    v[0] = v[1] = v[2] = 0;
  }
  for (unsigned int i = 1; i < all_particles.size(); ++i )
  {
    core::XYZR i_current = core::XYZR(all_particles[i]);
    core::XYZR i_previous = core::XYZR(all_particles[i - 1]);
    double dist = std::sqrt((i_current.get_coordinates() - i_previous.get_coordinates()).get_squared_magnitude());
    double r = i_current.get_radius() + i_previous.get_radius();
    double diff = dist - r;
    v += diff*diff;
    double dx = 2*diff*(i_current.get_coordinate(0) - i_previous.get_coordinate(0))/dist;
    double dy = 2*diff*(i_current.get_coordinate(1) - i_previous.get_coordinate(1))/dist;
    double dz = 2*diff*(i_current.get_coordinate(2) - i_previous.get_coordinate(2))/dist;
    der[i] += IMP::algebra::Vector3D(dx/sigma_, dy/sigma_, dz/sigma_);
  }
  if ( accum )
  {
    for ( unsigned int i = 0; i < all_particles.size(); ++i )
    {
      all_particles[i]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[i]), der[i], *accum);
    }
  }
  return v/sigma_;
}

ModelObjectsTemp ProteinChainRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Function setup - half_torus_distance
#####################################################*/
namespace
{
std::pair<double, algebra::Vector3D> half_torus_distance(double x, double y, double z, double R, double r)
{
  double d_xy = std::sqrt(x*x + y*y);
  //double d_xy2 = x*x + y*y;
  //double d_xy = std::sqrt(d_xy2);

  if ( d_xy < R )
  {
    const double eps = 1e-9;
    double d_tx, d_ty;
    //double d_txx, d_txy, d_tyy, d_tyx;
    if ( d_xy > eps )
    {
      d_tx = x - x/d_xy*R;    // x - R * cos(theta)
      //d_txx = 1 - R*(x*x/d_xy - d_xy)/d_xy2;
      //d_txy = -R*x*y/(d_xy2*d_xy);
      d_ty = y - y/d_xy*R;    // y - R * sin(theta)
      //d_tyy = 1 - R*(y*y/d_xy - d_xy)/d_xy2;
      //d_tyx = -R*y*x/(d_xy2*d_xy);
    }
    else
    {
      d_tx = x - R;
      //d_txx = 1;
      //d_txy = 0;
      d_ty = y;
      //d_tyy = 1;
      //d_tyx = 0;
    }

    double denom = std::sqrt(z*z + d_tx*d_tx + d_ty*d_ty);
    algebra::Vector3D der = algebra::Vector3D(0, 0, 0);
    /*
    if ( denom > eps )
    {
      der[0] = (d_tx*d_txx + d_ty*d_tyx)/denom;
      der[1] = (d_tx*d_txy + d_ty*d_tyy)/denom;
      der[2] = z/denom;
    }
    else
    {
      der[0] = (d_tx*d_txx + d_ty*d_tyx)/eps;
      der[1] = (d_tx*d_txy + d_ty*d_tyy)/eps;
      der[2] = z/eps;
    }
    */
    return std::make_pair(denom - r, der);
  }
  else
  {
    double dz = std::abs(z) - r;
    //double dz = z > 0 ? z - r : z + r;
    //double denom = std::sqrt(dz*dz + d_tx*d_tx + d_ty*d_ty);
    algebra::Vector3D der = algebra::Vector3D(0, 0, 0);
    /*
    if ( denom > eps )
    {
      der[0] = -(d_tx*d_txx + d_ty*d_tyx)/denom;
      der[1] = -(d_tx*d_txy + d_ty*d_tyy)/denom;
      der[2] = dz/denom;
    }
    else
    {
      der[0] = -(d_tx*d_txx + d_ty*d_tyx)/eps;
      der[1] = -(d_tx*d_txy + d_ty*d_tyy)/eps;
      der[2] = dz/eps;
    }
    */
    return std::make_pair(dz, der);
    //return std::make_pair(-denom, der);
  }
}
}


/*#####################################################
# Restraints setup - MembraneSurfaceLocationRestraint
#####################################################*/
MembraneSurfaceLocationRestraint::MembraneSurfaceLocationRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double R, double r, double thickness, double sigma)
  : Restraint(m, "MembraneSurfaceLocationRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
{
  sc_ = sc;
}

MembraneSurfaceLocationRestraint::MembraneSurfaceLocationRestraint(Model *m,
    double R, double r, double thickness, double sigma)
  : Restraint(m, "MembraneSurfaceLocationRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
{
}

void MembraneSurfaceLocationRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Surface location list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
MembraneSurfaceLocationRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    core::XYZ i_current = core::XYZ(all_particles[i]);
    /*
    double z = i_current.get_coordinate(2);
    if ( z > r_ )
      z = r_;
    else if ( z < -r_ )
      z = -r_;
    std::pair<double, algebra::Vector3D> dist = half_torus_distance(i_current.get_coordinate(0),
        i_current.get_coordinate(1), z, R_, r_);
    */
    std::pair<double, algebra::Vector3D> dist = half_torus_distance(i_current.get_coordinate(0),
        i_current.get_coordinate(1), i_current.get_coordinate(2), R_, r_);
    if ( std::fabs(dist.first) > thickness_ )
    {
      v += dist.first*dist.first;
      /*
      if ( accum )
      {
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]),
            dist.second*2*dist.first/sigma_, *accum);
      }
      */
    }
  }
  return v/sigma_;
}

ModelObjectsTemp MembraneSurfaceLocationRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - MembraneSurfaceLocationConditionalRestraint
#####################################################*/
MembraneSurfaceLocationConditionalRestraint::MembraneSurfaceLocationConditionalRestraint(Model *m,
    SingletonContainerAdaptor sc1,
    SingletonContainerAdaptor sc2,
    double R, double r, double thickness, double sigma)
  : Restraint(m, "MembraneSurfaceLocationConditionalRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
{
  sc1_ = sc1;
  sc2_ = sc2;
}

MembraneSurfaceLocationConditionalRestraint::MembraneSurfaceLocationConditionalRestraint(Model *m,
    double R, double r, double thickness, double sigma)
  : Restraint(m, "MembraneSurfaceLocationConditionalRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
{
}

void MembraneSurfaceLocationConditionalRestraint::set_particles1(const ParticlesTemp &ps) {
  if (!sc1_ && !ps.empty()) {
    sc1_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc1_)->set(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationConditionalRestraint::set_particles2(const ParticlesTemp &ps) {
  if (!sc2_ && !ps.empty()) {
    sc2_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc2_)->set(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationConditionalRestraint::add_particles1(const ParticlesTemp &ps) {
  if (!sc1_ && !ps.empty()) {
    sc1_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc1_)->add(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationConditionalRestraint::add_particles2(const ParticlesTemp &ps) {
  if (!sc2_ && !ps.empty()) {
    sc2_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc2_)->add(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationConditionalRestraint::add_particle1(Particle *ps) {
  if (!sc1_) {
    sc1_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Surface location list");
  }
  get_list(sc1_)->add(IMP::internal::get_index(ps));
}

void MembraneSurfaceLocationConditionalRestraint::add_particle2(Particle *ps) {
  if (!sc2_) {
    sc2_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Surface location list");
  }
  get_list(sc2_)->add(IMP::internal::get_index(ps));
}

double
MembraneSurfaceLocationConditionalRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{
  IMP_CHECK_OBJECT(sc1_.get());
  IMP_CHECK_OBJECT(sc2_.get());
  double v1 = 0.0;
  IMP::ParticlesTemp all_particles1 = sc1_->get();
  for (unsigned int i = 0; i < all_particles1.size(); ++i )
  {
    core::XYZ i_current = core::XYZ(all_particles1[i]);
    std::pair<double, algebra::Vector3D> dist = half_torus_distance(i_current.get_coordinate(0),
        i_current.get_coordinate(1), i_current.get_coordinate(2), R_, r_);
    if ( std::fabs(dist.first) > thickness_ )
    {
      v1 += dist.first*dist.first;
    }
  }
  double v2 = 0.0;
  IMP::ParticlesTemp all_particles2 = sc2_->get();
  for (unsigned int i = 0; i < all_particles2.size(); ++i )
  {
    core::XYZ i_current = core::XYZ(all_particles2[i]);
    std::pair<double, algebra::Vector3D> dist = half_torus_distance(i_current.get_coordinate(0),
        i_current.get_coordinate(1), i_current.get_coordinate(2), R_, r_);
    if ( std::fabs(dist.first) > thickness_ )
    {
      v2 += dist.first*dist.first;
    }
  }
  //printf("v1 = %g\t v2 = %g\t min=%g\n", v1, v2, std::min(v1, v2));
  return std::min(v1, v2)/sigma_;
}

ModelObjectsTemp MembraneSurfaceLocationConditionalRestraint::do_get_inputs() const {
  if ( !sc1_ )
    return ModelObjectsTemp();
  if ( !sc2_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc1_->get_all_possible_indexes();
  all += sc2_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - MembraneExclusionRestraint
#####################################################*/
MembraneExclusionRestraint::MembraneExclusionRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double R, double r, double thickness, double sigma)
  : Restraint(m, "MembraneExclusionRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
{
  sc_ = sc;
}

MembraneExclusionRestraint::MembraneExclusionRestraint(Model *m,
    double R, double r, double thickness, double sigma)
  : Restraint(m, "MembraneExclusionRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
{
}

void MembraneExclusionRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Membrane exclusion list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void MembraneExclusionRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Membrane exclusion list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void MembraneExclusionRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Membrane exclusion list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
MembraneExclusionRestraint::unprotected_evaluate(DerivativeAccumulator *) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0.0, x, y, z;
  core::XYZ i_current;
  IMP::ParticlesTemp all_particles = sc_->get();
  double excluded_radius = r_ + thickness_;
  double membrane_boundary = (R_ - excluded_radius) * (R_ - excluded_radius);

  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    i_current = core::XYZ(all_particles[i]);
    x = i_current.get_coordinate(0);
    y = i_current.get_coordinate(1);
    z = i_current.get_coordinate(2);

    if ( std::fabs(z) > excluded_radius )
      continue;

    if ( (x*x + y*y) < membrane_boundary )
      continue;

    std::pair<double, algebra::Vector3D> dist = half_torus_distance(x, y, z, R_, r_);
    if ( dist.first < thickness_ )
    {
      v += (thickness_ - dist.first) * (thickness_ - dist.first);
      /*
      if ( accum )
      {
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]),
            dist.second*2*dist.first/sigma_, *accum);
      }
      */
    }
  }
  return v/sigma_;
}

ModelObjectsTemp MembraneExclusionRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - PoreSideVolumeLocationRestraint
#####################################################*/
PoreSideVolumeLocationRestraint::PoreSideVolumeLocationRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double R, double r, double thickness,
    bool consider_radius, double sigma)
  : Restraint(m, "PoreSideVolumeLocationRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

PoreSideVolumeLocationRestraint::PoreSideVolumeLocationRestraint(Model *m,
    double R, double r, double thickness,
    bool consider_radius, double sigma)
  : Restraint(m, "PoreSideVolumeLocationRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
  , consider_radius_(consider_radius)
{
}

void PoreSideVolumeLocationRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void PoreSideVolumeLocationRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void PoreSideVolumeLocationRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Surface location list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
PoreSideVolumeLocationRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    core::XYZR i_current = core::XYZR(all_particles[i]);
    double z = i_current.get_coordinate(2);
    if ( z > r_ )
      z = r_;
    else if ( z < -r_ )
      z = -r_;
    std::pair<double, algebra::Vector3D> dist = half_torus_distance(i_current.get_coordinate(0),
        i_current.get_coordinate(1), z, R_, r_);
    double radius = consider_radius_ ? i_current.get_radius() : 0;
    dist.first -= radius + thickness_;
    if ( dist.first < 0 )
    {
      v += dist.first*dist.first;
      if ( accum )
      {
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]),
            dist.second*2*dist.first/sigma_, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp PoreSideVolumeLocationRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - PerinuclearVolumeLocationRestraint
#####################################################*/
PerinuclearVolumeLocationRestraint::PerinuclearVolumeLocationRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double R, double r, double thickness,
    bool consider_radius, double sigma)
  : Restraint(m, "PerinuclearVolumeLocationRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
  , consider_radius_(consider_radius)
{
  sc_ = sc;
}

PerinuclearVolumeLocationRestraint::PerinuclearVolumeLocationRestraint(Model *m,
    double R, double r, double thickness,
    bool consider_radius, double sigma)
  : Restraint(m, "PerinuclearVolumeLocationRestraint %1%")
  , R_(R)
  , r_(r)
  , sigma_(sigma)
  , thickness_(thickness/2)
  , consider_radius_(consider_radius)
{
}

void PerinuclearVolumeLocationRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void PerinuclearVolumeLocationRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Surface location list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void PerinuclearVolumeLocationRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Surface location list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
PerinuclearVolumeLocationRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  double v = 0;
  IMP::ParticlesTemp all_particles = sc_->get();
  for (unsigned int i = 0; i < all_particles.size(); ++i )
  {
    core::XYZR i_current = core::XYZR(all_particles[i]);
    double z = i_current.get_coordinate(2);
    if ( z > r_ )
      z = r_;
    else if ( z < -r_ )
      z = -r_;
    std::pair<double, algebra::Vector3D> dist = half_torus_distance(i_current.get_coordinate(0),
        i_current.get_coordinate(1), z, R_, r_);
    double radius = consider_radius_ ? i_current.get_radius() : 0;
    dist.first += radius + thickness_;
    if ( dist.first > 0 )
    {
      v += dist.first*dist.first;
      if ( accum )
      {
        all_particles[i]->get_model()->add_to_coordinate_derivatives(IMP::internal::get_index(all_particles[i]),
            dist.second*2*dist.first/sigma_, *accum);
      }
    }
  }
  return v/sigma_;
}

ModelObjectsTemp PerinuclearVolumeLocationRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - AssemblySymmetryByDistanceRestraint
#####################################################*/
AssemblySymmetryByDistanceRestraint::AssemblySymmetryByDistanceRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double sigma)
  : Restraint(m, "AssemblySymmetryByDistanceRestraint %1%")
  , sigma_(sigma)
{
  sc_ = sc;
}

AssemblySymmetryByDistanceRestraint::AssemblySymmetryByDistanceRestraint(Model *m,
    double sigma)
  : Restraint(m, "AssemblySymmetryByDistanceRestraint %1%")
  , sigma_(sigma)
{
}

void AssemblySymmetryByDistanceRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void AssemblySymmetryByDistanceRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void AssemblySymmetryByDistanceRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
AssemblySymmetryByDistanceRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  IMP::ParticlesTemp all_particles = sc_->get();
  IMP_USAGE_CHECK(all_particles.size() == 4, "AssemblySymmetryByDistanceRestraint must contain exactly 4 particles");
  double d1 = (core::XYZ(all_particles[0]).get_coordinates()
               - core::XYZ(all_particles[1]).get_coordinates()).get_magnitude();
  double d2 = (core::XYZ(all_particles[2]).get_coordinates()
               - core::XYZ(all_particles[3]).get_coordinates()).get_magnitude();
  double diff = d1 - d2;
  if ( accum )
  {
    {
      algebra::Vector3D v0 = core::XYZ(all_particles[0]).get_coordinates();
      algebra::Vector3D v1 = core::XYZ(all_particles[1]).get_coordinates();
      algebra::Vector3D der = diff*(v0 - v1)/(d1*sigma_);
      all_particles[0]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[0]), der, *accum);
      all_particles[1]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[1]), -der, *accum);
    }
    {
      algebra::Vector3D v0 = core::XYZ(all_particles[2]).get_coordinates();
      algebra::Vector3D v1 = core::XYZ(all_particles[3]).get_coordinates();
      algebra::Vector3D der = diff*(v0 - v1)/(d2*sigma_);
      all_particles[2]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[2]), -der, *accum);
      all_particles[3]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[3]), der, *accum);
    }
  }
  return diff*diff/sigma_;
}

ModelObjectsTemp AssemblySymmetryByDistanceRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - AssemblySymmetryByDihedralRestraint
#####################################################*/
AssemblySymmetryByDihedralRestraint::AssemblySymmetryByDihedralRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double sigma)
  : Restraint(m, "AssemblySymmetryByDihedralRestraint %1%")
  , sigma_(sigma)
{
  sc_ = sc;
}

AssemblySymmetryByDihedralRestraint::AssemblySymmetryByDihedralRestraint(Model *m,
    double sigma)
  : Restraint(m, "AssemblySymmetryByDihedralRestraint %1%")
  , sigma_(sigma)
{
}

void AssemblySymmetryByDihedralRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void AssemblySymmetryByDihedralRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void AssemblySymmetryByDihedralRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
AssemblySymmetryByDihedralRestraint::unprotected_evaluate(DerivativeAccumulator * /*accum*/) const
{
  IMP_CHECK_OBJECT(sc_.get());
  IMP::ParticlesTemp all_particles = sc_->get();
  IMP_USAGE_CHECK(all_particles.size() == 8, "AssemblySymmetryByDihedralRestraint must contain exactly 8 particles");
  double a1 = IMP::core::get_dihedral(core::XYZ(all_particles[0]),
      core::XYZ(all_particles[1]), core::XYZ(all_particles[2]),
      core::XYZ(all_particles[3]));
  double a2 = IMP::core::get_dihedral(core::XYZ(all_particles[4]),
      core::XYZ(all_particles[5]), core::XYZ(all_particles[6]),
      core::XYZ(all_particles[7]));
  double diff = a1 - a2;
  return diff*diff/sigma_;
}

ModelObjectsTemp AssemblySymmetryByDihedralRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}


/*#####################################################
# Restraints setup - ProteinProximityRestraint
#####################################################*/
ProteinProximityRestraint::ProteinProximityRestraint(Model *m,
    SingletonContainerAdaptor sc,
    double max_dist, double sigma)
  : Restraint(m, "ProteinProximityRestraint %1%")
  , sigma_(sigma)
  , max_dist_(max_dist)
{
  sc_ = sc;
}

ProteinProximityRestraint::ProteinProximityRestraint(Model *m,
    double max_dist, double sigma)
  : Restraint(m, "ProteinProximityRestraint %1%")
  , sigma_(sigma)
  , max_dist_(max_dist)
{
}

void ProteinProximityRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ProteinProximityRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps[0]->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ProteinProximityRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_ = new IMP::internal::StaticListContainer<SingletonContainer>(
        ps->get_model(), "Protein group configuration list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

double
ProteinProximityRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(sc_.get());
  IMP::ParticlesTemp all_particles = sc_->get();
  std::vector<IMP::algebra::Vector3D> der(all_particles.size());
  for ( unsigned int i = 0; i < der.size(); ++i )
  {
    IMP::algebra::Vector3D &v = der[i];
    v[0] = v[1] = v[2] = 0;
  }
  double v = 0;
  for ( size_t i = 0; i + 1 < all_particles.size(); ++i )
    for ( size_t j = i + 1; j < all_particles.size(); ++j )
    {
      double m = (core::XYZ(all_particles[i]).get_coordinates()
               - core::XYZ(all_particles[j]).get_coordinates()).get_magnitude();
      double d = m - max_dist_;
      if ( d > 0 )
      {
        v += d*d;
        double dx = 2*d*(core::XYZ(all_particles[j]).get_coordinate(0) - core::XYZ(all_particles[i]).get_coordinate(0))/m;
        double dy = 2*d*(core::XYZ(all_particles[j]).get_coordinate(1) - core::XYZ(all_particles[i]).get_coordinate(1))/m;
        double dz = 2*d*(core::XYZ(all_particles[j]).get_coordinate(2) - core::XYZ(all_particles[i]).get_coordinate(2))/m;
        der[j] += IMP::algebra::Vector3D(dx/sigma_, dy/sigma_, dz/sigma_);
      }
    }
  if ( accum )
  {
    for ( unsigned int i = 0; i < all_particles.size(); ++i )
    {
      all_particles[i]->get_model()->add_to_coordinate_derivatives(
          IMP::internal::get_index(all_particles[i]), der[i], *accum);
    }
  }
  return v/sigma_;
}

ModelObjectsTemp ProteinProximityRestraint::do_get_inputs() const {
  if ( !sc_ )
    return ModelObjectsTemp();
  ParticleIndexes all = sc_->get_all_possible_indexes();
  return IMP::get_particles(get_model(), all);
}

IMPNPC_END_NAMESPACE
