import IMP.kernel
import IMP.test
import IMP.atom
import IMP.container

class Tests(IMP.test.TestCase):
    def test_oriented_soap_score(self):
        """Check that orientation-dependent SOAP score is as expected"""
        m = IMP.kernel.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('soap_loop_test.pdb'),
                               m)
        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        c = IMP.container.ClosePairContainer(ps, 15.0, 0.0)
        f = IMP.atom.SameResiduePairFilter()
        c.add_pair_filter(f)

        sl = IMP.atom.OrientedSoapPairScore(
                         self.get_input_file_name('soap_loop_test.hdf5'))

        r = IMP.container.PairsRestraint(sl, c)
        m.add_restraint(r)

        score = m.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
