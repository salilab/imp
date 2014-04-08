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

    def test_list_ig(self):
        """Test that interaction graphs of lists decompose"""
        m = IMP.kernel.Model()
        ps = [IMP.kernel.Particle(m) for i in range(0, 3)]
        score = IMP.core.SoftSpherePairScore(1)
        r = IMP.container.PairsRestraint(
            score, [(ps[0], ps[1]), (ps[1], ps[2])])
        for p in ps:
            IMP.core.XYZR.setup_particle(p)
        ig = IMP.domino.get_interaction_graph([r], ps)
        self.assertEqual(len(ig.get_vertices()), 3)
        self.assertEqual(len(ig.get_in_neighbors(0)), 1)

    def test_global_min3(self):
        """Test that showing interaction graphs is fine"""
        m = IMP.kernel.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        ps = IMP.kernel._create_particles_from_pdb(
            self.get_input_file_name("small_protein.pdb"),
            m)
        # print "radius is ", IMP.core.XYZR(IMP.atom.get_leaves(p)[0]).get_radius()
        # exit(1)
        #sp= IMP.atom.get_simplified_by_residue(p, 1)
        cpf = IMP.core.QuadraticClosePairsFinder()
        cpf.set_distance(0.0)
        print len(ps), "leaves"
        cp = cpf.get_close_pairs(ps)
        for pr in cp:
            r = IMP.core.PairRestraint(
                IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(0, 1)), pr)
            m.add_restraint(r)
            r.set_name("pair")
        print "computing graph"
        pst = IMP.domino.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, NullStates())
        g = IMP.domino.get_interaction_graph([m.get_root_restraint_set()],
                                             pst)
        #w = IMP.display.PymolWriter(self.get_tmp_file_name("ig-large.pym"))
        gs = IMP.domino.get_interaction_graph_geometry(g)
        print "There are ", len(gs)
        # for gg in gs:
        #    w.add_geometry(gg)
        #del w

if __name__ == '__main__':
    IMP.test.main()
