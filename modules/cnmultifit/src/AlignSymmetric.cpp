/**
 *  \file AlignSymmetric.cpp
 *  \brief Fast alignment of a cyclic model to its density.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/cnmultifit/AlignSymmetric.h>
#include <IMP/cnmultifit/MolCnSymmAxisDetector.h>

IMPCNMULTIFIT_BEGIN_NAMESPACE

namespace {
// sort eigen values such that the symm axis appears first
void sort_helper(const algebra::PrincipalComponentAnalysis &pca,
                 int symm_mol_axis_ind,
                 Floats &ev_sorted) {
  Ints sorted_ind(3);
  if (symm_mol_axis_ind==0) {
    sorted_ind[0]=1;sorted_ind[1]=2;sorted_ind[2]=0;
  }
  else if (symm_mol_axis_ind == 1) {
    sorted_ind[0]=0;sorted_ind[1]=2;sorted_ind[2]=1;
  }
  else {
    sorted_ind[0]=0;sorted_ind[1]=1;sorted_ind[2]=2;
  }
  for(int i=0;i<3;i++) {
    ev_sorted[i]=std::sqrt(pca.get_principal_value(sorted_ind[i]));
  }
}
}

algebra::Transformation3Ds
AlignSymmetric::generate_cn_density_translations() const
{
  float trans_delta=spacing_;float max_trans=spacing_*5;
  algebra::Transformation3Ds ret_trans;
  algebra::Vector3D symm_axis=symm_map_->get_symmetry_axis();
  symm_axis = symm_axis*(1./algebra::get_l2_norm(symm_axis));
  float curr_trans=-max_trans;
  algebra::Rotation3D id_rot = algebra::get_identity_rotation_3d();
  while(curr_trans<max_trans){
    ret_trans.push_back(
                        algebra::Transformation3D(id_rot,
                                                  symm_axis*curr_trans));
    curr_trans+=trans_delta;
  }
  return ret_trans;
}
algebra::Transformation3Ds AlignSymmetric::generate_cn_density_rotations(
                              bool fine) const{
  algebra::Transformation3Ds ret_trans;
  float angle_delta=10.;
  if (fine) angle_delta=5.;
  int num_angles_explored=(360./cn_symm_deg_)/angle_delta;
  algebra::Vector3D symm_axis=symm_map_->get_symmetry_axis();
  algebra::Vector3D cen=symm_map_->get_pca().get_centroid();
  //translate such that the symm axis will pass through 0
  algebra::Transformation3D move_to_zero(algebra::get_identity_rotation_3d(),
                                         -cen);
  algebra::Transformation3D move_to_zero_inv = move_to_zero.get_inverse();

  for(int symm_ind=0;symm_ind<num_angles_explored;symm_ind++) {
    ret_trans.push_back(move_to_zero_inv*algebra::get_rotation_about_axis(
                         symm_axis,
                         PI*angle_delta*symm_ind/180)*move_to_zero);
  }
  return ret_trans;
}


AlignSymmetric::AlignSymmetric(em::DensityMap *dmap,float dens_t,
                               int cn_symm_deg){
  int dn_symm_deg=1;
  spacing_=dmap->get_spacing();
  cn_symm_deg_=cn_symm_deg;dn_symm_deg_=dn_symm_deg;
  symm_map_.reset(new CnSymmAxisDetector(cn_symm_deg_, dmap, dens_t, 0.0));
  int symm_map_axis_ind = symm_map_->get_symmetry_axis_index();
  map_v_ = Floats(3);
  sort_helper(symm_map_->get_pca(),
              symm_map_axis_ind,map_v_);
}

int AlignSymmetric::score_alignment(atom::Hierarchies mhs,
                               float max_allowed_diff) {
  MolCnSymmAxisDetector symm_mol(cn_symm_deg_, mhs);
  int symm_mol_axis_ind = symm_mol.get_symmetry_axis_index();
  int count=0;
  Floats mol_v(3);//the principal values of non-symm axis first
  sort_helper(symm_mol.get_pca(),symm_mol_axis_ind,mol_v);

  IMP_LOG_VERBOSE(
          "MAP :"<<map_v_[0]<<","<<map_v_[1]<<","<<map_v_[2]<<
          " MOL :" <<mol_v[0]<<","<<mol_v[1]<<","<<mol_v[2]<<std::endl);

  if (std::abs(map_v_[2]-dn_symm_deg_*mol_v[2])<max_allowed_diff){
    count++;
  }
  for(int ii=0;ii<2;ii++){
    if (std::abs(map_v_[ii]-mol_v[ii])<max_allowed_diff){
      count++;
    }
  }

  IMP_LOG_VERBOSE("MAP :"<<map_v_[0]<<","<<map_v_[1]<<","<<map_v_[2]<<
          " MOL :" <<mol_v[0]<<","<<mol_v[1]<<","<<mol_v[2]<<" counter:"
          <<count<<" dn_symm_deg:"<<dn_symm_deg_<<" diff:"<<max_allowed_diff
          <<std::endl);

  return count;
}

algebra::Transformation3Ds
  AlignSymmetric::get_symm_axis_alignments_from_model_to_density(
                            atom::Hierarchies mhs,
                            bool sample_translation,
                            bool fine_rotation_sampling) const {
  MolCnSymmAxisDetector symm_mol(cn_symm_deg_, mhs);
  int symm_mol_axis_ind = symm_mol.get_symmetry_axis_index();

  algebra::PrincipalComponentAnalysis map_pca = symm_map_->get_pca();
  algebra::PrincipalComponentAnalysis mol_pca = symm_mol.get_pca();

  //get all transformations from the mol_pca to the map_pca
  algebra::Transformation3Ds pca_trans;
  //pca_trans = algebra::get_alignments_from_first_to_second(mol_pca,map_pca);

  int mol_axis[2];
  if (symm_mol_axis_ind==0) {mol_axis[0]=1;mol_axis[1]=2;}
  else if (symm_mol_axis_ind==1) {mol_axis[0]=0;mol_axis[1]=2;}
  else {mol_axis[0]=0;mol_axis[1]=1;}
  int symm_map_axis_ind = symm_map_->get_symmetry_axis_index();
  int map_axis[2];
  if (symm_map_axis_ind==0) {map_axis[0]=1;map_axis[1]=2;}
  else if (symm_map_axis_ind==1) {map_axis[0]=0;map_axis[1]=2;}
  else {map_axis[0]=0;map_axis[1]=1;}

  algebra::Rotation3D map_rot = algebra::get_rotation_from_x_y_axes(
                    map_pca.get_principal_component(map_axis[0]),
                    map_pca.get_principal_component(map_axis[1]));
  int x_sign[2];x_sign[0]=1;x_sign[1]=-1;
  algebra::ReferenceFrame3D map_rf(algebra::Transformation3D(
       map_rot,map_pca.get_centroid()));
  for(int i=0;i<2;i++){
  algebra::Rotation3D mol_rot = algebra::get_rotation_from_x_y_axes(
        mol_pca.get_principal_component(mol_axis[0])*x_sign[i],
        mol_pca.get_principal_component(mol_axis[1]));
  algebra::ReferenceFrame3D mol_rf(algebra::Transformation3D(
                               mol_rot,mol_pca.get_centroid()));
  algebra::Transformation3D mol_pca2map_pca =
    algebra::get_transformation_from_first_to_second(
                                                     mol_rf,map_rf);

  pca_trans.push_back(mol_pca2map_pca);
  mol_rot = algebra::get_rotation_from_x_y_axes(
        mol_pca.get_principal_component(mol_axis[1])*x_sign[i],
        mol_pca.get_principal_component(mol_axis[0]));
  mol_rf=algebra::ReferenceFrame3D(algebra::Transformation3D(
                                           mol_rot,mol_pca.get_centroid()));
  mol_pca2map_pca =
    algebra::get_transformation_from_first_to_second(
                                                     mol_rf,map_rf);
  pca_trans.push_back(mol_pca2map_pca);
  }

  //get all rotations around the mol center
  algebra::Transformation3Ds symm_dens_rots = generate_cn_density_rotations(
                                     fine_rotation_sampling);
  //get all translations around the mol axis
  algebra::Transformation3Ds symm_dens_trans;
  if (sample_translation) {
    symm_dens_trans= generate_cn_density_translations();
  }
  else {
    symm_dens_trans.push_back(algebra::get_identity_transformation_3d());
  }
  algebra::Transformation3Ds rtrans;
  for(algebra::Transformation3Ds::const_iterator pca_it = pca_trans.begin();
      pca_it != pca_trans.end(); pca_it++) {
    for(algebra::Transformation3Ds::const_iterator
        rot_it = symm_dens_rots.begin();
        rot_it != symm_dens_rots.end(); rot_it++) {
      for(algebra::Transformation3Ds::const_iterator
          trans_it = symm_dens_trans.begin();
          trans_it != symm_dens_trans.end(); trans_it++) {
        rtrans.push_back((*trans_it)*(*rot_it)*(*pca_it));
      }
    }
  }
  return rtrans;
}

IMPCNMULTIFIT_END_NAMESPACE
