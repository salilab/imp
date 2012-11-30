/**
 * \file RotamerCalculator.cpp
 * \brief Object calculating coordinates based on rotamer library.
 *
 * Copyright 2007-2012 Sali Lab. All rights reserved.
 *
 */

#include <string>
//#include <iostream>
#include <boost/algorithm/string.hpp>
#include <IMP/constants.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/dihedrals.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/rotamer/RotamerCalculator.h>

IMPROTAMER_BEGIN_NAMESPACE

namespace
{

struct AngleConfiguration
{
  const char res[4];
  const char *chi_axis[4];
  const char *chi1_topology;
  int chi_topology[3];
};

enum { NRes = 18 };

AngleConfiguration angle_configurations[NRes] = {
  { "ARG",
    { "N-CA-CB-CG", "CD", "NE", "CZ" },
    "HB2,HB3,CG,HG2,HG3,CD,HD2,HD3,NE,HE,CZ,NH1,HH11,HH12,NH2,HH21,HH22",
    { 3, 6, 9}
  },
  {
    "ASN",
    { "N-CA-CB-CG", "OD1", 0, 0 },
    "HB2,HB3,CG,OD1,ND2,HD21,HD22",
    { 3, -1, -1 }
  },
  {
    "ASP",
    { "N-CA-CB-CG", "OD1", 0, 0 },
    "HB2,HB3,CG,OD1,OD2",
    { 3, -1, -1 }
  },
  {
    "CYS",
    { "N-CA-CB-SG", 0, 0, 0 },
    "HB2,HB3,SG,HG",
    { -1, -1, -1 }
  },
  {
    "GLN",
    { "N-CA-CB-CG", "CD", "OE1", 0 },
    "HB2,HB3,CG,HG2,HG3,CD,OE1,NE2,HE21,HE22",
    { 3, 6, -1 }
  },
  {
    "GLU",
    { "N-CA-CB-CG", "CD", "OE1", 0 },
    "HB2,HB3,CG,HG2,HG3,CD,OE1,OE2",
    { 3, 6, -1 }
  },
  {
    "HIS",
    { "N-CA-CB-CG", "ND1", 0, 0 },
    "HB2,HB3,CG,ND1,HD1,HE1,CE1,CD2,NE2,HD2",
    { 3, -1, -1 }
  },
  {
    "ILE",
    { "N-CA-CB-CG1", "CD1", 0, 0 },
    "HB,CG2,HG21,HG23,HG22,CG1,HG12,HG13,CD1,HD11,HD12,HD13",
    { 6, -1, -1 }
  },
  {
    "LEU",
    { "N-CA-CB-CG", "CD1", 0, 0 },
    "HB2,HB3,CG,CD2,HD21,HD22,HD23,HG,CD1,HD13,HD11,HD12",
    { 3, -1, -1 }
  },
  {
    "LYS",
    { "N-CA-CB-CG", "CD", "CE", "NZ" },
    "HB2,HB3,CG,HG3,HG2,CD,HD2,HD3,CE,HE2,HE3,NZ,HZ1,HZ2,HZ3",
    { 3, 6, 9 }
  },
  {
    "MET",
    { "N-CA-CB-CG", "SD", "CE", 0 },
    "HB2,HB3,CG,HG2,HG3,SD,CE,HE1,HE2,HE3",
    { 3, 6, -1 }
  },
  {
    "PHE",
    { "N-CA-CB-CG", "CD1", 0, 0 },
    "HB2,HB3,CG,CD2,HD2,CE2,HE2,CZ,HZ,CD1,HD1,CE1,HE1",
    { 3, -1, -1 }
  },
  {
    "PRO",
    { "N-CA-CB-CG", "CD", 0, 0 },
    "HB2,HB3,CG,HG3,HG2,CD,HD2,HD3",
    { 3, -1, -1 }
  },
  {
    "SER",
    { "N-CA-CB-OG", 0, 0, 0 },
    "HB2,HB3,OG,HG",
    { -1, -1, -1 }
  },
  {
    "THR",
    { "N-CA-CB-OG1", 0, 0, 0 },
    "CG2,HG23,HG22,HG21,HB,OG1,HG1",
    { -1, -1, -1 }
  },
  {
    "TRP",
    { "N-CA-CB-CG", "CD1", 0, 0 },
    "HB2,HB3,CG,CD1,HD1,NE1,HE1,CD2,CE2,CZ2,HZ2,CE3,HE3,CZ3,HZ3,CH2,HH2",
    { 3, -1, -1 }
  },
  {
    "TYR",
    { "N-CA-CB-CG", "CD1", 0, 0 },
    "HB2,HB3,CG,CD2,HD2,CE2,HE2,CZ,OH,HH,CD1,HD1,CE1,HE1",
    { 3, -1, -1 }
  },
  {
    "VAL",
    { "N-CA-CB-CG1", 0, 0, 0 },
    "CG1,HG11,HG12,HG13,HB,CG2,HG21,HG23,HG22",
    { -1, -1, -1 }
  }
};


double get_psi_angle(IMP::atom::Residue rd)
{
  IMP::atom::Atoms psi_atoms = IMP::atom::get_psi_dihedral_atoms(rd);
  // if residue is at the beginning or end of the chain, assume 0
  if ( psi_atoms.empty() )
    return 0;
  return IMP::core::get_dihedral(IMP::core::XYZ(psi_atoms[0]),
      IMP::core::XYZ(psi_atoms[1]), IMP::core::XYZ(psi_atoms[2]),
      IMP::core::XYZ(psi_atoms[3]));
}


double get_phi_angle(IMP::atom::Residue rd)
{
  IMP::atom::Atoms phi_atoms = IMP::atom::get_phi_dihedral_atoms(rd);
  // if residue is at the beginning or end of the chain, assume 0
  if ( phi_atoms.empty() )
    return 0;
  return IMP::core::get_dihedral(IMP::core::XYZ(phi_atoms[0]),
      IMP::core::XYZ(phi_atoms[1]), IMP::core::XYZ(phi_atoms[2]),
      IMP::core::XYZ(phi_atoms[3]));
}


}


