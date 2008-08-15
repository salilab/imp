import sys
sys.path.append("./probstat_0.912/build/lib.linux-x86_64-2.5")
import AnnotationEnumeration
import DOMINO
import JT
import IMP

class my_optimizer:
    def __init__(self,jt_filename, restraints_filename,num_of_particles):
        self.mdl = IMP.Model()
        self.particles = IMP.Particles()
        self.sampling_spaces=[]
        self.all_restraints=[]
        for i in xrange(num_of_particles):
            new_p = IMP.Particle()
            self.mdl.add_particle(new_p)
            self.particles.append(new_p)
            new_p.add_attribute(IMP.StringKey("name"),str(i))
            new_p.add_attribute(IMP.FloatKey("OPT"),0)
        self.__jt_setup(jt_filename)
        self.init_sampling_space()
        self.init_restraints(restraints_filename)

    def exhaustive_search(self):
        l=[] #number of states of each component
        for p in self.particles:
            l.append(self.discrete_sampler.get_space_size(p))
        ae = AnnotationEnumeration.CombinationIterator(l)
        min_score = sys.maxint
        min_comb = None
        for a in ae:
            #move the model to state
            for i,p in enumerate(self.particles):
                opt_val = int(self.discrete_sampler.get_state_val(
                               p,a[i],self.discrete_sampler.get_attribute(p,0)))
                p.set_value(IMP.FloatKey("OPT"),opt_val)
            score = self.mdl.evaluate(None)
            if score < min_score:
                min_score = score
                min_comb = a
        print "MINIMUM: " + str(min_score) + " min_comb: " + str(min_comb)
        return min_score
    def __jt_setup(self, jt_filename):
        self.jt = JT.JT()
        self.jt.init_graph(jt_filename,self.mdl,self.particles)
        self.d_opt = DOMINO.DominoOptimizer(self.mdl)
        self.d_opt.initialize_jt_graph(self.jt.number_of_nodes())
        for i in xrange(self.jt.number_of_nodes()):
            self.d_opt.add_jt_node(i,self.jt.nodes_data[i],self.mdl)
        for e in self.jt.edges():
            self.d_opt.add_jt_edge(e[0],e[1])

    def init_restraints(self,restraints_filename):
        self.all_restraints.append(IMP.RestraintSet("simple"))
        self.mdl.add_restraint(self.all_restraints[0])
        pair_restraints=[]
        for line in open(restraints_filename):
            s=line.split("|")
            if len(s) ==2:
                pair_restraints.append([int(s[0]),int(s[1])])
        for i,j in pair_restraints:
            #add restraints
            self.all_restraints.append(DOMINO.SimpleDiscreteRestraint(
                                     self.mdl,restraints_filename,
                                     self.particles[i],self.particles[j]))
            self.mdl.add_restraint(self.all_restraints[-1])
            self.all_restraints[-1]

    def optimize(self):
        self.d_opt.set_sampling_space(self.discrete_sampler)
        print "======4"
#      self.d_opt.show_restraint_graph()
        return self.d_opt.optimize(1)


    def init_sampling_space(self):
        self.discrete_sampler = DOMINO.SimpleDiscreteSampler()
        print "number of particles: " + str(len(self.particles))
        for i in xrange(len(self.particles)):
            number_of_states=3
            if i==1 or i==3:
                number_of_states=2
            self.sampling_spaces.append(DOMINO.SimpleDiscreteSpace(
                                                             number_of_states))
            self.discrete_sampler.add_space(self.particles[i],
                                            self.sampling_spaces[-1])
