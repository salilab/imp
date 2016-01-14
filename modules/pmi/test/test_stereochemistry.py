import IMP
import IMP.atom
import IMP.test
import IMP.pmi
import IMP.pmi.io
import IMP.pmi.topology
import IMP.pmi.restraints.stereochemistry


class StereochemistryTests(IMP.test.TestCase):

    def test_parse_dssp(self):
        """Test reading DSSP files"""
        sses = IMP.pmi.io.parse_dssp(self.get_input_file_name('chainA.dssp'),'A')
        self.assertEqual(sorted(sses.keys()),sorted(['helix','beta','loop']))
        self.assertEqual(sses['helix'][1][0],['A',100,126])
        self.assertEqual(len(sses['helix']),20)
        self.assertEqual(len(sses['beta']),3)
        self.assertEqual(len(sses['loop']),32)

    def test_elastic_network(self):
        """ test PMI setup of elastic nets """
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name('chainA.fasta'))
        st1 = s.create_state()
        m1 = st1.create_molecule("GCP2_YEAST",sequence=seqs["GCP2_YEAST"],chain_id='A')
        atomic_res = m1.add_structure(self.get_input_file_name('chainA.pdb'),
                                    chain_id='A')
        m1.add_representation(atomic_res,resolutions=[0])
        hier = s.build()

        # create elastic network from some SSEs
        sses = IMP.pmi.io.parse_dssp(self.get_input_file_name('chainA.dssp'),'A')
        er = IMP.pmi.restraints.stereochemistry.ElasticNetworkRestraint(
            selection_tuples=[sses['helix'][0][0]],
            strength=10.0,
            dist_cutoff=5.0,
            ca_only=True,
            hierarchy=hier)
        self.assertEqual(er.get_restraint().get_number_of_restraints(),50)

    def test_excluded_volume_sphere_pmi2(self):
        """ Tests excluded volume restraint in PMI2 """

        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st1 = s.create_state()
        seq = IMP.pmi.topology.Sequences(IMP.pmi.get_data_path("benchmark_sequence.fasta"))
        mol = st1.create_molecule("Test", seq["PDE6-CHAIN-A"])
        mol.add_structure(IMP.pmi.get_data_path("benchmark_starting_structure.pdb"), chain_id="A", offset=0)
        mol.add_representation(mol.get_atomic_residues(), resolutions=[1,10])
        mol.add_representation(mol.get_non_atomic_residues(), resolutions=[10])
        s.build()

        # Test that the correct number of particles are included
        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects=mol, resolution=1)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 748)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects=mol, resolution=10)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 92)

        # Test that default returns lowest resolution
        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects=mol)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 92)

        resis = mol.residue_range(15,20)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects=resis, resolution=1)
        self.assertEqual(len(ev.cpc.get_all_possible_indexes()), 5)

if __name__ == '__main__':
    IMP.test.main()
