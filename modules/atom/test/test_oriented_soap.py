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

    def test_oriented_soap_cache_hierarchy(self):
        """Check that OD-SOAP score cache is updated when Hierarchy changes"""
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('soap_loop_test.pdb'),
                               m)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        c = IMP.container.ClosePairContainer(ps, 15.0, 0.0)

        sl = IMP.atom.OrientedSoapPairScore(
            self.get_input_file_name('soap_loop_test.hdf5'))

        r = IMP.container.PairsRestraint(sl, c)

        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

        # Second score should be cached
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

        # Altering the hierarchy should force the cache to be rebuilt
        residues = [IMP.atom.Residue(r)
                    for r in IMP.atom.get_by_type(mh, IMP.atom.RESIDUE_TYPE)]
        p1ca = IMP.atom.Selection(
            mh, residue_index=1,
            atom_type=IMP.atom.AT_CA).get_selected_particle_indexes()[0]
        p1caa = IMP.atom.Atom(m, p1ca)
        residues[0].remove_child(p1caa)
        residues[1].add_child(p1caa)

        score = r.evaluate(False)
        # We removed the CA atom need for the interaction, so score=0
        self.assertAlmostEqual(score, 0.0, delta=0.01)

        # Restoring the CA atom should restore the score
        residues[1].remove_child(p1caa)
        residues[0].add_child(p1caa)
        score = r.evaluate(False)
        self.assertAlmostEqual(score, 1.0, delta=0.01)

    def test_oriented_soap_cache_model(self):
        """Check that OD-SOAP score cache is updated when Model changes"""
        m1 = IMP.Model()
        m2 = IMP.Model()
        mh1 = IMP.atom.read_pdb(self.get_input_file_name('soap_loop_test.pdb'),
                                m1)
        mh2 = IMP.atom.read_pdb(self.get_input_file_name('soap_loop_test.pdb'),
                                m2)
        residues = [IMP.atom.Residue(r)
                    for r in IMP.atom.get_by_type(mh2, IMP.atom.RESIDUE_TYPE)]
        p1ca = IMP.atom.Selection(
            mh2, residue_index=1,
            atom_type=IMP.atom.AT_CA).get_selected_particle_indexes()[0]
        p1caa = IMP.atom.Atom(m2, p1ca)
        residues[0].remove_child(p1caa)

        sl = IMP.atom.OrientedSoapPairScore(
            self.get_input_file_name('soap_loop_test.hdf5'))

        p11 = IMP.atom.Selection(
            mh1, residue_index=1,
            atom_type=IMP.atom.AT_C).get_selected_particle_indexes()[0]
        p21 = IMP.atom.Selection(
            mh1, residue_index=3,
            atom_type=IMP.atom.AT_C).get_selected_particle_indexes()[0]
        # Score for m1 should be 1.0 since all atoms are present
        self.assertAlmostEqual(sl.evaluate_index(m1, (p11, p21), None),
                               1.0, delta=1e-6)

        p12 = IMP.atom.Selection(
            mh2, residue_index=1,
            atom_type=IMP.atom.AT_C).get_selected_particle_indexes()[0]
        p22 = IMP.atom.Selection(
            mh2, residue_index=3,
            atom_type=IMP.atom.AT_C).get_selected_particle_indexes()[0]
        # Score for m2 should be 0.0 (not cached value) since we removed
        # the needed atom CA for the interaction
        self.assertAlmostEqual(sl.evaluate_index(m2, (p12, p22), None),
                               0.0, delta=1e-6)

        self.assertAlmostEqual(sl.evaluate_index(m1, (p11, p21), None),
                               1.0, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
