import sys
sys.path.append("./probstat_0.912/build/lib.linux-x86_64-2.5")
try:
    import AnnotationEnumeration
except ImportError:
    AnnotationEnumeration = None
import unittest
import IMP.utils
import IMP.test, IMP
import IMP.domino
import JT
from xml.dom import minidom , Node

class DOMINOTests(IMP.test.TestCase):
    """Test that the global minimum calculated by the inference is the
       right one"""
    def __load_data__(self,xml_filename):
        data={}
        r_doc = minidom.parse(xml_filename)
        for r_node in r_doc.getElementsByTagName('restraint'):
            key = str(r_node.attributes['p1'].value)+"_" \
                  +str(r_node.attributes['p2'].value)
            data[key]={}
            for s_node in r_node.getElementsByTagName('state'):
                s_key=str(s_node.attributes['s_p1'].value)+"_" \
                      +str(s_node.attributes['s_p2'].value)
                data[key][s_key]=float(s_node.attributes['val'].value)
        return data
    def __executive_search__(self):
        l=[]
        for p in self.particles:
            l.append(self.discrete_sampler.get_space_size(p))
        ae = AnnotationEnumeration.CombinationIterator(l)
        min_score = sys.maxint
        min_comb = None
        for a in ae:
            #move the model to state
            for p in self.particles:
                aa = int(self.discrete_sampler.get_state_val(p,0,
                                   self.discrete_sampler.get_attribute(p,0)))
                p.set_value(IMP.FloatKey("OPT"),aa)
                score = self.mdl.evaluate(None)

            score=0.0
            msg="for combination: " + str(a) + " : "
            for r in self.all_restraints:
                score = score+r.evaluate(None)
                msg = msg + str(r.evaluate(None)) + " , "
            print msg +" :: full score " + str(score) + "\n"
            if score < min_score:
                min_score = score
                min_comb = a
        print "MINIMUM: " + str(min_score) + " min_comb: " + str(min_comb)
    def __jt_setup(self):
        self.jt = JT.JT()
        self.jt.init_graph("simple_jt1.txt",self.mdl,self.particles)
        self.d_opt = IMP.domino.DominoOptimizer(self.mdl)
        print dir(self.d_opt)
        self.d_opt.initialize_jt_graph(self.jt.number_of_nodes())
        for i in xrange(self.jt.number_of_nodes()):
            self.d_opt.add_jt_node(i,self.jt.nodes_data[i],self.mdl)
        for e in self.jt.edges():
            self.d_opt.add_jt_edge(e[0],e[1])
    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        self.mdl = IMP.Model()
        self.particles = IMP.Particles()
        self.sampling_spaces=[]
        self.discrete_sampler = IMP.domino.SimpleDiscreteSampler()
        for i in xrange(5):
            new_p = IMP.Particle()
            self.mdl.add_particle(new_p)
            self.particles.append(new_p)
            new_p.add_attribute(IMP.StringKey("name"),str(i))
            new_p.add_attribute(IMP.FloatKey("OPT"),0)
            number_of_states=3
            if i==1 or i==3:
                number_of_states=2
            self.sampling_spaces.append(IMP.domino.SimpleDiscreteSpace(
                                                           number_of_states))
            self.discrete_sampler.add_space(new_p,self.sampling_spaces[-1])
#           print self.discrete_sampler.show()
        #add restraints
        self.all_restraints=[]
        r = self.all_restraints
        name = "simple_jt1_restraints.txt"
        r.append(IMP.domino.SimpleDiscreteRestraint(self.mdl, name,
                                          self.particles[0], self.particles[1]))
        r.append(IMP.domino.SimpleDiscreteRestraint(self.mdl, name,
                                          self.particles[0], self.particles[2]))
        r.append(IMP.domino.SimpleDiscreteRestraint(self.mdl, name,
                                          self.particles[0], self.particles[3]))
        r.append(IMP.domino.SimpleDiscreteRestraint(self.mdl, name,
                                          self.particles[1], self.particles[3]))
        r.append(IMP.domino.SimpleDiscreteRestraint(self.mdl, name,
                                          self.particles[3], self.particles[4]))
        r.append(IMP.domino.SimpleDiscreteRestraint(self.mdl, name,
                                          self.particles[2], self.particles[3]))

    def test_global_min(self):
        print "start test_global min"
        self.__executive_search__()
        #run the inference

#            self.rs = IMP.RestraintSet("simple")
#           self.mdl.add_restraint(self.rs)
        self.mdl.add_restraint(IMP.RestraintSet("simple"))
        for i in xrange(len(self.all_restraints)):
#                   self.rs.add_restraint(self.all_restraints[i])
            self.mdl.add_restraint(self.all_restraints[i])

        self.__jt_setup()
        self.d_opt.set_sampling_space(self.discrete_sampler)
        self.d_opt.show_restraint_graph()
        min_score1 = self.d_opt.optimize(1)
        min_score2 = self.mdl.evaluate(None)
        print "min_score1: " + str(min_score1) + " min_score2: " \
              + str(min_score2)
        self.assert_( min_score1 == min_score2 , "the score of the minimum configuration as calculated by the inference differs from the one calculated by the model " + str(min_score1) + " != " + str(min_score2))

if __name__ == '__main__':
    unittest.main()
