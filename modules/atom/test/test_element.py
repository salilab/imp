from StringIO import StringIO
import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):

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
        self.assertAlmostEqual(mass, 13346, delta=1.0)

    def test_element_write(self):
        """Check writing to pdb unknown elements"""
        m = IMP.Model()
        chain = IMP.atom.Chain.setup_particle(IMP.Particle(m), 'A')
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m))
        atom = IMP.atom.Atom.setup_particle(IMP.Particle(m), IMP.atom.AT_CA)
        xyz = IMP.core.XYZ.setup_particle(atom.get_particle(), IMP.algebra.Vector3D(0,0,0))
        chain.add_child(residue)
        residue.add_child(atom)

    def test_element_assignment(self):
        """Check element assignment based on atom name"""
        m = IMP.Model()

        #! read PDB
        mp = IMP.atom.read_pdb(self.open_input_file("elements.pdb"),
                               m, IMP.atom.NonWaterPDBSelector())
        ps = IMP.atom.get_by_type(mp, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(ps), 13)
        self.assertEqual(IMP.atom.Atom(ps[0].get_particle()).get_element(), IMP.atom.P)
        self.assertEqual(IMP.atom.Atom(ps[1].get_particle()).get_element(), IMP.atom.O)
        self.assertEqual(IMP.atom.Atom(ps[2].get_particle()).get_element(), IMP.atom.O)
        self.assertEqual(IMP.atom.Atom(ps[3].get_particle()).get_element(), IMP.atom.Ca)
        self.assertEqual(IMP.atom.Atom(ps[4].get_particle()).get_element(), IMP.atom.Mg)
        self.assertEqual(IMP.atom.Atom(ps[5].get_particle()).get_element(), IMP.atom.N)
        self.assertEqual(IMP.atom.Atom(ps[6].get_particle()).get_element(), IMP.atom.C)
        self.assertEqual(IMP.atom.Atom(ps[7].get_particle()).get_element(), IMP.atom.Cl)
        self.assertEqual(IMP.atom.Atom(ps[8].get_particle()).get_element(), IMP.atom.N)
        self.assertEqual(IMP.atom.Atom(ps[9].get_particle()).get_element(), IMP.atom.C)
        self.assertEqual(IMP.atom.Atom(ps[10].get_particle()).get_element(), IMP.atom.Ir)
        self.assertEqual(IMP.atom.Atom(ps[11].get_particle()).get_element(), IMP.atom.Pt)
        self.assertEqual(IMP.atom.Atom(ps[12].get_particle()).get_element(), IMP.atom.Na)

if __name__ == '__main__':
    IMP.test.main()
