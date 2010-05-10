/**
 *  \file sampling_space_utils.cpp
 *  \brief setting up tools
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/sampling_space_utils.h>
#include <IMP/domino/TransformationMappedDiscreteSet.h>
IMPMULTIFIT_BEGIN_NAMESPACE

domino::TransformationCartesianProductSampler *
  set_sampling_space(const DataContainer &data) {
  IMP_INTERNAL_CHECK(data.get_number_of_components()>0,
                     "no components to sample \n");
  Particles components;
  for(int i=0;i<data.get_number_of_components();i++) {
    Particle *comp = data.get_component(i);
    components.push_back(comp);
  }
  IMP_NEW(container::ListSingletonContainer,comp_cont,(components));
  //  IMP_NEW(domino::TransformationMappedDiscreteSet, discrete_set,());
  domino::TransformationMappedDiscreteSet *discrete_set=
    new domino::TransformationMappedDiscreteSet(comp_cont);
  IntKey orig_ind_key = IntKey("orig_ind");
  Model *mdl = data.get_component(0)->get_model();
  //add valid transformation for each rigid body

  for(int i=0;i<data.get_number_of_components();i++) {
    //the component
    Particle *comp = data.get_component(i);
    IMP_LOG(VERBOSE,"set sampling space for particle:"<<
            comp->get_name()<<std::endl);
    //the subset of fitting solutions close to the mapped ap
    IMP_LOG(VERBOSE,"getting fits"<<std::endl);
    FittingSolutionRecords ts = data.get_fitting_solutions(comp);
    IMP_LOG(VERBOSE,"number of fits:"<<ts.size()<<std::endl);
    //add these fitting solutions to the sampling space
    for(unsigned int ts_i=0;ts_i<ts.size();ts_i++) {
      Particle *state_p = new Particle(mdl);
      domino::Transformation::setup_particle(
        state_p,ts[ts_i].get_transformation(),ts_i);
      state_p->add_attribute(orig_ind_key,ts_i);//TODO - why is it needed?
      discrete_set->add_mapped_state(comp,state_p);
    }
    IMP_LOG(VERBOSE,"end setting sampling space for particle"<<std::endl);
  }
  domino::TransformationCartesianProductSampler *sampler =
    new domino::TransformationCartesianProductSampler(
           discrete_set,components,true);
  //  IMP_NEW(domino::TransformationCartesianProductSampler,sampler,
  //  (discrete_set,components,true));
  return sampler;
}
IMPMULTIFIT_END_NAMESPACE
