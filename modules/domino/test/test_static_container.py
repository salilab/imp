import IMP
import IMP.test
import IMP.domino
import IMP.core


class Tests(IMP.test.TestCase):
    def _test_global_min2(self):
        """Testing static container"""
        m= IMP.Model()
        ps=[]
        for i in range(0, 20):
            p= IMP.Particle(m)
            IMP.core.XYZ.setup_particle(p)
            ps.append(p)
        lsc= IMP.container.ListSingletonContainer(ps)
        cpc= IMP.container.ClosePairContainer(lsc, 5)
        dg= IMP.get_dependency_graph([m.get_root_restraint_set()])
        self.assertTrue(IMP.domino.get_is_static_container(lsc, dg, ps))
        self.assertFalse(IMP.domino.get_is_static_container(cpc, dg, ps))


if __name__ == '__main__':
    IMP.test.main()
