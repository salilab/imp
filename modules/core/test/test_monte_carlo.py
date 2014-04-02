import IMP
import IMP.test
import IMP.core
import IMP.container

class Tests(IMP.test.TestCase):

    def test_stats(self):
        """Test MonteCarlo stats"""
        m = IMP.kernel.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        mc = IMP.core.MonteCarlo(m)
        mc.set_log_level(IMP.base.SILENT)
        ps = []
        bb = IMP.algebra.get_unit_bounding_box_3d()
        for i in range(0, 10):
            p = IMP.kernel.Particle(m)
            d = IMP.core.XYZR.setup_particle(p)
            ps.append(d)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_radius(.1)
            d.set_coordinates_are_optimized(True)
        cpc = IMP.container.ConsecutivePairContainer(ps)
        hps = IMP.core.HarmonicDistancePairScore(1, 100)
        r = IMP.container.PairsRestraint(hps, cpc)
        m.add_restraint(r)
        ms = [IMP.core.BallMover([x], .1) for x in ps]
        mv = IMP.core.SerialMover(ms)
        mc.add_mover(mv)
        self.assertEqual(mc.get_number_of_accepted_steps(), 0)
        self.assertEqual(mc.get_number_of_upward_steps(), 0)
        self.assertEqual(mc.get_number_of_downward_steps(), 0)
        self.assertEqual(mc.get_number_of_proposed_steps(), 0)
        for i in range(0, 10):
            mc.optimize(100)
            self.assertEqual(mc.get_number_of_accepted_steps(),
                             mc.get_number_of_upward_steps() + 
                             mc.get_number_of_downward_steps())
            self.assertEqual(mc.get_number_of_proposed_steps(), 100)

if __name__ == '__main__':
    IMP.test.main()
