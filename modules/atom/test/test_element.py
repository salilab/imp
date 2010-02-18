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
                              m, IMP.atom.NonWaterPDBSelector())
        atoms = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        # compute mass
        # mass is assigned using elements, so we are testing both here
        mass = 0
        for a in atoms:
            mass += IMP.atom.Mass(a.get_particle()).get_mass()
        print mass
        self.assertInTolerance(mass, 13346, 1.0)

    def test_element_write(self):
        """Check writing to pdb unknown elements"""
        m = IMP.Model()
        chain = IMP.atom.Chain.setup_particle(IMP.Particle(m), 'A')
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m))
        atom = IMP.atom.Atom.setup_particle(IMP.Particle(m), IMP.atom.AT_CA)
        xyz = IMP.core.XYZ.setup_particle(atom.get_particle(), IMP.algebra.Vector3D(0,0,0))
        chain.add_child(residue)
        residue.add_child(atom)

if __name__ == '__main__':
    unittest.main()
