import unittest
from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.algebra

class ElementTableTest(IMP.test.TestCase):

    def test_element_mass(self):
        """Check assigning elements and masses to pdb with one protein"""
        m = IMP.Model()
        # read PDB, assign elements and masses
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterSelector())
        atoms = IMP.atom.get_by_type(mp, IMP.atom.Hierarchy.ATOM)
        # compute mass
        # mass is assigned using elements, so we are testing both here
        mass = 0
        for a in atoms:
            mass += a.get_mass()
        print mass
        self.assertInTolerance(mass, 13346.4, 1.0)

    def test_element_write(self):
        """Check writing to pdb unknown elements"""
        m = IMP.Model()
        rp = IMP.Particle(m)
        ap = IMP.Particle(m)
        cp = IMP.Particle(m);
        chain = IMP.atom.Chain.create(cp, 'A')
        residue = IMP.atom.Residue.create(rp)
        atom = IMP.atom.Atom.create(ap, IMP.atom.AT_CA)
        xyz = IMP.core.XYZ.create(ap)
        hcd = IMP.atom.Hierarchy.cast(cp)
        hrd = IMP.atom.Hierarchy.cast(rp)
        had = IMP.atom.Hierarchy.cast(ap)
        hcd.add_child(hrd)
        hrd.add_child(had)
        print atom.get_pdb_string()

if __name__ == '__main__':
    unittest.main()
