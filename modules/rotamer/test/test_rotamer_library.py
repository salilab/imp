import IMP
import IMP.test
import IMP.atom
import IMP.rotamer

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.rl = IMP.rotamer.RotamerLibrary()
        self.rl.read_library_file(self.get_input_file_name('sample.rotamers.lib'))

    def test_query1(self):
        # this corresponds to the first lines in the file
        # ARG  -180 -180    10     1  2  2  1  0.249730    62.5   176.9   176.6    85.7
        # ARG  -180 -180    10     1  2  2  2  0.154574    64.7  -177.9  -179.4   179.0
        # ARG  -180 -180    10     1  2  2  3  0.116822    63.5  -174.8  -175.6   -86.2
        # get_rotamers() takes inputs of residue type, phi angle, psi angle, sum of probability
        v = self.rl.get_rotamers(IMP.atom.ResidueType("ARG"), -180, -180, 0.5)
        # the number of elements that sum of probability to at least 0.5: 3
        self.assertEqual(len(v), 3)
        self.assertAlmostEqual(v[0].get_chi1(), 62.5, delta=1e-4)
        self.assertAlmostEqual(v[0].get_chi2(), 176.9, delta=1e-4)
        self.assertAlmostEqual(v[0].get_chi3(), 176.6, delta=1e-4)
        self.assertAlmostEqual(v[0].get_chi4(), 85.7, delta=1e-4)
        self.assertAlmostEqual(v[0].get_probability(), 0.249730, delta=1e-4)

        self.assertAlmostEqual(v[1].get_chi1(), 64.7, delta=1e-4)
        self.assertAlmostEqual(v[1].get_chi2(), -177.9, delta=1e-4)
        self.assertAlmostEqual(v[1].get_chi3(), -179.4, delta=1e-4)
        self.assertAlmostEqual(v[1].get_chi4(), 179.0, delta=1e-4)
        self.assertAlmostEqual(v[1].get_probability(), 0.154574, delta=1e-4)

    def test_query2(self):
        # this corresponds to
        # VAL   180   90     0     2  0  0  0  0.550887   175.8     0.0     0.0     0.0
        v = self.rl.get_rotamers(IMP.atom.ResidueType("VAL"), 180, 90, 0.5)
        self.assertEqual(len(v), 1)
        self.assertAlmostEqual(v[0].get_chi1(), 175.8, delta=1e-4)
        self.assertAlmostEqual(v[0].get_probability(), 0.550887, delta=1e-4)

if __name__ == '__main__':
    IMP.test.main()
