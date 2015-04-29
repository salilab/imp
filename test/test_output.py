import os
import IMP
import IMP.test
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_get_particle_infos(self):
        """Test get_particle_infos_for_pdb_writing with no particles"""
        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m)
        output = IMP.pmi.output.Output()
        output.init_pdb("test_output.pdb", simo.prot)
        info, center = output.get_particle_infos_for_pdb_writing(
                                              "test_output.pdb")
        self.assertEqual(len(info), 0)
        self.assertAlmostEqual(center[0], 0., delta=1e-5)
        self.assertAlmostEqual(center[1], 0., delta=1e-5)
        self.assertAlmostEqual(center[2], 0., delta=1e-5)
        os.unlink('test_output.pdb')

if __name__ == '__main__':
    IMP.test.main()
