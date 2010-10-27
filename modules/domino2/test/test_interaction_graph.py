import sys
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.display
import IMP.restrainer
import time

class NullStates(IMP.domino2.ParticleStates):
    def __init__(self):
        IMP.domino2.ParticleStates.__init__(self)
    def get_number_of_particle_states(self):
        return 1
class DOMINOTests(IMP.test.TestCase):
    def test_global_min2(self):
        """Test that more involved graphs are fine"""
        m= IMP.Model()
        ps=[]
        for i in range(3):
            p= IMP.atom.read_pdb(self.get_input_file_name("small_protein.pdb"), m)
            p.set_name("protein"+str(i))
            ps.append(p.get_particle())
            IMP.atom.setup_as_rigid_body(p)
        cp= IMP.container.ClosePairContainer(IMP.container.ListSingletonContainer(ps), 1, 0)
        r=IMP.container.PairsRestraint(IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(0,1)), cp);
        m.add_restraint(r)
        print "computing graph"
        pst= IMP.domino2.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, NullStates())
        g= IMP.domino2.get_interaction_graph(m.get_root_restraint_set(),
                                             pst)
        w= IMP.display.PymolWriter(self.get_tmp_file_name("ig0.pym"))
        for gg in IMP.domino2.get_interaction_graph_geometry(g):
            w.add_geometry(gg)
        del w
        print "done"
        vs= g.get_vertices()
        for v in vs:
            print v
            l= g.get_vertex_name(v)
            print l.get_name()
            self.assertIn(l, ps)
            self.assertEqual(len(g.get_out_neighbors(v)), 2)
        g.show()
    def test_global_min3(self):
        """Test that showing interaction graphs is fine"""
        m= IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        ps=[]
        p= IMP.atom.read_pdb(self.get_input_file_name("small_protein.pdb"), m)
        IMP.atom.add_radii(p)
        p.show()
        #print "radius is ", IMP.core.XYZR(IMP.atom.get_leaves(p)[0]).get_radius()
        #exit(1)
        #sp= IMP.atom.get_simplified_by_residue(p, 1)
        cpf= IMP.core.QuadraticClosePairsFinder()
        cpf.set_distance(0.0)
        print len(IMP.atom.get_leaves(p)), "leaves"
        lsc= IMP.container.ListSingletonContainer(IMP.atom.get_leaves(p))
        cp= cpf.get_close_pairs(lsc)
        for pr in cp:
            r=IMP.core.PairRestraint(IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(0,1)), pr);
            m.add_restraint(r)
            r.set_name("pair")
        print "computing graph"
        pst= IMP.domino2.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, NullStates())
        g= IMP.domino2.get_interaction_graph(m.get_root_restraint_set(),
                                             pst)
        w= IMP.display.PymolWriter(self.get_tmp_file_name("ig-large.pym"))
        gs=IMP.domino2.get_interaction_graph_geometry(g)
        print "There are ", len(gs)
        for gg in gs:
            w.add_geometry(gg)
        del w


    def test_global_min(self):
        """Test that simple interaction graphs are fine"""
        m= IMP.Model()
        ps=[]
        IMP.set_log_level(IMP.VERBOSE)
        m.set_log_level(IMP.SILENT)
        for i in range(3):
            p = IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.Vector3D(0,i,0))
            ps.append(p)
        h= IMP.core.Harmonic(0,1)
        for i in range(3):
            p0= ps[i]
            p1= ps[i-1]
            r= IMP.core.DistanceRestraint(h, p0, p1)
            m.add_restraint(r)
        pst= IMP.domino2.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, NullStates())
        g= IMP.domino2.get_interaction_graph(m.get_root_restraint_set(),
                                             pst)
        w= IMP.display.PymolWriter(self.get_tmp_file_name("ig0.pym"))
        for gg in IMP.domino2.get_interaction_graph_geometry(g):
            w.add_geometry(gg)
        del w
        print g
        vs= g.get_vertices()
        for v in vs:
            print v
            l= g.get_vertex_name(v)
            print l.get_name()
            self.assertIn(l, ps)
            self.assertEqual(len(g.get_out_neighbors(v)), 2)
        g.show()

if __name__ == '__main__':
    IMP.test.main()
