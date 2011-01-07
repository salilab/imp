import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra

def get_bonds(numbonds, getfunc, numendpoints):
    """Get all of the bonds (or angles, dihedrals) from a residue topology,
       as a list of strings."""
    def get_atom_name(bond, i):
        atom_name = bond.get_endpoint(i).get_atom_name()
        # Allow for difference in patch atom naming between CHARMM and IMP
        if len(atom_name) >= 2 and atom_name[0] in '12' and atom_name[1] == ':':
            atom_name = atom_name[0] + atom_name[2:]
        return atom_name
    bonds = []
    for bond in [getfunc(i) for i in range(numbonds)]:
        bond_str = "-".join([get_atom_name(bond, i)
                             for i in range(numendpoints)])
        bonds.append(bond_str)
    # Note that bonds are not ordered in the topology, so we must sort
    # them here in order to be able to compare them later
    bonds.sort()
    return bonds

class CHARMMParametersTests(IMP.test.TestCase):
    """Test the CHARMMParameters class"""

    def test_bond_parameters(self):
        """Check extraction of bond parameters"""
        p = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                      IMP.atom.get_data_path('par.lib'))
        self.assertRaises(IndexError, p.get_bond_parameters, 'garbage', 'CT2')
        for types in [('CT1', 'CT2'), ('CT2', 'CT1')]:
            bond = p.get_bond_parameters(*types)
            self.assertAlmostEqual(bond.force_constant, 222.500, delta=1e-4)
            self.assertAlmostEqual(bond.ideal, 1.5380, delta=1e-5)

        self.assertRaises(IndexError, p.get_angle_parameters,
                          'garbage', 'CT2', 'CT3')
        for types in [('OM', 'CM', 'FE'), ('FE', 'CM', 'OM')]:
            bond = p.get_angle_parameters(*types)
            self.assertAlmostEqual(bond.force_constant, 35.000, delta=1e-4)
            self.assertAlmostEqual(bond.ideal, 180.0000, delta=1e-5)
        self.assertRaises(IndexError, p.get_angle_parameters, 'OM', 'FE', 'CM')

        self.assertRaises(IndexError, p.get_dihedral_parameters,
                          'garbage', 'C', 'CT2', 'CT3')
        # Check multiple dihedrals
        bonds = p.get_dihedral_parameters('CP1', 'C', 'N', 'CP1')
        self.assertEqual(len(bonds), 2)
        self.assertAlmostEqual(bonds[0].force_constant, 2.7500, delta=1e-4)
        self.assertEqual(bonds[0].multiplicity, 2)
        self.assertAlmostEqual(bonds[0].ideal, 180.00, delta=1e-5)
        self.assertAlmostEqual(bonds[1].force_constant, 0.3000, delta=1e-4)
        self.assertEqual(bonds[1].multiplicity, 4)
        self.assertAlmostEqual(bonds[1].ideal, 0.00, delta=1e-5)

        # Check wildcards
        bonds = p.get_dihedral_parameters('OM', 'CT1', 'NH3', 'FE')
        self.assertEqual(len(bonds), 1)
        self.assertAlmostEqual(bonds[0].force_constant, 0.1000, delta=1e-4)
        self.assertEqual(bonds[0].multiplicity, 3)
        self.assertAlmostEqual(bonds[0].ideal, 0.00, delta=1e-5)

        self.assertRaises(IndexError, p.get_improper_parameters,
                          'garbage', 'C', 'CT2', 'CT3')
        for types in [('CPB', 'CPA', 'NPH', 'CPA'),
                      ('CPA', 'NPH', 'CPA', 'CPB')]:
            bond = p.get_improper_parameters(*types)
            self.assertAlmostEqual(bond.force_constant, 20.800, delta=1e-4)
            self.assertEqual(bond.multiplicity, 0)
            self.assertAlmostEqual(bond.ideal, 0.0000, delta=1e-5)
        # Check wildcards
        for types in [('CPB', 'CPA', 'NPH', 'C'),
                      ('CPB', 'CT2', 'CT3', 'C')]:
            bond = p.get_improper_parameters(*types)
            self.assertAlmostEqual(bond.force_constant, 90.000, delta=1e-4)
            self.assertEqual(bond.multiplicity, 0)
            self.assertAlmostEqual(bond.ideal, 0.0000, delta=1e-5)
        self.assertRaises(IndexError, p.get_improper_parameters,
                          'NPH', 'CPA', 'CPB', 'CPA')

    def test_read_topology(self):
        """Check parsing of topology file"""
        p = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'))
        r = p.get_residue_topology("ALA")
        self.assertEqual(r.get_number_of_internal_coordinates(), 12)
        ic = r.get_internal_coordinate(0)
        ep = [ic.get_endpoint(i).get_atom_name() for i in range(4)]
        self.assertEqual(ep, ['-C', 'CA', 'N', 'H'])
        self.assertEqual(ic.get_contains_atom('-C'), True)
        self.assertEqual(ic.get_contains_atom('C'), False)
        self.assertEqual(ic.get_contains_atom('*N'), False)
        self.assertEqual(ic.get_contains_atom('N'), True)
        self.assertAlmostEqual(ic.get_first_distance(), 1.3551, delta=1e-4)
        self.assertAlmostEqual(ic.get_first_angle(), 126.4900, delta=1e-4)
        self.assertEqual(ic.get_improper(), True)
        self.assertAlmostEqual(ic.get_dihedral(), 180.0000, delta=1e-4)
        self.assertAlmostEqual(ic.get_second_angle(), 115.4200, delta=1e-4)
        self.assertAlmostEqual(ic.get_second_distance(), 0.9996, delta=1e-4)
        ic = r.get_internal_coordinate(1)
        self.assertEqual(ic.get_improper(), False)
        ep = [ic.get_endpoint(i).get_atom_name() for i in range(4)]
        self.assertEqual(ep, ['-C', 'N', 'CA', 'C'])

    def assertResidueTopologiesEqual(self, rpdb, rcharmm):
        # Atom names must also match
        self.assertEqual(rpdb.get_number_of_atoms(),
                         rcharmm.get_number_of_atoms())
        for apdb in [rpdb.get_atom(i) \
                     for i in range(rpdb.get_number_of_atoms())]:
            pdb_name = apdb.get_name()
            # Allow for different patch atom naming between CHARMM and
            # our topology
            if len(pdb_name) >= 2 and pdb_name[0] in '12' \
               and pdb_name[1] == ':':
                acharmm = rcharmm.get_atom(pdb_name[0] + pdb_name[2:])
            else:
                acharmm = rcharmm.get_atom(pdb_name)
            self.assertEqual(apdb.get_charmm_type(),
                             acharmm.get_charmm_type())
        # Bonds must also match
        bonds_pdb = get_bonds(rpdb.get_number_of_bonds(),
                              rpdb.get_bond, 2)
        bonds_charmm = get_bonds(rcharmm.get_number_of_bonds(),
                                 rcharmm.get_bond, 2)
        self.assertEqual(bonds_pdb, bonds_charmm)
        # Angles must also match
        angles_pdb = get_bonds(rpdb.get_number_of_angles(),
                               rpdb.get_angle, 3)
        angles_charmm = get_bonds(rcharmm.get_number_of_angles(),
                                  rcharmm.get_angle, 3)
        self.assertEqual(angles_pdb, angles_charmm)
        # Dihedrals must also match
        dihedrals_pdb = get_bonds(rpdb.get_number_of_dihedrals(),
                                  rpdb.get_dihedral, 4)
        dihedrals_charmm = get_bonds(rcharmm.get_number_of_dihedrals(),
                                     rcharmm.get_dihedral, 4)
        self.assertEqual(dihedrals_pdb, dihedrals_charmm)
        # Impropers do not need to match exactly, since the IMP/Modeller
        # topology includes some extras, but every CHARMM improper must
        # be in our topology
        impropers_pdb = get_bonds(rpdb.get_number_of_impropers(),
                                  rpdb.get_improper, 4)
        impropers_charmm = get_bonds(rcharmm.get_number_of_impropers(),
                                     rcharmm.get_improper, 4)
        for i in impropers_charmm:
            self.assertIn(i, impropers_pdb)

    def test_map_names_to_pdb(self):
        """Check mapping of CHARMM names to PDB"""
        # Read CHARMM file containing PDB atom and residue names
        ppdb = IMP.atom.CHARMMParameters(IMP.atom.get_data_path('top.lib'),
                                         IMP.atom.get_data_path('par.lib'))
        # Read CHARMM file containing CHARMM atom and residue names; map names
        # to PDB
        pcharmm = IMP.atom.CHARMMParameters(
                          self.get_input_file_name('top_all22_prot_b5.inp'),
                          IMP.atom.get_data_path('par.lib'), True)
        # After mapping, atom and residue names should match
        for res in ['ALA', 'CYS', 'ASP', 'GLU', 'PHE', 'GLY', 'HIS', 'ILE',
                    'LYS', 'LEU', 'MET', 'ASN', 'PRO', 'GLN', 'ARG', 'SER',
                    'THR', 'VAL', 'TRP', 'TYR', 'TIP3', 'CAL']:
            rpdb = ppdb.get_residue_topology(res)
            rcharmm = pcharmm.get_residue_topology(res)
            self.assertResidueTopologiesEqual(rpdb, rcharmm)
        for res in ['NTER', 'CTER', 'GLYP', 'PROP', 'LINK', 'DISU']:
            rpdb = ppdb.get_patch(res)
            rcharmm = pcharmm.get_patch(res)
            self.assertResidueTopologiesEqual(rpdb, rcharmm)
            # Deleted atom lists must also match
            def get_removed_atom(res, i):
                atom_name = res.get_removed_atom(i)
                # Allow for difference in patch atom naming between CHARMM
                # and IMP
                if len(atom_name) >= 2 and atom_name[0] in '12' \
                   and atom_name[1] == ':':
                    atom_name = atom_name[0] + atom_name[2:]
                return atom_name
            atoms_pdb = [get_removed_atom(rpdb, i) \
                         for i in range(rpdb.get_number_of_removed_atoms())]
            atoms_charmm = [get_removed_atom(rcharmm, i) \
                         for i in range(rcharmm.get_number_of_removed_atoms())]
            self.assertEqual(atoms_pdb, atoms_charmm)

if __name__ == '__main__':
    IMP.test.main()
