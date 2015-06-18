import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):

    """Test the StructureSource decorator"""

    def test_get_set_source(self):
        mdl = IMP.Model()
        p = IMP.Particle(mdl)
        s = IMP.atom.StructureSource.setup_particle(p,'1ABC','G')
        self.assertEqual(s.get_source_id(),'1ABC')
        self.assertEqual(s.get_source_chain_id(),'G')

if __name__ == '__main__':
    IMP.test.main()
