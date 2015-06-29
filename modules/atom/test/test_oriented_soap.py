import IMP.test
import IMP.atom
import IMP.container


class Tests(IMP.test.TestCase):

    def test_oriented_soap_score(self):
        """Check that orientation-dependent SOAP score is as expected"""
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('soap_loop_test.pdb'),
                               m)
        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        c = IMP.container.ClosePairContainer(ps, 15.0, 0.0)

        sl = IMP.atom.OrientedSoapPairScore(
            self.get_input_file_name('soap_loop_test.hdf5'))
        c.add_pair_filter(sl.get_pair_filter())

        r = IMP.container.PairsRestraint(sl, c)

        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
