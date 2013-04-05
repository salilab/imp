/**
 * \file RotamerCalculator.cpp
 * \brief Object calculating coordinates based on rotamer library.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <string>
#include <cmath>
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
  ++size_;
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
  ResidueRotamer r(rt);

  // the coordinates at index 0 are the original coordinates
  IMP::atom::Hierarchies mhs = IMP::atom::get_by_type(rd, IMP::atom::ATOM_TYPE);
  for ( size_t i = 0; i != mhs.size(); ++i )
  {
    IMP::atom::Atom at = mhs[i].get_as_atom();
    r.add_coordinates(at.get_atom_type(), IMP::core::XYZ(at).get_coordinates());
  }
  r.size_ = 1;
  r.probabilities_.push_back(0);

  // unknown residue will not be rotated
  unsigned r_idx = rt.get_index();
  if ( r_idx >= residues_.size() || residues_[r_idx].empty() )
  {
    IMP_LOG_VERBOSE( "Residue " << rt << " is unknown" << std::endl);
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
    r.probabilities_.push_back(ra.get_probability());
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
        IMP_LOG_VERBOSE( "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i] << std::endl);
        complete = false;
      }
      IMP::atom::Atom at_b = IMP::atom::get_atom(rd, res_data.at_axes[i + 1]);
      if ( !at_b )
      {
        IMP_LOG_VERBOSE( "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i + 1] << std::endl);
        complete = false;
      }
      IMP::atom::Atom at_c = IMP::atom::get_atom(rd, res_data.at_axes[i + 2]);
      if ( !at_c )
      {
        IMP_LOG_VERBOSE( "Residue " << rt << ": no atom of type " <<
                res_data.at_axes[i + 2] << std::endl);
        complete = false;
      }
      IMP::atom::Atom at_d = IMP::atom::get_atom(rd, res_data.at_axes[i + 3]);
      if ( !at_d )
      {
        IMP_LOG_VERBOSE( "Residue " << rt << ": no atom of type " <<
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


double ResidueRotamer::get_probability(unsigned index) const
{
  IMP_USAGE_CHECK(index < probabilities_.size(),
      "no rotamer present at given index");
  return probabilities_[index];
}


namespace
{


bool is_backbone(unsigned at_idx)
{
  return ( at_idx == IMP::atom::AT_N.get_index() ||
      at_idx == IMP::atom::AT_H.get_index() ||
      at_idx == IMP::atom::AT_HA.get_index() ||
      at_idx == IMP::atom::AT_CA.get_index() ||
      at_idx == IMP::atom::AT_C.get_index() ||
      at_idx == IMP::atom::AT_O.get_index() );
}

}


void ResidueRotamer::create_bounding_boxes(ResidueRotamer::Box3D &bb_box,
          ResidueRotamer::Box3D &sc_box, ResidueRotamer::Boxes3D &rot_boxes)
{
  bb_box.clear();
  sc_box.clear();
  rot_boxes.clear();
  for ( size_t at_idx = 0; at_idx != residue_coordinates_.size(); ++at_idx )
    if ( !residue_coordinates_[at_idx].empty() )
    {
      if ( is_backbone(at_idx) )
      {
        // backbone atom - only consider original coordinates as
        // they do not change (eq. 20-25)
        IMP::algebra::Vector3D const &coords = residue_coordinates_[at_idx][0];
        bb_box.xmin = std::min(bb_box.xmin, coords[0] - 3.5);
        bb_box.ymin = std::min(bb_box.ymin, coords[1] - 3.5);
        bb_box.zmin = std::min(bb_box.zmin, coords[2] - 3.5);
        bb_box.xmax = std::max(bb_box.xmax, coords[0] + 3.5);
        bb_box.ymax = std::max(bb_box.ymax, coords[1] + 3.5);
        bb_box.zmax = std::max(bb_box.zmax, coords[2] + 3.5);
      }
      else
      {
        if ( rot_boxes.size() < residue_coordinates_[at_idx].size() )
          rot_boxes.resize(residue_coordinates_[at_idx].size());
        // side chain atom - must consider all rotamers (eq. 26 - 31)
        for ( size_t j = 1; j < residue_coordinates_[at_idx].size(); ++j )
        {
          IMP::algebra::Vector3D const &coords =
                                      residue_coordinates_[at_idx][j];
          sc_box.xmin = std::min(sc_box.xmin, coords[0] - 3.5);
          sc_box.ymin = std::min(sc_box.ymin, coords[1] - 3.5);
          sc_box.zmin = std::min(sc_box.zmin, coords[2] - 3.5);
          sc_box.xmax = std::max(sc_box.xmax, coords[0] + 3.5);
          sc_box.ymax = std::max(sc_box.ymax, coords[1] + 3.5);
          sc_box.zmax = std::max(sc_box.zmax, coords[2] + 3.5);

          // now consider specific rotamer in rotamer boxes (eq. 32 - 37)
          Box3D &rot_box = rot_boxes[j];
          rot_box.xmin = std::min(rot_box.xmin, coords[0] - 3.5);
          rot_box.ymin = std::min(rot_box.ymin, coords[1] - 3.5);
          rot_box.zmin = std::min(rot_box.zmin, coords[2] - 3.5);
          rot_box.xmax = std::max(rot_box.xmax, coords[0] + 3.5);
          rot_box.ymax = std::max(rot_box.ymax, coords[1] + 3.5);
          rot_box.zmax = std::max(rot_box.zmax, coords[2] + 3.5);
        }
      }
    }
}


void ResidueRotamer::set_coordinates(unsigned index,
      IMP::atom::Residue &rd) const
{
  IMP_USAGE_CHECK(index < size_,
      "no rotamer at given index");
  IMP_USAGE_CHECK(rd.get_residue_type() == residue_type_,
      "wrong residue type");
  IMP::atom::Hierarchies mhs = IMP::atom::get_by_type(rd, IMP::atom::ATOM_TYPE);
  for ( size_t i = 0; i != mhs.size(); ++i )
  {
    IMP::atom::Atom at = mhs[i].get_as_atom();
    IMP::atom::AtomType at_t = at.get_atom_type();
    if ( get_atom_exists(at_t) )
    {
      const IMP::algebra::Vector3D &coords = get_coordinates(index, at_t);
      IMP::core::XYZ(at).set_coordinates(coords);
    }
  }
}


bool ResidueRotamer::intersect(const ResidueRotamer::Box3D &b1,
                               const ResidueRotamer::Box3D &b2)
{
  return b1.xmax >= b2.xmin && b1.xmin <= b2.xmax && b1.ymax >= b2.ymin &&
         b1.ymin <= b2.ymax && b1.zmax >= b2.zmin && b1.zmin <= b2.zmax;
}


void RotamerCalculator::transform(const IMP::atom::Hierarchy &protein,
                                  const IMP::PairScore *score, double thr,
                                  double K, int num_iter) const
{
  IMP_USAGE_CHECK(protein, "protein must me non-null");
  IMP::atom::Hierarchies mhs = IMP::atom::get_by_type(protein,
                                         IMP::atom::RESIDUE_TYPE);
  const size_t num_res = mhs.size();
  ResidueRotamers rotamers;
  rotamers.reserve(num_res);
  ResidueRotamer::Boxes3D bb_boxes(num_res);
  ResidueRotamer::Boxes3D sc_boxes(num_res);
  IMP_LOG_VERBOSE( "Computing bounding boxes" << std::endl);
  std::vector<ResidueRotamer::Boxes3D> rot_boxes(num_res);
  for ( size_t i = 0; i != num_res; ++i )
  {
    IMP::atom::Residue rd = mhs[i].get_as_residue();
    ResidueRotamer rr = get_rotamer(rd, thr);
    rotamers.push_back(rr);
    rr.create_bounding_boxes(bb_boxes[i], sc_boxes[i], rot_boxes[i]);
  }
  IMP_LOG_VERBOSE( "Equation (39)" << std::endl);
  // implementation of equation (39) E^{BB}_{ij}
  std::vector< std::vector<double> > E_bb(num_res);
  for ( size_t i = 0; i != num_res; ++i )
  {
    IMP_LOG_VERBOSE( "Processing residue " << i << " out of " <<
                                             num_res << std::endl);
    IMP::atom::Residue rd_i = mhs[i].get_as_residue();
    IMP::atom::Hierarchies at_i = IMP::atom::get_by_type(rd_i,
                                             IMP::atom::ATOM_TYPE);
    E_bb[i].resize(rotamers[i].get_size());
    // for each rotamer j...
    for ( unsigned j = 1; j < rotamers[i].get_size(); ++j )
    {
      rotamers[i].set_coordinates(j, rd_i);
      // for eah l in Nbr_BB(i)...
      for ( size_t l = 0; l != num_res; ++l )
        if ( l != i )
        {
          if ( !ResidueRotamer::intersect(bb_boxes[l], sc_boxes[i]) )
            continue;
          IMP::atom::Residue rd_l = mhs[i].get_as_residue();
          IMP::atom::Hierarchies at_l = IMP::atom::get_by_type(rd_l,
                                                IMP::atom::ATOM_TYPE);
          // for each k in side chain(i)...
          for ( size_t k = 0; k != at_i.size(); ++k )
          {
            IMP::atom::Atom at_ik = at_i[k].get_as_atom();
            if ( is_backbone(at_ik.get_atom_type().get_index()) )
              continue;
            // for each n in backbone(l)...
            for ( size_t n =0; n != at_l.size(); ++n )
            {
              IMP::atom::Atom at_ln = at_l[n].get_as_atom();
              if ( !is_backbone(at_ln.get_atom_type().get_index()) )
                continue;
              E_bb[i][j]
                += score->evaluate_index(protein->get_model(),
                      IMP::ParticleIndexPair(at_ik.get_particle()->get_index(),
                                       at_ln.get_particle()->get_index()),
                                         nullptr);
            }
          }
        }
      rotamers[i].set_coordinates(0, rd_i);
    }
  }

  // now we are ready to compute eq. (41) and (42)
  std::vector< std::vector<double> > E_P(num_res);
  std::vector< std::vector<double> > q(num_res);
  for ( size_t i = 0; i != num_res; ++i )
  {
    E_P[i].resize(rotamers[i].get_size());
    q[i].resize(rotamers[i].get_size());
  }

  // we will need to iterate num_iter times
  for ( int iter = 0; iter < num_iter; ++iter )
  {
    IMP_LOG_VERBOSE( "Equation (41), iteration " << iter << std::endl);
    // equation (41)...
    for ( size_t i = 0; i != num_res; ++i )
    {
      IMP::atom::Residue rd_i = mhs[i].get_as_residue();
      IMP::atom::Hierarchies at_i = IMP::atom::get_by_type(rd_i,
                                                 IMP::atom::ATOM_TYPE);
      for ( unsigned j = 1; j < rotamers[i].get_size(); ++j )
      {
        E_P[i][j] = E_bb[i][j];
        for ( size_t l = 0; l != num_res; ++l )
          if ( l != i )
          {
            IMP::atom::Residue rd_l = mhs[l].get_as_residue();
            IMP::atom::Hierarchies at_l = IMP::atom::get_by_type(rd_l,
                                                 IMP::atom::ATOM_TYPE);
            for ( unsigned m = 1; m < rotamers[l].get_size(); ++m )
            {
              if ( !ResidueRotamer::intersect(rot_boxes[i][j],
                                              rot_boxes[l][m]) )
                continue;
              double q_lm = rotamers[l].get_probability(m);
              // now compute E^SC_{ijlm} from eq. (40)
              double E_SC = 0;
              rotamers[i].set_coordinates(j, rd_i);
              rotamers[l].set_coordinates(m, rd_l);
              // for k in side-chain(i)...
              for ( size_t k = 0; k != at_i.size(); ++k )
              {
                IMP::atom::Atom at_ik = at_i[k].get_as_atom();
                if ( is_backbone(at_ik.get_atom_type().get_index()) )
                  continue;
                // for n in side-chain(l)...
                for ( size_t n = 0; n != at_l.size(); ++n )
                {
                  IMP::atom::Atom at_ln = at_l[n].get_as_atom();
                  if ( is_backbone(at_ln.get_atom_type().get_index()) )
                    continue;
                  E_SC += score->evaluate_index(protein->get_model(),
                IMP::ParticleIndexPair(at_ik.get_particle()->get_index(),
                                       at_ln.get_particle()->get_index()),
                                          nullptr);
                }
              }
              rotamers[i].set_coordinates(0, rd_i);
              rotamers[l].set_coordinates(0, rd_l);
              E_P[i][j] += q_lm*E_SC;
            }
          }
      }
    }

    IMP_LOG_VERBOSE( "Equation (42), iteration " << iter << std::endl);
    // equation (42)...
    for ( size_t i = 0; i != num_res; ++i )
    {
      double denom = 0;
      for ( unsigned k = 1; k < rotamers[i].get_size(); ++k )
      {
        //IMP_LOG_VERBOSE( "E_P[" << i << "][" << k << "] = " << E_P[i][k] <<
        //std::endl);
        //IMP_LOG_VERBOSE( "P[" << i << "][" << k << "] = " <<
        //rotamers[i].get_probability(k) << std::endl);
        denom += std::exp(-K*E_P[i][k])*rotamers[i].get_probability(k);
      }
      for ( unsigned j = 1; j < rotamers[i].get_size(); ++j )
      {
        q[i][j] = std::exp(-K*E_P[i][j])*rotamers[i].get_probability(j)/denom;
        //IMP_LOG_VERBOSE( "Computed q[" << i << "][" << j << "] = " <<
        //q[i][j] << std::endl);
      }
    }

    // store new probabilities
    for ( size_t i = 0; i != num_res; ++i )
      for ( unsigned j = 1; j < rotamers[i].get_size(); ++j )
        rotamers[i].probabilities_[j] = q[i][j];
  }


  // now we are ready to transform - choose max probability

  IMP_LOG_VERBOSE( "Transforming all residues" << std::endl);
  for ( size_t i = 0; i != num_res; ++i )
  {
    IMP::atom::Residue rd_i = mhs[i].get_as_residue();
    // find max probability
    unsigned best = 0;
    double best_prob = 0;
    for ( unsigned j = 1; j < rotamers[i].get_size(); ++j )
      if ( rotamers[i].get_probability(j) > best_prob )
      {
        best_prob = rotamers[i].get_probability(j);
        best = j;
      }
    if ( best > 0 )
    {
      IMP_LOG_VERBOSE( "Setting coordinates for residue " << rd_i << std::endl);
      rotamers[i].set_coordinates(best, rd_i);
    }
  }
}


IMPROTAMER_END_NAMESPACE
