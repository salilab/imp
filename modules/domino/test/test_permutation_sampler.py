import sys
import unittest
import IMP.utils
import IMP.test, IMP
import IMP.domino as domino
import IMP.core
class DOMINOTests(IMP.test.TestCase):
    def __set_sampling_space__(self):
        atts=IMP.FloatKeys()
        x_key = IMP.FloatKey("x")
        y_key = IMP.FloatKey("y")
        z_key = IMP.FloatKey("z")
        #name_key = IMP.StringKey("name")
        atts.append(x_key);atts.append(y_key);atts.append(z_key)
        self.discrete_set = domino.DiscreteSet(atts)
        for i in range(4):
            new_p=IMP.Particle(self.imp_model)
            new_p.add_attribute(x_key,3*i,True)
            new_p.add_attribute(y_key,2*i,True)
            new_p.add_attribute(z_key,0.0,True)
            self.discrete_set.add_state(new_p)

    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        self.__set_sampling_space__()
        #set particles
        self.particles = IMP.Particles()
        p1 = self.create_point_particle(self.imp_model,0.0,0.0,0.0)
        p2 = self.create_point_particle(self.imp_model,0.0,0.0,0.0)
        p3 = self.create_point_particle(self.imp_model,0.0,0.0,0.0)
        p4 = self.create_point_particle(self.imp_model,0.0,0.0,0.0)
        #name_key = IMP.StringKey("name")
        p1.add_attribute(IMP.domino.node_name_key(),"0")
        p2.add_attribute(IMP.domino.node_name_key(),"1")
        p3.add_attribute(IMP.domino.node_name_key(),"2")
        p4.add_attribute(IMP.domino.node_name_key(),"3")
        self.particles.append(p1)
        self.particles.append(p2)
        self.particles.append(p3)
        self.particles.append(p4)

        #set restraints
        self.rsrs = []
        sf = IMP.core.Harmonic(1.0, 0.5)
        r1 = IMP.core.DistanceRestraint(sf, p1, p2)
        r2 = IMP.core.DistanceRestraint(sf, p2, p3)
        r3 = IMP.core.DistanceRestraint(sf, p3, p4)
        r4 = IMP.core.DistanceRestraint(sf, p1, p4)
        r1.set_was_owned(True)
        r2.set_was_owned(True)
        r3.set_was_owned(True)
        r4.set_was_owned(True)
        self.rsrs.append(r1)
        self.rsrs.append(r2)
        self.rsrs.append(r3)
        self.rsrs.append(r4)
        for r in self.rsrs:
            self.imp_model.add_restraint(r)
        self.sampler = domino.PermutationSampler(self.discrete_set,self.particles)

    def test_global_min(self):
        """
        Test that the global minimum is achived 4 times ( 4 registrations
        of an ordered ring)
        """
        jt_filename = self.get_input_file_name("permutation_test_jt.txt")
        d_opt = IMP.domino.DominoOptimizer(jt_filename,self.imp_model)
        for r in self.rsrs:
            d_opt.add_restraint(r)
        d_opt.set_sampling_space(self.sampler)
        num_sol=5
        d_opt.set_number_of_solutions(num_sol)
        print d_opt.optimize(1)
        rg = d_opt = d_opt.get_graph()
        scores=[6.7889,6.7889,6.7889,6.7889,22.6833]
        for i in range(num_sol):
            score_inf = rg.get_opt_combination(i).get_total_score()
            self.assertAlmostEqual(score_inf,scores[i],places=1)
if __name__ == '__main__':
    unittest.main()
