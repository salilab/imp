import IMP
import IMP.test

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.topology


class Tests(IMP.test.TestCase):
    def make_topology(self):
        # input parameter
        pdbfile = self.get_input_file_name("1WCM.pdb")
        seqs = IMP.pmi.topology.Sequences(self.get_input_file_name(
                                                '1WCM.fasta.txt'))

        components = ["Rpb3"]
        chains = "CCD"
        colors = [0., 0.5, 1.0]
        beadsize = 20

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st1 = simo.create_state()
        hier_dict = {}

        for n in range(len(components)):
            mol = st1.create_molecule(components[n],
                                      sequence=seqs["1WCM:" + chains[n]])
            hier_dict[components[n]] = mol
            atomic = mol.add_structure(pdbfile, chain_id=chains[n])
            mol.add_representation(mol.get_atomic_residues(),
                                   resolutions=[1, 10, 100], color=colors[n])
            mol.add_representation(mol.get_non_atomic_residues(),
                                   resolutions=[10], color=colors[n])
        root_hier = simo.build()
        return m, root_hier, hier_dict

    def test_residue_restraints(self):
        """Test Residue(Bond|Angle|Dihedral)Restraint"""
        m, root_hier, hier_dict = self.make_topology()

        rb = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(
            objects=hier_dict["Rpb3"][29:40])
        rb.add_to_model()
        out = rb.get_output()(None)
        self.assertAlmostEqual(float(out['_TotalScore']), 0.016708,
                               delta=1e-4)

        ra = IMP.pmi.restraints.stereochemistry.ResidueAngleRestraint(
            objects=hier_dict["Rpb3"][29:40])
        ra.add_to_model()
        out = ra.get_output()(None)
        self.assertAlmostEqual(float(out['_TotalScore']), 1.0178198,
                               delta=1e-4)

        # Test default (all trans)
        rd = IMP.pmi.restraints.stereochemistry.ResidueDihedralRestraint(
            objects=hier_dict["Rpb3"][29:40])
        rd.add_to_model()
        out = rd.get_output()(None)
        self.assertAlmostEqual(float(out['_TotalScore']), 51.451047,
                               delta=1e-4)

        # Specify cis or trans
        rd = IMP.pmi.restraints.stereochemistry.ResidueDihedralRestraint(
            objects=hier_dict["Rpb3"][29:40], stringsequence='TTTTCCTT')
        rd.add_to_model()
        out = rd.get_output()(None)
        self.assertAlmostEqual(float(out['_TotalScore']), 44.721909,
                               delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
