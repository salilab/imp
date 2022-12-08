from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d as em2d
import pickle


class WriteStatisticsOptimizerScore(IMP.OptimizerState):

    """An Optimizer score to get the values of the statistics after a
       given set of evaluations"""

    def __init__(self, m):
        IMP.OptimizerState.__init__(self, m, "WriteStats")
        self.count = 0

    def update(self):
        if (self.count != 10):
            self.count += 1
            return
        else:
            self.count = 0
        o = self.get_optimizer()
        m = o.get_model()
        m.show_restraint_score_statistics()
        m.show_all_statistics()
        # for i in range(0,m.get_number_of_restraints()):
        #    r=m.get_restraint(i)
        #    print "restraint",r.get_name(),"value",r.evaluate(False)

    def do_show(self, stream):
        pass


class Tests(IMP.test.TestCase):

    def test_building_an_optimization_problem_with_em2d_restraint(self):
        """Test that an a optimization with em2d restraint is properly built"""
        m = IMP.Model()
        prot = IMP.atom.read_pdb(self.get_input_file_name("1z5s.pdb"),
                                 m, IMP.atom.ATOMPDBSelector())
        # get the chains
        chains = IMP.atom.get_by_type(prot, IMP.atom.CHAIN_TYPE)
        # set the chains as rigid bodies
        rigid_bodies = []
        native_chain_centers = []
        for c in chains:
            atoms = IMP.core.get_leaves(c)
            rbd = IMP.core.RigidBody.setup_particle(c, atoms)
            rbd.set_coordinates_are_optimized(True)
            rigid_bodies.append(rbd)
            native_chain_centers.append(rbd.get_coordinates())
        self.assertEqual(
            len(rigid_bodies),
            4,
            "Problem generating rigid bodies")

        # set distance restraints
        d01 = IMP.algebra.get_distance(native_chain_centers[0],
                                       native_chain_centers[1])
        r01 = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(d01, 1),
                                         chains[0], chains[1])
        r01.set_name("distance 0-1")
        d12 = IMP.algebra.get_distance(native_chain_centers[1],
                                       native_chain_centers[2])
        r12 = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(d12, 1),
                                         chains[1], chains[2])
        r12.set_name("distance 1-2")
        d23 = IMP.algebra.get_distance(native_chain_centers[2],
                                       native_chain_centers[3])
        r23 = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(d23, 1),
                                         chains[2], chains[3])
        r23.set_name("distance 2-3")
        d30 = IMP.algebra.get_distance(native_chain_centers[3],
                                       native_chain_centers[0])
        r30 = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(d30, 1),
                                         chains[3], chains[0])
        r30.set_name("distance 3-0")
        top_rs = IMP.RestraintSet(m)
        # set distance restraints
        for r in [r01, r12, r23, r30]:
            top_rs.add_restraint(r)
        self.assertEqual(top_rs.get_number_of_restraints(), 4,
                         "Incorrect number of distance restraints")
        # set em2D restraint
        srw = em2d.SpiderImageReaderWriter()
        selection_file = self.get_input_file_name("all-1z5s-projections.sel")
        images_to_read_names = [
            IMP.get_relative_path(selection_file, x)
            for x in em2d.read_selection_file(selection_file)]
        em_images = em2d.read_images(images_to_read_names, srw)

        self.assertEqual(len(em_images), 3, "Incorrect number images read")

        apix = 1.5
        resolution = 1
        n_projections = 20
        params = em2d.Em2DRestraintParameters(apix, resolution, n_projections)
        params.save_match_images = False
        params.coarse_registration_method = em2d.ALIGN2D_PREPROCESSING
        score_function = em2d.EM2DScore()

        em2d_restraint = em2d.Em2DRestraint(m)
        em2d_restraint.setup(score_function, params)
        em2d_restraint.set_images(em_images)
        em2d_restraint.set_name("em2d restraint")
        container = IMP.container.ListSingletonContainer(
            m, IMP.core.get_leaves(prot))
        em2d_restraint.set_particles(container)
        em2d_restraints_set = IMP.RestraintSet(m)
        em2d_restraints_set.add_restraint(em2d_restraint)
        em2d_restraints_set.set_weight(1000)  # weight for the em2D restraint
        top_rs.add_restraint(em2d_restraints_set)
        self.assertEqual(top_rs.get_number_of_restraints(), 5,
                         "Incorrect number of restraints")
        # MONTECARLO OPTIMIZATION
        s = IMP.core.MonteCarlo(m)
        s.set_scoring_function(top_rs)
        # Add movers for the rigid bodies
        movers = []
        for rbd in rigid_bodies:
            movers.append(IMP.core.RigidBodyMover(m, rbd, 5, 2))
        s.add_movers(movers)
        self.assertEqual(s.get_number_of_movers(), 4,
                         "Incorrect number of MonteCarlo movers")

        # Optimizer state to save intermediate configurations
        o_state = IMP.atom.WritePDBOptimizerState(chains,
                                                  "intermediate-step-%1%.pdb")
        o_state.set_period(11)
        s.add_optimizer_state(o_state)

        ostate2 = WriteStatisticsOptimizerScore(m)
        s.add_optimizer_state(ostate2)

    def test_pickle_restraint_parameters(self):
        """Test (un-)pickle of Em2DRestraintParameters"""
        p = em2d.Em2DRestraintParameters(1.5, 1, 20)
        p.save_match_images = False
        p.coarse_registration_method = em2d.ALIGN2D_PREPROCESSING
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)
        self.assertEqual(newp.n_projections, 20)
        self.assertFalse(newp.save_match_images)
        self.assertEqual(newp.coarse_registration_method,
                         em2d.ALIGN2D_PREPROCESSING)


if __name__ == '__main__':
    IMP.test.main()