RotamerCalculator::RotamerCalculator(const RotamerLibrary *rl)
  : IMP::base::Object("RotamerCalculator%1%")
  , rl_(rl)
{
  IMP_USAGE_CHECK(rl != 0, "RotamerLibrary must be non-null");
  for ( int i = 0; i < NRes; ++i )
  {
    const AngleConfiguration &ac = angle_configurations[i];
    IMP::atom::ResidueType rd(ac.res);
    if ( rd.get_index() >= residues_.size() )
      residues_.resize(rd.get_index() + 1);
    ResidueData &res_d = residues_[rd.get_index()];
    std::vector<IMP::atom::AtomType> rot_atoms;
    {
      std::vector<std::string> at_names;
      boost::split(at_names, ac.chi1_topology, boost::is_any_of(","));
      rot_atoms.reserve(at_names.size());
      for ( size_t k = 0; k != at_names.size(); ++k )
        rot_atoms.push_back(IMP::atom::AtomType(at_names[k]));
    }
    size_t current_start = 0;
    res_d.n_angles = 0;
    int j = -1;
    do
    {
      int mask = 1<<(j + 1);
      for ( size_t k = current_start; k != rot_atoms.size(); ++k )
      {
        unsigned idx = rot_atoms[k].get_index();
        if ( idx >= res_d.rot_atoms.size() )
          res_d.rot_atoms.resize(idx + 1, 0);
        res_d.rot_atoms[idx] |= mask;
      }
      ++res_d.n_angles;
      if ( j < 2 && ac.chi_topology[j + 1] >= 0 )
      {
        current_start = ac.chi_topology[++j];
      }
      else
        break;
    } while ( true );
    {
      std::vector<std::string> at_axes_names;
      boost::split(at_axes_names, ac.chi_axis[0], boost::is_any_of("-"));
      for ( int j = 1; j < 4; ++j )
        if ( ac.chi_axis[j] )
          at_axes_names.push_back(ac.chi_axis[j]);
      res_d.at_axes.reserve(at_axes_names.size());
      for ( size_t k = 0; k != at_axes_names.size(); ++k )
        res_d.at_axes.push_back(IMP::atom::AtomType(at_axes_names[k]));
    }
  }
}


