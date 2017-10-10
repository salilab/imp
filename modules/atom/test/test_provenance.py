from __future__ import print_function
import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):

    def test_provenance(self):
        """Test Provenance decorator"""
        m = IMP.Model()
        p = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        self.assertTrue(IMP.atom.Provenance.get_is_setup(p))
        self.assertFalse(p.get_previous_state())

        p2 = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        p.set_previous_state(p2)
        self.assertEqual(p.get_previous_state(), p2)

    def test_structure_provenance(self):
        """Test StructureProvenance decorator"""
        m = IMP.Model()
        p = IMP.atom.StructureProvenance.setup_particle(m, IMP.Particle(m),
                               "testfile", "testchain")
        self.assertTrue(IMP.atom.StructureProvenance.get_is_setup(p))
        self.assertEqual(p.get_filename(), "testfile")
        self.assertEqual(p.get_chain_id(), "testchain")

    def test_provenanced(self):
        """Test Provenanced decorator"""
        m = IMP.Model()
        p = IMP.atom.Provenance.setup_particle(m, IMP.Particle(m))
        pd = IMP.atom.Provenanced.setup_particle(m, IMP.Particle(m), p)
        self.assertEqual(pd.get_provenance(), p)
        pd.set_provenance(p)
        self.assertTrue(IMP.atom.Provenanced.get_is_setup(pd))
        self.assertFalse(IMP.atom.Provenanced.get_is_setup(p))
        self.assertFalse(IMP.atom.Provenance.get_is_setup(pd))

if __name__ == '__main__':
    IMP.test.main()
