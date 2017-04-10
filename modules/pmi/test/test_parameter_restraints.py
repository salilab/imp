from __future__ import print_function, division
import math
import IMP
import IMP.isd
import IMP.pmi
import IMP.pmi.restraints.parameters
import IMP.test


class Tests(IMP.test.TestCase):

    def test_weight_restraint(self):
        m = IMP.Model()
        weight = IMP.isd.Weight.setup_particle(IMP.Particle(m))
        weight.add_weight()
        weight.add_weight()
        weight.set_weights([.1, .9])
        lower, upper, kappa = .2, .8, 1.
        r = IMP.pmi.restraints.parameters.WeightRestraint(weight, lower,
                                                          upper, kappa)
        self.assertAlmostEqual(
            float(r.get_output()["_TotalScore"]), 0.01, delta=1e-6)

        weight.add_weight()
        weight.set_weights([0., .15, .85])
        self.assertAlmostEqual(
            float(r.get_output()["_TotalScore"]), .0225, delta=1e-6)

    def test_jeffreys_prior(self):
        m = IMP.Model()
        sigma = IMP.isd.Scale.setup_particle(IMP.Particle(m))
        sigma.set_scale(1.)
        r = IMP.pmi.restraints.parameters.JeffreysPrior(sigma)

        self.assertAlmostEqual(
            float(r.get_output()["_TotalScore"]), 0., delta=1e-6)

        sigma.set_scale(2.)
        self.assertAlmostEqual(
            float(r.get_output()["_TotalScore"]), -math.log(.5), delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
