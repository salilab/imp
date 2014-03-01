from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom
import os


class Tests(IMP.test.TestCase):

    def test_pdb_conect_string(self):
        """Test get_pdb_conect_record_string()"""
        s = IMP.atom.get_pdb_conect_record_string(1001, 2)
        self.assertEqual(s, 'CONECT 1001    2\n')

    def test_write(self):
        """Simple test of writing a PDB"""
        m = IMP.kernel.Model()
        name = "test.pdb"
        f = open(name, "w")
        # create a pdb file of 5 CA atoms
        for i in range(5):
            p = IMP.kernel.Particle(m)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(i, i, i))
            f.write(IMP.atom.get_pdb_string(IMP.core.XYZ(p).get_coordinates(),
                                            i, IMP.atom.AT_CA, IMP.atom.ALA, 'A', i))
        f.close()
        mp = IMP.atom.read_pdb(name,
                               m, IMP.atom.CAlphaPDBSelector())
        os.unlink(name)
        self.assertEqual(len(IMP.core.get_leaves(mp)), 5)
        m.evaluate(False)

    def test_pad(self):
        """Make sure that atom names are padded correctly in PDB files"""
        # Make sure non-standard atom types are defined
        ff = IMP.atom.get_all_atom_CHARMM_parameters()
        v = IMP.algebra.Vector3D(0, 0, 0)
        t = IMP.atom.get_element_table()
        for name, element, expected in (('C', 'C', ' C  '),
                                        ('C', 'Un', ' C  '),
                                        ('CA', 'C', ' CA '),
                                        ('CA', 'CA', 'CA  '),
                                        ('HET:CAD', 'C', ' CAD'),
                                        ('HET:HAD1', 'H', 'HAD1'),
                                        ('HET:HG', 'H', ' HG '),
                                        ('HG', 'HG', 'HG  ')):
            s = IMP.atom.get_pdb_string(v, 1, IMP.atom.AtomType(name),
                                        IMP.atom.ALA, 'A', 1, ' ', 1.0, 0.0,
                                        t.get_element(element))
            self.assertEqual(s[12:16], expected)

    def test_no_mangle(self):
        """Test that PDB atom names are not mangled"""
        # Atom names should not be changed by a read/write PDB cycle; this would
        # break usage of PDBs containing CHARMM atom names.
        for atom in ('OT1', 'OT2', 'OXT', 'HE21', 'HE22', '1HE2',
                     '2HE2', 'foo'):
            s = StringIO()
            s.write('ATOM      2 %-4s ALA A   1      17.121  17.162   '
                    '6.197  1.00 15.60           C\n' % atom)
            s.seek(0)

            m = IMP.kernel.Model()
            pdb = IMP.atom.read_pdb(s, m)

            s = StringIO()
            IMP.atom.write_pdb(pdb, s)
            m.evaluate(False)
            print s.getvalue()
            self.assertEqual(s.getvalue()[15 + 12:15 + 16].strip(), atom)

    def test_read_atom(self):
        """Test that all fields are read from PDB ATOM records"""
        s = StringIO()
        # PDB is fixed-format; we should be able to read coordinates even
        # without spaces between them
        s.write('ATOM      1  N   ALA A   5    3000.0001000.4002000.600'
                '  2.00  6.40           N\n')
        s.seek(0)

        m = IMP.kernel.Model()
        pdb = IMP.atom.read_pdb(s, m)
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 1)
        a = IMP.atom.Atom(atoms[0])
        r = IMP.atom.Residue(a.get_parent())
        c = IMP.atom.Chain(r.get_parent())
        self.assertEqual(a.get_input_index(), 1)
        self.assertEqual(a.get_atom_type().get_string(), 'N')
        # Note: currently don't read alternate location or insertion code
        self.assertEqual(r.get_residue_type().get_string(), 'ALA')
        self.assertEqual(c.get_id(), 'A')
        self.assertEqual(r.get_index(), 5)
        coord = a.get_as_xyz().get_coordinates()
        self.assertAlmostEqual(coord[0], 3000.000, delta=0.001)
        self.assertAlmostEqual(coord[1], 1000.400, delta=0.001)
        self.assertAlmostEqual(coord[2], 2000.600, delta=0.001)
        self.assertAlmostEqual(a.get_occupancy(), 2.00, delta=0.01)
        self.assertAlmostEqual(a.get_temperature_factor(), 6.40, delta=0.01)

    def test_read_short_atom_line(self):
        """Test that we can read PDB ATOM record with coordinates only"""
        s = StringIO()
        s.write('ATOM                          3000.0001000.4002000.600\n')
        s.seek(0)
        m = IMP.kernel.Model()
        pdb = IMP.atom.read_pdb(s, m)
        atoms = IMP.atom.get_by_type(pdb, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 1)
        coord = IMP.core.XYZ(atoms[0]).get_coordinates()
        self.assertAlmostEqual(coord[0], 3000.000, delta=0.001)
        self.assertAlmostEqual(coord[1], 1000.400, delta=0.001)
        self.assertAlmostEqual(coord[2], 2000.600, delta=0.001)

if __name__ == '__main__':
    IMP.test.main()
