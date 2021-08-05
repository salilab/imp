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

        # Our simple test library only contains interactions for CYS:C:CA,
        # CYS:C:O and GLY:C:CA. With the test PDB we get only one interaction,
        # between 1:CYS:C:CA and 3:GLY:C:CA.
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

    def test_oriented_soap_cache(self):
        """Check that OD-SOAP score cache is updated when necessary"""
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('soap_loop_test.pdb'),
                               m)

        sl = IMP.atom.OrientedSoapPairScore(
            self.get_input_file_name('soap_loop_test.hdf5'))

        # Check score between 1:CYS:C:CA and 3:GLY:C:CA
        p1 = IMP.atom.Selection(
            mh, residue_index=1,
            atom_type=IMP.atom.AT_C).get_selected_particle_indexes()[0]
        p2 = IMP.atom.Selection(
            mh, residue_index=3,
            atom_type=IMP.atom.AT_C).get_selected_particle_indexes()[0]

        r = IMP.core.PairRestraint(m, sl, (p1, p2))
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

        # Second score should be cached
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

        # Altering the hierarchy should force the cache to be rebuilt
        p1ca = IMP.atom.Selection(
            mh, residue_index=1,
            atom_type=IMP.atom.AT_CA).get_selected_particle_indexes()[0]
        p1caa = IMP.atom.Atom(m, p1ca)
        p1res = IMP.atom.get_residue(p1caa)
        p1res.remove_child(p1caa)

        score = r.evaluate(False)
        # We removed the CA atom need for the interaction, so score=0
        self.assertAlmostEqual(score, 0.0, delta=0.01)

        # Restoring the CA atom should restore the score
        p1res.add_child(p1caa)
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