void ResidueRotamer::add_coordinates(const IMP::atom::AtomType &at,
      const IMP::algebra::Vector3D &coords)
{
  unsigned at_idx = at.get_index();
  if ( at_idx >= residue_coordinates_.size() )
    residue_coordinates_.resize(at_idx + 1);
  residue_coordinates_[at_idx].push_back(coords);
}


const IMP::algebra::Vector3D &ResidueRotamer::get_coordinates(unsigned index,
      const IMP::atom::AtomType &at) const
{
  unsigned at_idx = at.get_index();
  IMP_USAGE_CHECK(at_idx < residue_coordinates_.size() &&
      index < residue_coordinates_[at_idx].size(),
      "no atom of this type present or not enough cases");
  return residue_coordinates_[at_idx][index];
}


bool ResidueRotamer::get_atom_exists(const IMP::atom::AtomType &at) const
{
  unsigned idx = at.get_index();
  return idx < residue_coordinates_.size() &&
    !residue_coordinates_[idx].empty();
}


unsigned ResidueRotamer::get_number_of_cases(
                          const IMP::atom::AtomType &at) const
{
  unsigned idx = at.get_index();
  if ( idx >= residue_coordinates_.size() )
    return 0;
  return residue_coordinates_[idx].size();
}


void ResidueRotamer::push_coordinates()
{
  for ( size_t i = 0; i != residue_coordinates_.size(); ++i )
    if ( !residue_coordinates_[i].empty() )
    {
      IMP::algebra::Vector3D coords = residue_coordinates_[i][0];
      residue_coordinates_[i].push_back(coords);
    }
}


IMP::algebra::Vector3D &ResidueRotamer::get_coordinates(
                                     const IMP::atom::AtomType &at)
{
  unsigned at_idx = at.get_index();
  IMP_USAGE_CHECK(at_idx < residue_coordinates_.size() &&
      !residue_coordinates_[at_idx].empty(), "no atom of this type present");
  return residue_coordinates_[at_idx].back();
}


