import IMP
import IMP.test
import IMP.atom


def _make_hierarchy_decorators(m, *types):
    decorators = []
    for t in types:
        p = IMP.kernel.Particle(m)
        d = t[0].setup_particle(p, t[1])
        decorators.append(d)
    return decorators


def _make_bonded_atoms(m):
    r1, r2 = _make_hierarchy_decorators(
        m, (IMP.atom.Residue, IMP.atom.VAL), (IMP.atom.Residue, IMP.atom.VAL))
    atoms1 = _make_hierarchy_decorators(m, (IMP.atom.Atom, IMP.atom.AT_C), (
        IMP.atom.Atom, IMP.atom.AT_C), (IMP.atom.Atom, IMP.atom.AT_C), (IMP.atom.Atom, IMP.atom.AT_C))
    atoms2 = _make_hierarchy_decorators(m, (IMP.atom.Atom, IMP.atom.AT_C), (
        IMP.atom.Atom, IMP.atom.AT_C), (IMP.atom.Atom, IMP.atom.AT_C), (IMP.atom.Atom, IMP.atom.AT_C))
    for a in atoms1:
        r1.add_child(a)
    for a in atoms2:
        r2.add_child(a)
    bonded1 = [IMP.atom.Bonded.setup_particle(a.get_particle())
               for a in atoms1]
    bonded2 = [IMP.atom.Bonded.setup_particle(a.get_particle())
               for a in atoms2]
    return r1, r2, bonded1, bonded2


class Tests(IMP.test.TestCase):

    def test_get_by_type(self):
        """Check hierarchy get_by_type"""
        m = IMP.kernel.Model()
        s1, s2, r1, r2, a1, a2 = _make_hierarchy_decorators(
            m, (IMP.atom.State,1), (IMP.atom.State,2),
            (IMP.atom.Residue, IMP.atom.VAL), (IMP.atom.Residue, IMP.atom.VAL),
            (IMP.atom.Atom, IMP.atom.AT_C), (IMP.atom.Atom, IMP.atom.AT_C))
        s1.add_child(r1)
        s2.add_child(r2)
        r1.add_child(a1)
        r1.add_child(a2)
        self.assertEqual(s1, IMP.atom.get_root(a1))

        # r1 should be found in state 1, r2 in state 2
        self.assertEqual(list(IMP.atom.get_by_type(s1, IMP.atom.RESIDUE_TYPE)),
                         [r1.get_particle()])
        self.assertEqual(list(IMP.atom.get_by_type(s2, IMP.atom.RESIDUE_TYPE)),
                         [r2.get_particle()])

        # each state should be found under itself
        self.assertEqual(list(IMP.atom.get_by_type(s1, IMP.atom.STATE_TYPE)),
                         [s1.get_particle()])
        self.assertEqual(list(IMP.atom.get_by_type(s2, IMP.atom.STATE_TYPE)),
                         [s2.get_particle()])

        # Both atoms should be found under r1 (none under r2)
        self.assertEqual(list(IMP.atom.get_by_type(r1, IMP.atom.ATOM_TYPE)),
                         [a1.get_particle(), a2.get_particle()])
        self.assertEqual(
            list(IMP.atom.get_by_type(r2, IMP.atom.ATOM_TYPE)), [])

        # Each residue should be found under itself
        self.assertEqual(list(IMP.atom.get_by_type(r1, IMP.atom.RESIDUE_TYPE)),
                         [r1.get_particle(), ])
        self.assertEqual(list(IMP.atom.get_by_type(r2, IMP.atom.RESIDUE_TYPE)),
                         [r2.get_particle(), ])

        # Neither residue should be found under any atom
        self.assertEqual(
            list(IMP.atom.get_by_type(a1, IMP.atom.RESIDUE_TYPE)), [])
        self.assertEqual(
            list(IMP.atom.get_by_type(a2, IMP.atom.RESIDUE_TYPE)), [])

    def test_get_internal_bonds(self):
        """Check hierarchy get_internal_bonds"""
        m = IMP.kernel.Model()
        # Bonds external to the hierarchy should not be counted
        r1, r2, bonded1, bonded2 = _make_bonded_atoms(m)
        IMP.atom.create_bond(bonded1[0], bonded2[0], IMP.atom.Bond.SINGLE)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r1)), 0)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r2)), 0)

        # Each bond should only be counted once
        r1, r2, bonded1, bonded2 = _make_bonded_atoms(m)
        IMP.atom.create_bond(bonded1[0], bonded1[3], IMP.atom.Bond.SINGLE)
        bonds = IMP.atom.get_internal_bonds(r1)
        self.assertEqual(len(bonds), 1)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r2)), 0)
        self.assertEqual(bonds[0].get_bonded(0), bonded1[0])
        self.assertEqual(bonds[0].get_bonded(1), bonded1[3])

        # Check multiple bonds to one particle
        r1, r2, bonded1, bonded2 = _make_bonded_atoms(m)
        for end in (1, 2, 3):
            IMP.atom.create_bond(bonded1[0], bonded1[end],
                                 IMP.atom.Bond.SINGLE)
        self.assertEqual(len(IMP.atom.get_internal_bonds(r1)), 3)

    def test_equality(self):
        """Check equality of AtomType types"""
        at0 = IMP.atom.AtomType(0)
        at1 = IMP.atom.AtomType(0)
        at2 = IMP.atom.AtomType(1)
        self.assertEqual(at0, at1)
        self.assertNotEqual(at0, at2)

if __name__ == '__main__':
    IMP.test.main()
