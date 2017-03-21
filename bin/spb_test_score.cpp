/**
 * \brief test score of a model
 *
 * Given an RMF of structural coordaintes along with the
 * RMF of ISD particles the scores of all restraints are
 * printed out.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <IMP/membrane.h>
#include <IMP/display.h>
#include <IMP/rmf.h>
#include <string>
#include <list>
#include <map>
#include <math.h>
#include <time.h>
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <IMP/flags.h>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{
 IMP::setup_from_argv(argc,argv,"Test score of a given model","modelrmf isdrmf"
,2);

 // read input
 std::string inputfile="config.ini";

 // parsing input
 SPBParameters mydata=get_SPBParameters(inputfile,"0");

 // create a new model
 IMP_NEW(Model,m,());
 IMP_NEW(RestraintSet, allrs, (m, "All restraints"));

 // List of particles for layer restraint
 IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
 IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));

 // List of MonteCarloMovers for MC, not used here
 core::MonteCarloMovers mvs;

 //
 // ISD PARTICLES
 //
 std::map<std::string,IMP::Pointer<Particle> > ISD_ps=
   add_ISD_particles(m,allrs,mydata,mvs);
 // create list of particles from map
 Particles ISD_ps_list;
 std::map<std::string, IMP::Pointer<Particle> >::iterator itr;
 for(itr = ISD_ps.begin(); itr != ISD_ps.end(); ++itr){
  ISD_ps_list.push_back((*itr).second);
 }
 //
 // PROTEIN REPRESENTATION
 //
 atom::Hierarchies all_mol=
   create_representation(m,allrs,mydata,CP_ps,IL2_ps,mvs,
     ISD_ps["SideXY"],ISD_ps["SideZ"],0);

 //
 // CREATING RESTRAINTS
 //
 std::map< std::string, IMP::Pointer<RestraintSet> > rst_map=
        spb_assemble_restraints(m,allrs,mydata,all_mol,CP_ps,IL2_ps,ISD_ps);


 //
 // CREATE LIST TO LINK TO RMF
 //
 atom::Hierarchies hhs;
 for(unsigned int i=0;i<all_mol.size();++i){
  atom::Hierarchies hs=all_mol[i].get_children();
  for(unsigned int j=0;j<hs.size();++j) {hhs.push_back(hs[j]);}
 }

 // print statistics
 double fretr_score=0.0;
 double y2h_score=0.0;

 // open rmf for coordinates
 RMF::FileConstHandle rhc =
   RMF::open_rmf_file_read_only(argv[1]);
 // read various info
 RMF::Category my_kc_read   = rhc.get_category("my data");

 RMF::FloatKey my_key0_read = rhc.get_key<RMF::FloatTag>(my_kc_read,"my score");
 RMF::IntKey   my_key1_read = rhc.get_key<RMF::IntTag>(my_kc_read,"my index");
 RMF::FloatKey   my_key2_read = rhc.get_key<
 RMF::FloatTag>(my_kc_read,"my bias");

 // linking hierarchies
 rmf::link_hierarchies(rhc, hhs);
 // number of frames
 unsigned int nframes=rhc.get_number_of_frames();

 // open rmf for ISD particles
 RMF::FileConstHandle rhc_ISD =
        RMF::open_rmf_file_read_only(argv[2]);
 // linking particles
 rmf::link_particles(rhc_ISD, ISD_ps_list);
 // number of frames
 unsigned int nframes_ISD=rhc_ISD.get_number_of_frames();

 // check number of frames are the same
 if(nframes!=nframes_ISD) {exit(1);}


 // cycle on frames
 unsigned int imc=0;
 for(unsigned int imc = 0; imc < nframes; imc++){

   // load coordinates
   rmf::load_frame(rhc,RMF::FrameID(imc));
   //rhc.set_current_frame(RMF::FrameID(imc));

   // and ISD particles
   //rhc_ISD.set_current_frame(RMF::FrameID(imc));
   rmf::load_frame(rhc_ISD,RMF::FrameID(imc));

   // get score and REM index
   Float score = (rhc.get_root_node()).get_value(my_key0_read);
   Int myindex = (rhc.get_root_node()).get_value(my_key1_read);
   Float bias = (rhc.get_root_node()).get_value(my_key2_read);

  // do the scoring here
  //

  double totalscore = allrs->evaluate(false);

  std::cout << "Stored score" << score <<
  " Calculated score " <<totalscore <<std::endl;
   if(mydata.add_fret){fretr_score=rst_map["FRET_R"]->evaluate(false);}
   if(mydata.add_y2h) {y2h_score=rst_map["Y2H"]->evaluate(false);}

         // get all the restraints in the model

         for(unsigned i=0;i<allrs->get_number_of_restraints();++i) {
         IMP::Pointer<IMP::Restraint> rst=dynamic_cast<
            IMP::Restraint*>(allrs->get_restraint(i));
         std::cout<< rst->get_name() << " " << rst->get_score() <<std::endl;
  }
  for(unsigned i=0;i<rst_map["FRET_R"]->get_number_of_restraints();++i){
            IMP::Pointer<isd::FretRestraint> rst=
            dynamic_cast<isd::FretRestraint*>(
            rst_map["FRET_R"]->get_restraint(i));
    std::string name = rst->get_name();
    Float fmod       = rst->get_model_fretr();
    Float fmod_err   = rst->get_standard_error();
    Float fexp       = rst->get_experimental_value();

     printf("TimeStep %10d Name %30s  Model %6.3f",imc,name.c_str(),fmod);
     printf("Model_Error %6.3f  Exp %6.3f\n",fmod_err,fexp);

      }

 break;
 }

 // close RMFs
 rhc     = RMF::FileHandle();
 rhc_ISD = RMF::FileHandle();


 return 0;


}
