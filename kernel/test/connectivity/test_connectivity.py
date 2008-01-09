import unittest
import os
import IMP
import IMP.test
import IMP.utils

class ConnectivityTests(IMP.test.TestCase):
    """Class to test connectivity restraints"""

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

        p1.add_attribute(IMP.FloatKey("radius"), 1.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 1)
        p1.add_attribute(IMP.IntKey("id"), 1)

        p1 = self.particles[1]
        p1.add_attribute(IMP.FloatKey("radius"), 1.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 1)
        p1.add_attribute(IMP.IntKey("id"), 2)

        p1 = self.particles[2]
        p1.add_attribute(IMP.FloatKey("radius"), 1.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 1)
        p1.add_attribute(IMP.IntKey("id"), 3)

        p1 = self.particles[3]
        p1.add_attribute(IMP.FloatKey("radius"), 1.5, False)
        p1.add_attribute(IMP.IntKey("protein"), 2)
        p1.add_attribute(IMP.IntKey("id"), 4)

        p1 = self.particles[4]
        p1.add_attribute(IMP.FloatKey("radius"), 1.5, False)
        p1.add_attribute(IMP.IntKey("protein"), 2)
        p1.add_attribute(IMP.IntKey("id"), 5)

        p1 = self.particles[5]
        p1.add_attribute(IMP.FloatKey("radius"), 1.5, False)
        p1.add_attribute(IMP.IntKey("protein"), 2)
        p1.add_attribute(IMP.IntKey("id"), 6)

        p1 = self.particles[6]
        p1.add_attribute(IMP.FloatKey("radius"), 1.5, False)
        p1.add_attribute(IMP.IntKey("protein"), 2)
        p1.add_attribute(IMP.IntKey("id"), 7)

        p1 = self.particles[7]
        p1.add_attribute(IMP.FloatKey("radius"), 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 8)

        p1 = self.particles[8]
        p1.add_attribute(IMP.FloatKey("radius"), 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 9)

        p1 = self.particles[9]
        p1.add_attribute(IMP.FloatKey("radius"), 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 10)

        p1 = self.particles[10]
        p1.add_attribute(IMP.FloatKey("radius"), 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 11)

        p1 = self.particles[11]
        p1.add_attribute(IMP.FloatKey("radius"), 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 12)

        self.opt = IMP.ConjugateGradients()
        self.opt.set_threshold(1e-4)
        self.opt.set_model(self.imp_model)

    def test_connectivity(self):
        """Test connectivity restraint.
           All particles in a single protein should be connected, and all
           proteins should be connected, either directly or indirectly
           through other proteins."""
        self.randomize_particles(self.particles, 50.0)

        rs = IMP.RestraintSet("connect")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint(rs)

        # add connectivity restraints

        particle_indexes = IMP.Ints()
        rsrs = []

        score_func_params_ub = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                        0.0, 0.1)

        # set up exclusion volumes
        IMP.utils.set_up_exclusion_volumes(self.imp_model, self.particles,
                                           IMP.FloatKey("radius"), rsrs)

        # connect 3 proteins together
        particle_indexes.clear()
        for i in range(12):
            particle_indexes.push_back(i)
        rsrs.append(IMP.ConnectivityRestraint(self.imp_model, particle_indexes,
                                              IMP.IntKey("protein"), IMP.FloatKey("radius"),
                                              score_func_params_ub))

        # connect particles in protein1 together
        particle_indexes.clear()
        for i in range(3):
            particle_indexes.push_back(i)
        rsrs.append(IMP.ConnectivityRestraint(self.imp_model, particle_indexes,
                                              IMP.IntKey("id"), IMP.FloatKey("radius"),
                                              score_func_params_ub))

        # connect particles in protein2 together
        particle_indexes.clear()
        for i in range(3, 7):
            particle_indexes.push_back(i)
        rsrs.append(IMP.ConnectivityRestraint(self.imp_model, particle_indexes,
                                              IMP.IntKey("id"), IMP.FloatKey("radius"),
                                              score_func_params_ub))

        # connect particles in protein3 together
        particle_indexes.clear()
        for i in range(7, 12):
            particle_indexes.push_back(i)
        rsrs.append(IMP.ConnectivityRestraint(self.imp_model, particle_indexes,
                                              IMP.IntKey("id"), IMP.FloatKey("radius"),
                                              score_func_params_ub))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.randomize_particles(self.particles, 50.0)
        self.opt.optimize(55)

        # min distances
        for i in range(len(self.particles)):
            p = self.particles[i]
            icoord = (p.x(), p.y(), p.z())
            irad = p.get_value(IMP.FloatKey("radius"))
            for j in range(i+1,len(self.particles)):
                p = self.particles[j]
                jcoord = (p.x(), p.y(), p.z())
                jrad = p.get_value(IMP.FloatKey("radius"))
                self.assert_(self.check_min_distance(icoord, jcoord,
                                                     irad + jrad - 0.05),
                             "min distance for any pair condition")

        # max distances
        d12 = 10000000
        d13 = 10000000
        d23 = 10000000
        for i in range(len(self.particles)):
            p = self.particles[i]
            icoord = (p.x(), p.y(), p.z())
            irad = p.get_value(IMP.FloatKey("radius"))
            t1 = p.get_value(IMP.IntKey("protein"))
            for j in range(i+1,len(self.particles)):
                p = self.particles[j]
                jcoord = (p.x(), p.y(), p.z())
                jrad = p.get_value(IMP.FloatKey("radius"))
                t2 = p.get_value(IMP.IntKey("protein"))
                d = self.get_distance(icoord, jcoord) - irad - jrad
                if t1 == 1 and t2 == 2:
                    if d < d12:
                        d12 = d
                if t1 == 1 and t2 == 3:
                    if d < d13:
                        d13 = d
                if t1 == 2 and t2 == 3:
                    if d < d23:
                        d23 = d

        sum = 0;
        max_dist = 0.05
        if d12 < max_dist:
            sum = sum + 1
        if d13 < max_dist:
            sum = sum + 1
        if d23 < max_dist:
            sum = sum + 1
        self.assert_(sum >= 2, "min spanning tree for three particle types")

if __name__ == '__main__':
    unittest.main()
