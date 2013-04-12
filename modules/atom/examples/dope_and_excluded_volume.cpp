/** \example score_functor/dope_and_excluded_volume.cpp

    This example shows you a way to create a pair score that combines
    IMP::score_functor::Dope and excluded volume (via
    IMP::score_functor::HarmonicLowerBound).
*/

#include <IMP/score_functor/Dope.h>
#include <IMP/score_functor/HarmonicLowerBound.h>
#include <IMP/score_functor/DistancePairScore.h>
#include <IMP/score_functor/SphereDistance.h>
#include <IMP/score_functor/AddScores.h>
#include <IMP/atom/DopePairScore.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/force_fields.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/base/object_macros.h>
#include <IMP/core/XYZR.h>
#include <IMP/base/flags.h>
#include <IMP/base/nullptr_macros.h>

namespace {
  const double dope_threshold=16;
  const double spring_constant=1;

  // create some pairs that can be score with dope
  IMP::ParticleIndexPairs setup_pairs(IMP::Model *m) {
    IMP::ParticleIndex rpi= m->add_particle("root");
    IMP::atom::Chain chain=IMP::atom::Chain::setup_particle(m, rpi, 'A');
    IMP::ParticleIndexes atoms;
    for (unsigned int i=0; i< 2; ++i) {
      IMP::ParticleIndex rpi= m->add_particle("residue");
      IMP::atom::Residue residue
        = IMP::atom::Residue::setup_particle(m, rpi,
                                             IMP::atom::ALA, i);
      chain.add_child(residue);

      IMP::ParticleIndex api= m->add_particle("atom");
      IMP::atom::Atom atom
        = IMP::atom::Atom::setup_particle(m, api,
                                          IMP::atom::AT_CA);
      IMP::algebra::Vector3D coords(0,10*i, 0);
      IMP::core::XYZ::setup_particle(m, api, coords);
      residue.add_child(atom);
      atoms.push_back(api);
    }

    // add Dope atom types
    IMP::atom::add_radii(chain);
    IMP::atom::add_dope_score_data(chain);

    IMP::ParticleIndexPairs all_pairs;
    for (unsigned int i=0; i< atoms.size(); ++i) {
      for (unsigned int j=0; j< i; ++j) {
        all_pairs.push_back(IMP::ParticleIndexPair(atoms[i], atoms[j]));
      }
    }
    return all_pairs;
  }
}

int main(int argc, char *argv[]) {
  try {
  // do normal IMP initialization of command line arguments
  // Run with --help to see options.
  IMP::base::setup_from_argv(argc, argv,
                             "Show to to use dope and excluded volume");

  typedef IMP::score_functor::HarmonicLowerBound Harmonic;
  typedef IMP::score_functor::SphereDistance<Harmonic> SoftSphere;
  typedef IMP::score_functor::AddScores<IMP::score_functor::Dope,
                                        SoftSphere> Score;
  typedef IMP::score_functor::DistancePairScore<Score>
    DopeAndExcludedVolumeDistancePairScore;

  // create one
  IMP::base::OwnerPointer<DopeAndExcludedVolumeDistancePairScore>
    score= new DopeAndExcludedVolumeDistancePairScore
    (Score(IMP::score_functor::Dope(dope_threshold),
           SoftSphere(Harmonic(spring_constant))));

  // Now let's use it
  IMP::base::OwnerPointer<IMP::Model> model= new IMP::Model();
  IMP::ParticleIndexPairs pips= setup_pairs(model);

  for (unsigned int i=0; i< pips.size(); ++i) {
    std::cout << "Score is " << score->evaluate_index(model, pips[i],
                                                      IMP_NULLPTR)
            << std::endl;
  }
  return 0;
  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
