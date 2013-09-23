/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <string>
#include <cstdlib>

#include <IMP.h>
#include <IMP/kernel/Model.h>
#include <IMP/kernel/Particle.h>
#include <IMP/base.h>
#include <IMP/core.h>
#include <IMP/macros.h>

#include <IMP/saxs/Profile.h>
#include <IMP/isd.h>

#include <IMP/benchmark/utility.h>
#include <IMP/benchmark/benchmark_macros.h>
#include <IMP/base/flags.h>

namespace {
using namespace IMP;
using namespace IMP::base;
using namespace IMP::isd;

FloatsList read_profile(std::string name, unsigned subs, unsigned cut)
{
    saxs::Profile *prof = new saxs::Profile(name);
    FloatsList ret;
    Floats q,I,err;
    for (unsigned i=0; i<prof->size(); i++){
        if (i>=cut) break;
        if (i%subs!=0) continue;
        q.push_back(prof->get_q(i));
        I.push_back(prof->get_intensity(i));
        err.push_back(prof->get_error(i));
    }
    //delete prof;

    ret.push_back(q);
    ret.push_back(I);
    ret.push_back(err);
    return ret;
}

Scales setup_particles(IMP::kernel::Model *m)
{
    IMP_NEW(kernel::Particle, pG, (m));
    Scale G = Scale(Scale::setup_particle(pG, 300.));
    IMP_NEW(kernel::Particle, pRg, (m));
    Scale Rg = Scale(Scale::setup_particle(pRg, 30.));
    IMP_NEW(kernel::Particle, pd, (m));
    Scale d = Scale(Scale::setup_particle(pd, 4.));
    IMP_NEW(kernel::Particle, ps, (m));
    Scale s = Scale(Scale::setup_particle(ps, 0.));
    IMP_NEW(kernel::Particle, pA, (m));
    Scale A = Scale(Scale::setup_particle(pA, 0.));
    IMP_NEW(kernel::Particle, ptau, (m));
    Scale tau = Scale(Scale::setup_particle(ptau, 10.));
    IMP_NEW(kernel::Particle, plambda, (m));
    Scale lambda = Scale(Scale::setup_particle(plambda, 0.08));
    IMP_NEW(kernel::Particle, psigma, (m));
    Scale sigma = Scale(Scale::setup_particle(psigma, 10.));
    s.set_upper(3.);
    s.set_upper(d);
    IMP_NEW(JeffreysRestraint, jrs, (m, sigma));
    m->add_restraint(jrs);
    IMP_NEW(JeffreysRestraint, jrl, (m, lambda));
    m->add_restraint(jrl);
    lambda.set_lower(0.001);
    sigma.set_lower(1.);
    G.set_scale_is_optimized(true);
    Rg.set_scale_is_optimized(true);
    d.set_scale_is_optimized(true);
    s.set_scale_is_optimized(true);
    A.set_scale_is_optimized(false);
    tau.set_scale_is_optimized(true);
    lambda.set_scale_is_optimized(true);
    sigma.set_scale_is_optimized(true);
    Scales scales;
    scales.push_back(G);
    scales.push_back(Rg);
    scales.push_back(d);
    scales.push_back(s);
    scales.push_back(A);
    scales.push_back(tau);
    scales.push_back(lambda);
    scales.push_back(sigma);
    return scales;
}

IMP::kernel::Model *do_setup(std::string profile, unsigned subs, unsigned cut)
{
  FloatsList data(read_profile(profile,subs,cut));
  FloatsList qvals;
  double qmax = 0;
  double qmin = 9999999;
  for (unsigned i=0; i<data[0].size(); i++){
      Floats q;
      q.push_back(data[0][i]);
      qvals.push_back(q);
      if (q[0] < qmin) qmin = q[0];
      if (q[0] > qmax) qmax = q[0];
  }
  //start gpi instance and possibly gpr also
  IMP_NEW(kernel::Model, m, ());
  Scales particles(setup_particles(m));
  IMP_NEW(GeneralizedGuinierPorodFunction, mean, (particles[0].get_particle(),
              particles[1].get_particle(), particles[2].get_particle() ,
              particles[3].get_particle(), particles[4].get_particle()));
  IMP_NEW(Covariance1DFunction, covariance,
          (particles[5].get_particle(),particles[6].get_particle(),2.0));
  IMP_NEW(GaussianProcessInterpolation, gpi, (qvals,
          data[1], data[2], 10, mean, covariance, particles[7]));
  IMP_NEW(GaussianProcessInterpolationRestraint, gpr, (m,gpi));
  m->add_restraint(gpr);
  //gpi->get_posterior_covariance(qvals[0],qvals[0]); //precompute matrices
  m->evaluate(true);
  return m.release();
}

void run_benchmark(std::string profile, unsigned subs, unsigned cut)
{
  double time;
  base::Pointer<IMP::kernel::Model> m;
  IMP_TIME({ m = do_setup(profile, subs, cut); }, time);
  IMP::benchmark::report("setup", time, 0.);

  IMP_TIME({ m->evaluate(true); }, time);
  IMP::benchmark::report("evaluate", time, 0.);
}

  boost::int64_t subs = -1, cut = -1;
  std::string profile;
  IMP::base::AddStringFlag fprofile("profile", "The [optional] profile",
                                    &profile);
  IMP::base::AddIntFlag fsubs("subs", "Something or another", &subs);
  IMP::base::AddIntFlag fcut("cutoff", "Something else", &cut);

}

int main(int argc, char **argv) {
  IMP::base::setup_from_argv(argc, argv,
                             "Benchmark a gaussian process");
  if (cut == -1) {
    // Run benchmark with defaults
    run_benchmark(IMP::benchmark::get_data_path("lyzexp.dat"), 1, 200);
  } else {
    run_benchmark(profile, subs, cut);
  }
  return IMP::benchmark::get_return_value();
}