ResidueRotamer RotamerCalculator::get_rotamer(const IMP::atom::Residue &rd,
                                              double thr) const
{
  IMP::atom::ResidueType rt = rd.get_residue_type();
  ResidueRotamer r;

  // the coordinates at index 0 are the original coordinates
  IMP::atom::Hierarchies mhs = IMP::atom::get_by_type(rd, IMP::atom::ATOM_TYPE);
  for ( size_t i = 0; i != mhs.size(); ++i )
  {
    IMP::atom::Atom at = mhs[i].get_as_atom();
    r.add_coordinates(at.get_atom_type(), IMP::core::XYZ(at).get_coordinates());
  }

  // unknown residue will not be rotated
  unsigned r_idx = rt.get_index();
  if ( r_idx >= residues_.size() || residues_[r_idx].empty() )
  {
    IMP_LOG(VERBOSE, "Residue " << rt << " is unknown" << std::endl);
    return r;
  }

  // query the rotamer library for chi-angles (the library stores angles
  // in degrees)
  const double to_deg = 180/IMP::PI, from_deg = IMP::PI/180;
  double psi = get_psi_angle(rd)*to_deg, phi = get_phi_angle(rd)*to_deg;
  RotamerLibrary::RotamerRange rr = rl_->get_rotamers_fast(rt, phi, psi, thr);

  // we need to create fake particles representing rotated atoms
  // (for get_dihedral)
  IMP_NEW(IMP::Model, model, ());
  IMP_NEW(IMP::Particle, p_a, (model));
  IMP_NEW(IMP::Particle, p_b, (model));
  IMP_NEW(IMP::Particle, p_c, (model));
  IMP_NEW(IMP::Particle, p_d, (model));
  IMP::core::XYZ xyz_a = IMP::core::XYZ::setup_particle(p_a);
  IMP::core::XYZ xyz_b = IMP::core::XYZ::setup_particle(p_b);
  IMP::core::XYZ xyz_c = IMP::core::XYZ::setup_particle(p_c);
  IMP::core::XYZ xyz_d = IMP::core::XYZ::setup_particle(p_d);

  const ResidueData &res_data = residues_[r_idx];

  // iterate through all sets of chi angles
  for ( RotamerLibrary::RotamerIterator p = rr.begin(); p != rr.end(); ++p )
  {
    r.push_coordinates();
    const RotamerAngleTuple &ra = *p;
    double desired_angles[4] = {ra.get_chi1()*from_deg, ra.get_chi2()*from_deg,
      ra.get_chi3()*from_deg, ra.get_chi4()*from_deg};

    // iterate through chi-angles
    for ( int i = 0; i < res_data.n_angles; ++i )
    {
      // find the axis atoms
      // sometimes the required atoms are missing - log and skip the rotation
      // in these cases
      bool complete = true;
      IMP::atom::Atom at_a = IMP::atom::get_atom(rd, res_data.at_axes[i]);
      if ( !at_a )
      {
        IMP_LOG(VERBOSE, "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i] << std::endl);
        complete = false;
      }
      IMP::atom::Atom at_b = IMP::atom::get_atom(rd, res_data.at_axes[i + 1]);
      if ( !at_b )
      {
        IMP_LOG(VERBOSE, "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i + 1] << std::endl);
        complete = false;
      }
      IMP::atom::Atom at_c = IMP::atom::get_atom(rd, res_data.at_axes[i + 2]);
      if ( !at_c )
      {
        IMP_LOG(VERBOSE, "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i + 2] << std::endl);
        complete = false;
      }
      IMP::atom::Atom at_d = IMP::atom::get_atom(rd, res_data.at_axes[i + 3]);
      if ( !at_d )
      {
        IMP_LOG(VERBOSE, "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i + 3] << std::endl);
        complete = false;
      }
      if ( !complete )
        break;

      // get the current coordinates of the axis atoms
      IMP::algebra::Vector3D &c_a = r.get_coordinates(at_a.get_atom_type());
      IMP::algebra::Vector3D &c_b = r.get_coordinates(at_b.get_atom_type());
      IMP::algebra::Vector3D &c_c = r.get_coordinates(at_c.get_atom_type());
      IMP::algebra::Vector3D &c_d = r.get_coordinates(at_d.get_atom_type());
      xyz_a.set_coordinates(c_a);
      xyz_b.set_coordinates(c_b);
      xyz_c.set_coordinates(c_c);
      xyz_d.set_coordinates(c_d);

      // compute the original dihedral angle
      double actual_angle = IMP::core::get_dihedral(xyz_a, xyz_b, xyz_c,
          xyz_d);

      // now perform the rotations (about c_b->c_c axis) and store the
      // coordinates
      int mask = 1<<i;
      IMP::algebra::Vector3D rot_axis = c_c - c_b;
      IMP::algebra::Rotation3D rot = IMP::algebra::get_rotation_about_axis(
          rot_axis, desired_angles[i] - actual_angle);
      IMP::algebra::Transformation3D t = IMP::algebra::get_rotation_about_point(
          c_b, rot);
      for ( size_t k = 0; k != mhs.size(); ++k )
      {
        IMP::atom::Atom at = mhs[k].get_as_atom();
        IMP::atom::AtomType at_type = at.get_atom_type();
        unsigned at_idx = at_type.get_index();
        if ( at_idx < res_data.rot_atoms.size() &&
            (res_data.rot_atoms[at_idx]&mask) )
        {
          // Atom "at" participates in this rotation
          IMP::algebra::Vector3D &current = r.get_coordinates(at_type);
          current = t.get_transformed(current);
        }
      }
    }
  }

  return r;
}


IMPROTAMER_END_NAMESPACE
