#include <IMP/core/rigid_bodies.h>
#include <IMP/kernel/Model.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/base/flags.h>
#include <IMP/core/RigidBodyMover.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/kernel/internal/InternalListSingletonContainer.h>
#include <IMP/test/test_macros.h>
#include <IMP/core/internal/CoreClosePairContainer.h>


namespace {
  std::string get_module_name() {return "anon";}
  std::string get_module_version() {return "anon";}
  class ICMover: public IMP::core::MonteCarloMover {
    IMP::kernel::ParticleIndex pi_;
    IMP::algebra::Sphere3D ball_;
    IMP::algebra::Vector3D old_;
  public:
    ICMover(IMP::kernel::Model *m, IMP::kernel::ParticleIndex pi, double r):
      IMP::core::MonteCarloMover(m, "ICMover%1%") {
      pi_ = pi;
      ball_ = IMP::algebra::Sphere3D(IMP::algebra::get_zero_vector_d<3>(),
                                     r);
    }
    IMP::core::MonteCarloMoverResult do_propose() {
      IMP::core::NonRigidMember nrm(get_model(), pi_);
      IMP::algebra::Vector3D ic = nrm.get_internal_coordinates();
      old_ = ic;
      ic += IMP::algebra::get_random_vector_in(ball_);
      nrm.set_internal_coordinates(ic);
      return IMP::core::MonteCarloMoverResult(IMP::kernel::ParticleIndexes(1,
                                                                           pi_),
                                              1.0);
    }
    void do_reject() {
      IMP::core::NonRigidMember nrm(get_model(), pi_);
      nrm.set_internal_coordinates(old_);
    }
    void do_accept() {
    }
    IMP::kernel::ModelObjectsTemp do_get_inputs() const {
      return IMP::kernel::ModelObjectsTemp(1, get_model()->get_particle(pi_));
    }
    IMP_OBJECT_METHODS(ICMover);
  };

  IMP::core::MonteCarloMovers create_ic_movers(IMP::kernel::Model *m,
                                            IMP::kernel::ParticleIndexes pis) {
    IMP::core::MonteCarloMovers ret;
    IMP_FOREACH(IMP::kernel::ParticleIndex pi, pis) {
      ret.push_back(new ICMover(m, pi, 1));
    }
    return ret;
  }

  IMP::kernel::ParticleIndexes get_not_bodies(IMP::kernel::Model *m,
                                      const IMP::kernel::ParticleIndexes &in) {
    IMP::kernel::ParticleIndexes ret;
    for (unsigned int i = 0; i < in.size(); ++i) {
      if (!IMP::core::RigidBody::get_is_setup(m, in[i])) {
        ret.push_back(in[i]);
      }
    }
    return ret;
  }

  IMP::core::RigidBody create_rigid_body(IMP::kernel::Model *m,
                                         std::string name) {
    IMP::kernel::ParticleIndex pi = m->add_particle("rb" + name);
    IMP::core::RigidBody ret
      = IMP::core::RigidBody::setup_particle(m, pi,
                                             IMP::algebra::ReferenceFrame3D());
    ret.set_coordinates_are_optimized(true);
    for (unsigned int i = 0; i < 3; ++i) {
      IMP::kernel::ParticleIndex cpi = m->add_particle("pm" + name);
      IMP::algebra::Vector3D v = IMP::algebra::get_zero_vector_d<3>();
      v[i] = 1;
      IMP::core::XYZR::setup_particle(m, cpi, IMP::algebra::Sphere3D(v, 1));
      ret.add_member(cpi);
    }
    return ret;
  }

  IMP::kernel::ParticleIndexes add_non_rigid(IMP::kernel::Model *m,
                                             IMP::core::RigidBody rb,
                                             std::string name) {
    IMP::kernel::ParticleIndexes ret;
    for (unsigned int i = 0; i < 2; ++i) {
      IMP::kernel::ParticleIndex pi = m->add_particle("nrb" + name);
      IMP::algebra::Vector3D v = IMP::algebra::get_zero_vector_d<3>();
      v[i] = -1;
      IMP::core::XYZR::setup_particle(m, pi, IMP::algebra::Sphere3D(v, 1));
      rb.add_non_rigid_member(pi);
      ret.push_back(pi);
    }
    return ret;
  }

