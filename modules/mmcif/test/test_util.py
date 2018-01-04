from __future__ import print_function
import IMP.test
import IMP.mmcif

class Tests(IMP.test.TestCase):
    def test_no_chains(self):
        """Trying to add a Hierarchy with no chains should give an error"""
        system = IMP.mmcif.System()
        state = IMP.mmcif.State(system)
        m = state.model
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        e = IMP.mmcif.Ensemble(state, "cluster 1")
        self.assertRaises(ValueError, e.add_model, [h], [], "model1")

if __name__ == '__main__':
    IMP.test.main()
