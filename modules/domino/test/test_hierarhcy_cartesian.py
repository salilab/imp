import sys
import unittest
import IMP.utils
import IMP.test, IMP
import IMP.domino as domino
import IMP.core
import IMP.modeller
import time
class DOMINOTests(IMP.test.TestCase):

    def __set_attributes__(self):
        self.atts=IMP.FloatKeys()
        self.atts.append(IMP.FloatKey("x"))
        self.atts.append(IMP.FloatKey("y"))
        self.atts.append(IMP.FloatKey("z"))
        self.atts.append(IMP.FloatKey("a"))
        self.atts.append(IMP.FloatKey("b"))
        self.atts.append(IMP.FloatKey("c"))
        self.atts.append(IMP.FloatKey("d"))


    #set representation and scoring
    def __set_representation__(self):
        #load three proteins
        self.particles = IMP.Particles() #the particles to be optimized
        self.h_particles = []  #thier molecular hierarhcy decorator

        for s in ['1','2','3']:
            mp = IMP.modeller.read_pdb(
                self.get_input_file_name('prot'+s+'.pdb'), self.imp_model)
            self.h_particles.append(mp)
            p = mp.get_particle()
            p.set_value(IMP.StringKey("name"),"prot"+s)
            self.particles.append(p)
            for att in self.atts[0:3]:
                p.add_attribute(att,0.0)

    def __set_restraints__(self):
        rsrs=[]
        self.opt_score = 0.0
        for i in xrange(2):
            ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
            ss= IMP.core.DistancePairScore(ub)
            r= IMP.core.ConnectivityRestraint(ss)
            ps = IMP.Particles()
            ps_refined=[]
            for j in xrange(2):
                ps_refined.append(IMP.core.hierarchy_get_leaves(self.h_particles[i+j]))
                ps.append(self.particles[i+j])
            print "ps_refined lenght is : " + str(len(ps_refined))
            for e in ps_refined:
                r.add_particles(e)
            self.imp_model.add_restraint(r)
            r.set_was_owned(True)
            self.d_opt.add_restraint(r,ps,1.0)
#             beg  = time.time()
            self.opt_score = self.opt_score + r.evaluate(None)
#             end = time.time()
#             dt    = end - beg
#             print 'connectivity restraint calculation took %9.6f Seconds' % (dt)

    def __set_optimizer__(self):
        jt_filename = self.get_input_file_name("hierarhcy_jt.txt")
        self.d_opt = IMP.domino.DominoOptimizer(jt_filename,self.imp_model)


    def __set_discrete_sampling_space__(self):
        self.m_discrete_set = domino.TransformationMappedDiscreteSet(self.particles)
        #set 4 optinal centroids for each of the particles
        for j,p in enumerate(self.particles):
            for i in xrange(3):
                new_p=IMP.Particle()
                self.imp_model.add_particle(new_p)
                if (i==j):
                    new_p.add_attribute(self.atts[0],0.0,True)
                    new_p.add_attribute(self.atts[1],0.0,True)
                    new_p.add_attribute(self.atts[2],0.0,True)
                    rt = IMP.algebra.identity_rotation()
                else:
                    new_p.add_attribute(self.atts[0],30*i+5*j,True)
                    new_p.add_attribute(self.atts[1],8*i*i+12*j,True)
                    new_p.add_attribute(self.atts[2],12*i+j*j,True)
                    rt = IMP.algebra.rotation_from_fixed_xyz( 0.3*i+0.5*j,i+j*j,i*i+1.2*j)
                abcd=rt.get_quaternion()
                new_p.add_attribute(self.atts[3],abcd[0],True)
                new_p.add_attribute(self.atts[4],abcd[1],True)
                new_p.add_attribute(self.atts[5],abcd[2],True)
                new_p.add_attribute(self.atts[6],abcd[3],True)

                self.m_discrete_set.add_state(new_p)
                self.m_discrete_set.add_mapped_state(p,new_p)
        self.sampler = domino.TransformationCartesianProductSampler(self.m_discrete_set,self.particles,True)


    def setUp(self):

        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.imp_model = IMP.Model()
        IMP.set_check_level(IMP.NONE)
        self.__set_attributes__()
        self.__set_representation__()
        self.__set_optimizer__()
        self.__set_discrete_sampling_space__()
        self.__set_restraints__()


    def test_global_min(self):
        """
        Test that the global minimum is achieved
        """
        self.d_opt.set_sampling_space(self.sampler)
        num_sol=5
        self.d_opt.set_number_of_solutions(num_sol);
        print self.d_opt.optimize(1)
        rg = self.d_opt.get_graph()
        print "OPT SCORE ::::::::::::; " + str(self.opt_score)
        scores=[self.opt_score,33.3592,66.5539,86.989,95.7]
        scores=[33.3545,33.3592,66.5539,86.989,95.7]
        for i in xrange(num_sol):
            score_inf = rg.get_opt_combination(i).get_total_score()
            self.assertAlmostEqual(score_inf,scores[i],places=1)
if __name__ == '__main__':
    unittest.main()
