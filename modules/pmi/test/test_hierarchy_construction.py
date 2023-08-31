import IMP
import os
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.tools
import IMP.pmi.output

class Tests(IMP.test.TestCase):
    def test_hierarchy_construction(self):
        """Test construction of a hierarchy"""
        self._test_hierarchy_internal(self.get_input_file_name("mini.pdb"))

    def test_hierarchy_construction_mmcif(self):
        """Test construction of a hierarchy using mmCIF inputs"""
        self._test_hierarchy_internal(self.get_input_file_name("mini.cif"))

    def _test_hierarchy_internal(self, pdbfile):
        # input parameter
        fastafile = self.get_input_file_name("mini.fasta")

        components = ["Rpb1", "Rpb2" ]
        chains = "AB"
        colors = [0.,  1.]
        beadsize = 20
        sequences = IMP.pmi.topology.Sequences(fastafile)

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st = simo.create_state()

        for n in range(len(components)):
            mol = st.create_molecule(components[n], sequence=sequences[n])
            atomic = mol.add_structure(pdbfile, chain_id=chains[n])
            mol.add_representation(mol.get_atomic_residues(),
                                   resolutions=[1, 10])
            mol.add_representation(mol.get_non_atomic_residues(),
                                   resolutions=[10])
        hier = simo.build()
        self.assertEqual(len(IMP.atom.get_leaves(hier)), 12)

        ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                        included_objects=hier, resolution=10)
        ev.add_to_model()

        o = IMP.pmi.output.Output()
        o.init_rmf("conformations.rmf", [hier])
        o.write_rmf("conformations.rmf")

        o.close_rmf("conformations.rmf")
        os.unlink('conformations.rmf')

    def test_hierarchy_mse_construction(self):
        """Test construction of a hierarchy containing MSE"""
        pdbfile = self.get_input_file_name("mini_mse.pdb")
        fastafile = self.get_input_file_name("mini_mse.fasta")
        sequences = IMP.pmi.topology.Sequences(fastafile)

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st = simo.create_state()

        mol = st.create_molecule('Rpb1', sequence=sequences[0])
        atomic = mol.add_structure(pdbfile, chain_id='A')
        # Only build residues that are in the PDB file
        mol.add_representation(mol.get_atomic_residues(),
                               resolutions=[1])
        hier = simo.build()
        residues = IMP.atom.get_by_type(hier, IMP.atom.RESIDUE_TYPE)
        rtypes = [IMP.atom.Residue(r).get_residue_type().get_string()
                  for r in residues]
        # All residues in PDB file (including the first one, MSE, mapped to
        # MET here) should be present
        self.assertEqual(rtypes,
                         ['MET', 'GLY', 'GLN', 'GLN', 'TYR', 'SER', 'SER'])


if __name__ == '__main__':
    IMP.test.main()