  void move(IMP::core::MonteCarloMovers mvs) {
    IMP_FOREACH(IMP::core::MonteCarloMover *m, mvs) {
      m->propose();
      m->accept();
    }
  }

  void check_close_pairs(IMP::kernel::Model *m,
                         IMP::kernel::ParticleIndexPairs cpcpps) {

    m->update();
    IMP::kernel::ParticleIndexes ps
      = get_not_bodies(m, m->get_particle_indexes());
    for (unsigned int i = 0; i < ps.size(); ++i) {
      if (IMP::core::NonRigidMember::get_is_setup(m, ps[i])) {
        IMP::core::NonRigidMember d(m, ps[i]);
        IMP::algebra::Vector3D ic = d.get_internal_coordinates();
        IMP::algebra::Vector3D oc = d.get_coordinates();
        IMP::algebra::ReferenceFrame3D rf
          = d.get_rigid_body().get_reference_frame();
        IMP::algebra::Vector3D oic = rf.get_global_coordinates(ic);
        double dist = IMP::algebra::get_distance(oic, oc);

        IMP_TEST_LESS_THAN(dist, .1);
      }
      for (unsigned int j = 0; j < i; ++j) {
        IMP::kernel::ParticleIndexPair pp(ps[i], ps[j]);
        IMP::kernel::ParticleIndexPair ppi(ps[j], ps[i]);
        IMP::core::XYZR d0(m, pp[0]);
        IMP::core::XYZR d1(m, pp[1]);
        if (IMP::core::RigidMember::get_is_setup(m, ps[i])
            && IMP::core::RigidMember::get_is_setup(m, ps[j])
            && IMP::core::RigidMember(m, ps[i]).get_rigid_body()
            == IMP::core::RigidMember(m, ps[j]).get_rigid_body()) {
          IMP_TEST_TRUE(std::find(cpcpps.begin(), cpcpps.end(), pp)
                        == cpcpps.end()
                        && std::find(cpcpps.begin(), cpcpps.end(), ppi)
                        == cpcpps.end());
        } else {
          double d = IMP::core::get_distance(d0, d1);
          if (d < 0) {
            IMP_TEST_TRUE(std::find(cpcpps.begin(), cpcpps.end(), pp)
                          != cpcpps.end()
                          || std::find(cpcpps.begin(), cpcpps.end(), ppi)
                          != cpcpps.end());
          }
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  try {
  IMP::base::setup_from_argv(argc, argv,
               "Test non rigid first test in a way that works with valgrind.");
  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::core::RigidBody r0 = create_rigid_body(m, "0");
  IMP::core::RigidBody r1 = create_rigid_body(m, "1");
  IMP::kernel::ParticleIndexes nr0 = add_non_rigid(m, r0, "0");
  IMP::kernel::ParticleIndexes nr1 = add_non_rigid(m, r1, "1");
  IMP_NEW(IMP::kernel::internal::InternalListSingletonContainer,
          lsc, (m, "lsc"));
  lsc->set(get_not_bodies(m, m->get_particle_indexes()));
  IMP_NEW(IMP::core::internal::CoreClosePairContainer, cpc,
          (lsc, 0, new IMP::core::RigidClosePairsFinder(), 0));
  IMP::core::MonteCarloMovers movers;
  movers.push_back(new IMP::core::RigidBodyMover(m, r0.get_particle_index(),
                                                 1, 1));
  movers.push_back(new IMP::core::RigidBodyMover(m, r1.get_particle_index(),
                                                 1, 1));
  movers += create_ic_movers(m, nr0 + nr1);
  for (unsigned int i = 0; i < 100; ++i) {
    std::cout << i << std::endl;
    m->update();
    check_close_pairs(m, cpc->get_indexes());
    move(movers);
  }
  } catch (IMP::base::Exception e) {
    std::cerr << "Failed with exception " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
