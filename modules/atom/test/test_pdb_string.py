import unittest
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
            f.write(IMP.atom.pdb_string(IMP.core.XYZ(p).get_coordinates(),
                             i,IMP.atom.AT_CA,IMP.atom.ALA,'A',i))
        f.close()
        mp = IMP.atom.read_pdb(name,
                               m, IMP.atom.CAlphaPDBSelector())
        os.unlink(name)
        self.assertEqual(len(IMP.core.get_leaves(mp)),5)
if __name__ == '__main__':
    unittest.main()
