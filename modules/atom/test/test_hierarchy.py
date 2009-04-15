import unittest
import IMP
import IMP.test
import IMP.atom

ATOM = IMP.atom.MolecularHierarchyDecorator.ATOM
RESIDUE = IMP.atom.MolecularHierarchyDecorator.RESIDUE
UNKNOWN = IMP.atom.MolecularHierarchyDecorator.UNKNOWN

def _make_hierarchy_decorators(m, *types):
    decorators = []
    for t in types:
        p = IMP.Particle(m)
        d = IMP.atom.MolecularHierarchyDecorator.create(p, t)
        decorators.append(d)
    return decorators

def _make_bonded_atoms(m):
    r1, r2 = _make_hierarchy_decorators(m, RESIDUE, RESIDUE)
    atoms1 = _make_hierarchy_decorators(m, ATOM, ATOM, ATOM, ATOM)
    atoms2 = _make_hierarchy_decorators(m, ATOM, ATOM, ATOM, ATOM)
    for a in atoms1:
        r1.add_child(a)
    for a in atoms2:
        r2.add_child(a)
    bonded1 = [IMP.atom.BondedDecorator.create(a.get_particle())
               for a in atoms1]
    bonded2 = [IMP.atom.BondedDecorator.create(a.get_particle())
               for a in atoms2]
    return r1, r2, bonded1, bonded2


class HierarchyTests(IMP.test.TestCase):

    def test_get_by_type(self):
        """Check hierarchy get_by_type"""
        m = IMP.Model()
        r1, r2, a1, a2 = _make_hierarchy_decorators(m, RESIDUE, RESIDUE,
                                                    ATOM, ATOM)
        r1.add_child(a1)
        r1.add_child(a2)
        # Both atoms should be found under r1 (none under r2)
        self.assertEqual(IMP.atom.get_by_type(r1, ATOM),
                         (a1.get_particle(), a2.get_particle()))
        self.assertEqual(IMP.atom.get_by_type(r2, ATOM), ())

        # Each residue should be found under itself
        self.assertEqual(IMP.atom.get_by_type(r1, RESIDUE),
                         (r1.get_particle(),))
        self.assertEqual(IMP.atom.get_by_type(r2, RESIDUE),
                         (r2.get_particle(),))

        # Neither residue should be found under any atom
        self.assertEqual(IMP.atom.get_by_type(a1, RESIDUE), ())
        self.assertEqual(IMP.atom.get_by_type(a2, RESIDUE), ())

    def test_get_add_child(self):
        """Check hierarchy get_child and add_child"""
        m = IMP.Model()
        parent, child = _make_hierarchy_decorators(m, RESIDUE, ATOM)
        self.assertEqual(parent.add_child(child), 0)
        self.assertEqual(parent.get_child(0), child)
        self.assertEqual(child.get_parent(), parent)
        self.assertRaises(IndexError, parent.get_child, 1)

        # Cannot put a residue under an atom
        parent, child = _make_hierarchy_decorators(m, ATOM, RESIDUE)
        self.assertRaises(ValueError, parent.add_child, child)
        # Neither parent nor child can be UNKNOWN
        parent, child = _make_hierarchy_decorators(m, ATOM, UNKNOWN)
        self.assertRaises(ValueError, parent.add_child, child)
        parent, child = _make_hierarchy_decorators(m, UNKNOWN, ATOM)
        self.assertRaises(ValueError, parent.add_child, child)

    def test_get_set_type(self):
        """Check hierarchy get_type, get_type_string and set_type"""
        m = IMP.Model()
        (d,) = _make_hierarchy_decorators(m, UNKNOWN)
        self.assertEqual(d.get_type(), UNKNOWN)
        for (typ, string) in ((ATOM, 'atom'), (RESIDUE, 'residue'),
                              (UNKNOWN, 'unknown')):
            d.set_type(typ)
            self.assertEqual(d.get_type(), typ)
            self.assertEqual(d.get_type_string(), string)

    def test_get_internal_bonds(self):
        """Check hierarchy get_internal_bonds"""
        m = IMP.Model()
        # Bonds external to the hierarchy should not be counted
        r1, r2, bonded1, bonded2 = _make_bonded_atoms(m)
        IMP.atom.bond(bonded1[0], bonded2[0], IMP.atom.BondDecorator.COVALENT)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r1)), 0)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r2)), 0)

        # Each bond should only be counted once
        r1, r2, bonded1, bonded2 = _make_bonded_atoms(m)
        IMP.atom.bond(bonded1[0], bonded1[3], IMP.atom.BondDecorator.COVALENT)
        bonds = IMP.atom.get_internal_bonds(r1)
        self.assertEqual(len(bonds), 1)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r2)), 0)
        self.assertEqual(bonds[0].get_bonded(0), bonded1[0])
        self.assertEqual(bonds[0].get_bonded(1), bonded1[3])

        # Check multiple bonds to one particle
        r1, r2, bonded1, bonded2 = _make_bonded_atoms(m)
        for end in (1, 2, 3):
            IMP.atom.bond(bonded1[0], bonded1[end],
                          IMP.atom.BondDecorator.COVALENT)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r1)), 3)

if __name__ == '__main__':
    unittest.main()
