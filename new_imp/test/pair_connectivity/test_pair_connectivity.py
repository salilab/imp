import unittest
import os
import IMP
import IMP.test
import IMP.utils

class PairConnectivityRestraintTests(IMP.test.IMPTestCase):
    """Test pair connectivity restraints."""

    def setUp(self):
        """Build test model and optimizer"""
        self.imp_model = IMP.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        for p in range(12):
            self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                        0., 0., 0.))
        p1 = self.particles[0]
        radkey= IMP.FloatKey("radius")
        pkey=IMP.IntKey("protein")
        idkey=IMP.IntKey("id")

        p1.add_attribute(radkey, 1.0, False)
        p1.add_attribute(pkey, 1)
        p1.add_attribute(idkey, 1)

        p1 = self.particles[1]
        p1.add_attribute(radkey, 1.0, False)
        p1.add_attribute(pkey, 1)
        p1.add_attribute(idkey, 2)

        p1 = self.particles[2]
        p1.add_attribute(radkey, 1.0, False)
        p1.add_attribute(pkey, 1)
        p1.add_attribute(idkey, 3)

        p1 = self.particles[3]
        p1.add_attribute(radkey, 1.5, False)
        p1.add_attribute(pkey, 1)
        p1.add_attribute(idkey, 4)

        p1 = self.particles[4]
        p1.add_attribute(radkey, 1.5, False)
        p1.add_attribute(pkey, 1)
        p1.add_attribute(idkey, 5)

        p1 = self.particles[5]
        p1.add_attribute(radkey, 1.5, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 6)

        p1 = self.particles[6]
        p1.add_attribute(radkey, 1.5, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 7)

        p1 = self.particles[7]
        p1.add_attribute(radkey, 2.0, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 8)

        p1 = self.particles[8]
        p1.add_attribute(radkey, 2.0, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 9)

        p1 = self.particles[9]
        p1.add_attribute(radkey, 2.0, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 10)

        p1 = self.particles[10]
        p1.add_attribute(radkey, 2.0, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 11)

        p1 = self.particles[11]
        p1.add_attribute(radkey, 2.0, False)
        p1.add_attribute(pkey, 2)
        p1.add_attribute(idkey, 12)

        self.opt = IMP.ConjugateGradients()
        self.opt.set_model(self.imp_model)
        self.opt.set_threshold(1e-4)

    def _run_test_pair_connectivity(self, use_python):
        """Test pair connectivity restraint.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins """
        rs = IMP.RestraintSet("connect")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint(rs)

        # add connectivity restraints

        particle_indexes1 = IMP.Ints()
        particle_indexes2 = IMP.Ints()
        rsrs = []
        # connect 2 proteins together by two beads
        particle_indexes1.clear()
        for i in range(0, 5):
            particle_indexes1.push_back(i)
        particle_indexes2.clear()
        for i in range(5, 12):
            particle_indexes2.push_back(i)
        num_connects = 3

        if use_python:
            # set up exclusion volumes using a Python loop:
            IMP.utils.set_up_exclusion_volumes(self.imp_model, self.particles,
                                               IMP.FloatKey("radius"), rsrs)
        else:
            # use the C++ exclusion volume restraint:
            score_func_params = IMP.BasicScoreFuncParams("harmonic_lower_bound",
                                                         0.0, 0.1)
            rsrs.append(IMP.ExclusionVolumeRestraint(self.imp_model,
                                                     particle_indexes1,
                                                     particle_indexes2,
                                                     IMP.FloatKey("radius"),
                                                     score_func_params))

        # it should work whether this is True or False
        # However, if it is False, the close pairs should all be between
        # distinct particles
        particle_reuse = False
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     0.0, 0.1)
        rsrs.append(IMP.PairConnectivityRestraint(self.imp_model,
                                                  particle_indexes1,
                                                  particle_indexes2,
                                                  IMP.FloatKey("radius"),
                                                  score_func_params,
                                                  num_connects, particle_reuse))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.randomize_particles(self.particles, 50.0)
        self.opt.optimize(55)

        # min distances
        min_dist = []
        for i in range(num_connects):
            min_dist.append(10000000)

        for (i, pi) in enumerate(self.particles[0:5]):
            icoord = (pi.x(), pi.y(), pi.z())
            irad = pi.get_value(IMP.FloatKey("radius"))
            for (j, pj) in enumerate(self.particles[5:12]):
                jcoord = (pj.x(), pj.y(), pj.z())
                jrad = pj.get_value(IMP.FloatKey("radius"))
                d = self.get_distance(icoord, jcoord) - irad - jrad
                found = False
                for k in range(num_connects):
                    if not found:
                        if d < min_dist[k]:
                            found = True

                    if found:
                        d1 = min_dist[k]
                        min_dist[k] = d
                        d = d1

        for i in range(num_connects):
            self.assert_(min_dist[i] < 0.05,
                         "min distance for any pair condition")

    def test_pair_connectivity_python(self):
        """Test pair connectivity restraint using Python exclusion volumes"""
        self._run_test_pair_connectivity(True)

    def test_pair_connectivity_c(self):
        """Test pair connectivity restraint using C++ exclusion volumes"""
        self._run_test_pair_connectivity(False)

if __name__ == '__main__':
    unittest.main()
