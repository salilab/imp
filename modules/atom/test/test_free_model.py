import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Check that deleting the model doesn't make things crash """
        if IMP.build != "debug":
            self.skipTest("Only tested in debug mode")
        def init_linear():
            topology = IMP.atom.CHARMMTopology(IMP.atom.get_all_atom_CHARMM_parameters())
            topology.add_sequence('IACGACKPECPVNIIQGS')
            topology.apply_default_patches()

            m = IMP.Model()
            h = topology.create_hierarchy(m)

            topology.add_atom_types(h)
            topology.add_coordinates(h)

            # Write out the final structure to a PDB file
            IMP.set_log_level(IMP.MEMORY)
            return  h
        h = init_linear()
        self.assertRaises(IMP.base.InternalException, h.show)

if __name__ == '__main__':
    IMP.test.main()
