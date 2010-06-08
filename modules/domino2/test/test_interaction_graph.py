import sys
import unittest
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time

class DOMINOTests(IMP.test.TestCase):

    def test_global_min2(self):
        """Test that more involved graphs are fine"""
        m= IMP.Model()
        ps=[]
        for i in range(3):
            p= IMP.atom.read_pdb(self.get_input_file_name("small_protein.pdb"), m)
            ps.append(p.get_particle())
            IMP.atom.setup_as_rigid_body(p)
        cp= IMP.container.ClosePairContainer(IMP.container.ListSingletonContainer(ps), 1)
        r=IMP.container.PairsRestraint(IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(0,1)), cp);
        m.add_restraint(r)
        print "computing graph"
        g= IMP.domino2.get_interaction_graph(m, ps)
        print "done"
        vs= g.get_vertices()
        for v in vs:
            print v
            l= g.get_label(v)
            print l.get_name()
            self.assert_(l in ps)
            self.assertEqual(len(g.get_out_neighbors(v)), 2)
        g.show()


    def test_global_min(self):
        """Test that simple interaction graphs are fine"""
        m= IMP.Model()
        ps=[]
        for i in range(3):
            p = IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            ps.append(p)
        h= IMP.core.Harmonic(0,1)
        for i in range(3):
            p0= ps[i]
            p1= ps[i-1]
            r= IMP.core.DistanceRestraint(h, p0, p1)
            m.add_restraint(r)
        g= IMP.domino2.get_interaction_graph(m, ps)
        vs= g.get_vertices()
        for v in vs:
            print v
            l= g.get_label(v)
            print l.get_name()
            self.assert_(l in ps)
            self.assertEqual(len(g.get_out_neighbors(v)), 2)
        g.show()

if __name__ == '__main__':
    unittest.main()
