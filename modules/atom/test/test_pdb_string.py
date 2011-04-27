from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom
import os
class PDBReadWriteTest(IMP.test.TestCase):
    def test_write(self):
        """Simple test of writing a PDB"""
        m = IMP.Model()
        name="test.pdb"
        f=open(name,"w")
        #create a pdb file of 5 CA atoms
        for i in range(5):
            p=IMP.Particle(m)
            IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(i,i,i))
            f.write(IMP.atom.get_pdb_string(IMP.core.XYZ(p).get_coordinates(),
                             i,IMP.atom.AT_CA,IMP.atom.ALA,'A',i))
        f.close()
        mp = IMP.atom.read_pdb(name,
                               m, IMP.atom.CAlphaPDBSelector())
        os.unlink(name)
        self.assertEqual(len(IMP.core.get_leaves(mp)),5)

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

            m = IMP.Model()
            pdb = IMP.atom.read_pdb(s, m)

            s = StringIO()
            IMP.atom.write_pdb(pdb, s)
            print s.getvalue()
            self.assertEqual(s.getvalue()[15+12:15+16].strip(), atom)

if __name__ == '__main__':
    IMP.test.main()
