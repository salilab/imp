import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.container
import IMP.display


class NullStates(IMP.domino.ParticleStates):

    def __init__(self):
        IMP.domino.ParticleStates.__init__(self)

    def get_number_of_particle_states(self):
        return 1


class Tests(IMP.test.TestCase):

    def test_global_min2(self):
        """Test that more involved graphs are fine"""
        m = IMP.Model()
        aps = []
        rbs = []
        for i in range(2):
            ps = IMP._create_particles_from_pdb(
                self.get_input_file_name("small_protein.pdb"),
                m)
            p = IMP.Particle(m)
            p.set_name("protein" + str(i))
            aps.extend(ps)
            rb = IMP.core.RigidBody.setup_particle(p, ps)
            rbs.append(rb)
        cp = IMP.container.ClosePairContainer(
            IMP.container.ListSingletonContainer(m, aps),
            1,
            0)
        r = IMP.container.PairsRestraint(
            IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(0, 1)), cp)
        rs = IMP.RestraintSet(m)
        rs.add_restraint(r)
        print("computing graph")
        pst = IMP.domino.ParticleStatesTable()
        for p in rbs:
            pst.set_particle_states(p, NullStates())
        g = IMP.domino.get_interaction_graph([rs], pst)
        #w = IMP.display.PymolWriter(self.get_tmp_file_name("ig0.pym"))
        # for gg in IMP.domino.get_interaction_graph_geometry(g):
        #    w.add_geometry(gg)
        #del w
        print("done")
        vs = g.get_vertices()
        # IMP.show_graphviz(g)
        for v in vs:
            print(v)
            l = g.get_vertex_name(v)
            print(l.get_name())
            self.assertIn(l, rbs)
            self.assertEqual(len(g.get_out_neighbors(v)), 1)
            # g.show()

    def test_global_min(self):
        """Test that simple interaction graphs are fine"""
        m = IMP.Model()
        ps = []
        IMP.set_log_level(IMP.SILENT)
        m.set_log_level(IMP.SILENT)
        for i in range(3):
            p = IMP.Particle(m)
            d = IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.Vector3D(0, i, 0))
            ps.append(p)
        h = IMP.core.Harmonic(0, 1)
        rs = IMP.RestraintSet(m)
        for i in range(3):
            p0 = ps[i]
            p1 = ps[i - 1]
            r = IMP.core.DistanceRestraint(m, h, p0, p1)
            rs.add_restraint(r)
        pst = IMP.domino.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, NullStates())
        g = IMP.domino.get_interaction_graph([rs], pst)
        #w = IMP.display.PymolWriter(self.get_tmp_file_name("ig0.pym"))
        # for gg in IMP.domino.get_interaction_graph_geometry(g):
        #    w.add_geometry(gg)
        #del w
        print(g)
        vs = g.get_vertices()
        # IMP.show_graphviz(g)
        for v in vs:
            print(v)
            l = g.get_vertex_name(v)
            print(l.get_name())
            self.assertIn(l, ps)
            self.assertEqual(len(g.get_out_neighbors(v)), 2)
            # g.show()

if __name__ == '__main__':
    IMP.test.main()
