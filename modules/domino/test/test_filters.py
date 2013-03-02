import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.container
import IMP.algebra
import random

def cross(args):
    ans = [[]]
    for arg in args:
        ans = [x+[y] for x in ans for y in arg]
    return ans

class Tests(IMP.test.TestCase):
    def _get_filtered(self, f, n):
        states= [[0,1,2,3,4,5] for x in range(n)]
        all= cross(states)
        ret=[]
        for s in all:
            if f.get_is_ok(IMP.domino.Assignment(s)):
                ret.append(s)
        return ret
    def test_global_min2(self):
        """Test exclusion filtering pairs"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ss= IMP.domino.Subset(ps)
        print ps, ss
        ft= IMP.domino.ExclusionSubsetFilterTable()
        pairs=[(ps[0], ps[1]), (ps[3], ps[4])]
        for pr in pairs:
            ft.add_pair(pr)
        psp=[x for x in ps]
        random.shuffle(psp)
        css= IMP.domino.Subset(psp[0:5])
        print css
        f= ft.get_subset_filter(css, [])
        print f, css
        if not f:
            return
        filtered= self._get_filtered(f, 5)
        for s in filtered:
            for i, p0 in enumerate(css):
                for j, p1 in enumerate(css):
                    if i==j:
                        continue
                    if (p0, p1) in pairs or (p1, p0) in pairs:
                        self.assertNotEqual(s[i], s[j])
    def test_global_min3(self):
        """Test exclusion filtering all"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ss= IMP.domino.Subset(ps)
        print [x.get_name() for x in ps], ss
        ft= IMP.domino.ExclusionSubsetFilterTable()
        ft.add_set(ps)
        psp=[x for x in ps]
        random.shuffle(psp)
        css= IMP.domino.Subset(psp[0:5])
        print "selected", css
        f= ft.get_subset_filter(css, [])
        filtered= self._get_filtered(f, 5)
        print "filtered", filtered
        for s in filtered:
            print s
            for i, p0 in enumerate(css):
                for j, p1 in enumerate(css):
                    if i==j:
                        continue
                    self.assertNotEqual(s[i], s[j])
    def test_global_min4(self):
        """Test equivalence filtering"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ss= IMP.domino.Subset(ps)
        print [x.get_name() for x in ps], ss
        ft= IMP.domino.EquivalenceSubsetFilterTable()
        ft.add_set(ps)
        psp=[x for x in ps]
        random.shuffle(psp)
        css= IMP.domino.Subset(psp[0:5])
        print "selected", css
        f= ft.get_subset_filter(css, [])
        filtered= self._get_filtered(f, 5)
        seen=[]
        for s in filtered:
            cur= [x for x in s]
            cur.sort()
            print s, cur
            self.assertNotIn(cur, seen)
            seen.append(cur)


    def test_min_filter(self):
        """Test minimum filtering"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,3)]
        for i in range(0,3):
            IMP.core.XYZR.setup_particle(ps[i],IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),2))
        lsc1=IMP.container.ListSingletonContainer(m)
        lsc1.add_particles([ps[0]]);
        lsc2=IMP.container.ListSingletonContainer(m)
        lsc2.add_particles([ps[1], ps[2]])
        nbl=IMP.container.CloseBipartitePairContainer(lsc1,lsc2,2)
        h=IMP.core.HarmonicLowerBound(0,1)
        sd=IMP.core.SphereDistancePairScore(h)
        pr=IMP.container.PairsRestraint(sd,nbl)
        max_score=.9
        pr.set_maximum_score(max_score)
        pr.set_model(m)
        prd=pr.create_decomposition()
        IMP.show_restraint_hierarchy(prd)
        print prd.get_input_particles()
        rs=IMP.RestraintSet.get_from(prd)
        #create particles state table
        pst=IMP.domino.ParticleStatesTable()
        states=IMP.domino.XYZStates([IMP.algebra.Vector3D(i,i,i) for i in range(3)])
        for p in ps:
            pst.set_particle_states(p,states)
        max_violations=0
        rc= IMP.domino.RestraintCache(pst)
        rc.add_restraints([prd])
        ft= IMP.domino.MinimumRestraintScoreSubsetFilterTable(rs.get_restraints(),
                                                              rc,max_violations)
        samp=IMP.domino.DominoSampler(m,pst)
        samp.set_subset_filter_tables([ft])
        cs=samp.get_sample()
        #print "number of solutions",cs.get_number_of_configurations()
        ok_combs=0
        for i in range(cs.get_number_of_configurations()):
            cs.load_configuration(i)
            #print IMP.core.XYZ(ps[0]),IMP.core.XYZ(ps[1]),pr.evaluate(None)
            if pr.evaluate(None)<=max_score:
                ok_combs=ok_combs+1
        self.assertEqual(cs.get_number_of_configurations(),ok_combs)
if __name__ == '__main__':
    IMP.test.main()
