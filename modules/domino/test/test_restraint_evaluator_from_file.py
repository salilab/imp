import sys
import unittest
import IMP
import IMP.test
import my_optimizer

class DOMINOTests(IMP.test.TestCase):
    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.NONE)
        re=IMP.domino.RestraintEvaluatorFromFile()
        mdl=IMP.Model()
        #initialize particles and sampling spaces
        ps=IMP.Particles()
        discrete_sampler = IMP.domino.SimpleDiscreteSampler()
        sampling_spaces=[]
        for i in range(8):
            ps.append(self.create_point_particle(mdl,0.0, 0.0, 0.0))
            ps[-1].add_attribute(IMP.domino.node_name_key(),str(i))
            ps[-1].add_attribute(IMP.FloatKey("OPT"),0,False)
            number_of_states=3
            if i==1 or i==3:
                number_of_states=2
            sampling_spaces.append(IMP.domino.SimpleDiscreteSpace(number_of_states))
            discrete_sampler.add_space(ps[-1],
                                       sampling_spaces[-1])
        #set optimizer
        jt = IMP.domino.JunctionTree()
        jt_filename=self.get_input_file_name("simple_jt2.txt")
        IMP.domino.read_junction_tree(jt_filename,jt)
        d_opt = IMP.domino.DominoOptimizer(jt,mdl,re)
        d_opt.set_sampling_space(discrete_sampler)
        #add restraints to mdl and optimizer
        pairs=[[0,1],[1,4],[2,6],[2,7],[3,4],[4,5],[4,6],[6,7]]
        rs=IMP.Restraints()
        for p in pairs:
            p1=ps[p[0]]
            p2=ps[p[1]]
            r=IMP.core.DistanceRestraint(IMP.core.Harmonic(0., 1.),p1,p2)
            rs.append(r)
            mdl.add_restraint(r)
            fn=self.get_input_file_name("comb_"+str(p[0])+"_"+str(p[1])+".txt")
            re.set_restraint_file(r,fn)
            ps1=IMP.Particles()
            ps1.append(p1)
            ps1.append(p2)
            d_opt.add_restraint(r,ps1)

        self.infered_score = d_opt.optimize(1)
        min_combination = d_opt.get_graph().get_opt_combination(0)
        self.exhaustive_search()


    def exhaustive_search(self):
        pairs=[[0,1,{}],[1,4,{}],[2,6,{}],[2,7,{}],[3,4,{}],[4,5,{}],[4,6,{}],[6,7,{}]]
        for j,p in enumerate(pairs):
            fn=self.get_input_file_name("comb_"+str(p[0])+"_"+str(p[1])+".txt")
            f=open(fn)
            lines=f.readlines()
            f.close()
            for i in range(1,len(lines)):
                s=lines[i].split("|")
                pairs[j][2][str(int(s[0]))+"_"+str(int(s[1]))]=float(s[2])
        min_score = sys.maxint
        min_comb = None
        for i0 in range(3):
            for i1 in range(2):
                for i2 in range(3):
                    for i3 in range(2):
                        for i4 in range(3):
                            for i5 in range(3):
                                for i6 in range(3):
                                    for i7 in range(3):
                                        sum=pairs[0][2][str(i0)+"_"+str(i1)]+\
                                            pairs[1][2][str(i1)+"_"+str(i4)]+\
                                            pairs[2][2][str(i2)+"_"+str(i6)]+\
                                            pairs[3][2][str(i2)+"_"+str(i7)]+\
                                            pairs[4][2][str(i3)+"_"+str(i4)]+\
                                            pairs[5][2][str(i4)+"_"+str(i5)]+\
                                            pairs[6][2][str(i4)+"_"+str(i6)]+\
                                            pairs[7][2][str(i6)+"_"+str(i7)]
                                        if sum<min_score:
                                            min_score=sum
                                            min_comb=[i0,i1,i2,i3,i4,i5,i6,i7]
        self.infered_score1 = min_score

    def test_inference_1(self):
        print "infered_score1: " + str(self.infered_score1)
        print "infered_score: " + str(self.infered_score)
        self.assert_( abs(self.infered_score - self.infered_score1) < 0.001 , "the score of the minimum configuration as calculated by the inference is wrong " + str(self.infered_score) + " != " + str(self.infered_score1))


if __name__ == '__main__':
    unittest.main()
