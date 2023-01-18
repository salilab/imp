from __future__ import print_function
import IMP
import IMP.test
import IMP.atom
import string
import pickle


class Tests(IMP.test.TestCase):

    def test_chain_ids(self):
        """Check chain IDs assigned by create_hierarchy()"""
        topology = IMP.atom.CHARMMTopology(
            IMP.atom.get_heavy_atom_CHARMM_parameters())
        topology.add_sequence('A/' * 56 + 'A')
        m = IMP.Model()
        h = topology.create_hierarchy(m)
        chains = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)
        cids = [IMP.atom.Chain(c).get_id() for c in chains]
        letters = string.ascii_uppercase
        expected = ([x for x in letters]
                    + ['A' + x for x in letters]
                    + ['B' + x for x in letters[:5]])
        self.assertEqual(cids, expected)

    def test_add_sequence(self):
        """Test CHARMMTopology::add_sequence()"""
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        t = IMP.atom.CHARMMTopology(ff)
        self.assertEqual(t.get_number_of_segments(), 0)
        # Should add an empty segment
        t.add_sequence('')
        self.assertEqual(t.get_number_of_segments(), 1)
        self.assertEqual(t.get_segment(0).get_number_of_residues(), 0)

        # Should add two non-empty segments
        t = IMP.atom.CHARMMTopology(ff)
        t.add_sequence('ACY/MF')
        self.assertEqual(t.get_number_of_segments(), 2)
        self.assertEqual(t.get_segment(0).get_number_of_residues(), 3)
        self.assertEqual(t.get_segment(1).get_number_of_residues(), 2)

        # Should add three non-empty segments
        t = IMP.atom.CHARMMTopology(ff)
        t.add_sequence('ACG/(ADE)(CYT)(GUA)/(DADE)(DGUA)')
        self.assertEqual(t.get_number_of_segments(), 3)
        self.assertEqual(t.get_segment(0).get_number_of_residues(), 3)
        self.assertEqual(t.get_segment(1).get_number_of_residues(), 3)
        self.assertEqual(t.get_segment(2).get_number_of_residues(), 2)
        self.assertEqual(
            [r.get_type() for r in t.get_segment(0).get_residues()],
            ['ALA', 'CYS', 'GLY'])
        self.assertEqual(
            [r.get_type() for r in t.get_segment(1).get_residues()],
            ['A', 'C', 'G'])
        self.assertEqual(
            [r.get_type() for r in t.get_segment(2).get_residues()],
            ['DA', 'DG'])

        # Invalid one-letter codes should cause an exception and add no
        # segments
        t = IMP.atom.CHARMMTopology(ff)
        self.assertRaises(ValueError, t.add_sequence, 'a')
        self.assertEqual(t.get_number_of_segments(), 0)

        # Invalid three-letter codes should cause an exception and add no
        # segments
        t = IMP.atom.CHARMMTopology(ff)
        self.assertRaises(ValueError, t.add_sequence, '(aaa)')
        self.assertEqual(t.get_number_of_segments(), 0)

        # Unterminated three-letter codes should cause an exception and add no
        # segments
        t = IMP.atom.CHARMMTopology(ff)
        self.assertRaises(IndexError, t.add_sequence, '(ADE')
        self.assertEqual(t.get_number_of_segments(), 0)
        t = IMP.atom.CHARMMTopology(ff)
        self.assertRaises(IndexError, t.add_sequence, '(ADE/')
        self.assertEqual(t.get_number_of_segments(), 0)

    def test_segments_python_list(self):
        """Test the Python list-like CHARMMTopology.segments member"""
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        t = IMP.atom.CHARMMTopology(ff)
        t.add_sequence('C/C/C/C/C/C/C/C')
        self.assertEqual(len(t.segments), 8)
        r = repr(t.segments)
        self.assertEqual(t.segments[6], t.segments[-2])
        self.assertEqual(t.segments[1:10:4], [t.segments[1], t.segments[5]])
        del t.segments[2::2]

        st = IMP.atom.CHARMMSegmentTopology()
        st_not_in = IMP.atom.CHARMMSegmentTopology()
        t.segments.append(st)
        self.assertIn(st, t.segments)
        self.assertNotIn(st_not_in, t.segments)
        self.assertEqual(t.segments.index(st), 5)
        self.assertRaises(ValueError, t.segments.index, st, start=6)
        self.assertRaises(ValueError, t.segments.index, st, start=0, stop=0)
        self.assertRaises(ValueError, t.segments.index, st_not_in)
        t.segments.remove(st)
        self.assertRaises(ValueError, t.segments.index, st)
        self.assertRaises(ValueError, t.segments.remove, st_not_in)
        t.segments.append(st)
        self.assertEqual(len(t.segments), 6)
        del t.segments[3]
        del t.segments[-1]
        self.assertEqual(len(t.segments), 4)
        name = t.segments[3].get_name()
        s = t.segments.pop()
        self.assertEqual(s.get_name(), name)
        self.assertEqual(len(t.segments), 3)
        def _delfunc():
            del t.segments[42]
        self.assertRaises(IndexError, _delfunc)

        del t.segments
        self.assertRaises(IndexError, t.segments.pop)
        t.segments = [st]
        self.assertEqual(len(t.segments), 1)
        t.segments.clear()
        self.assertEqual(len(t.segments), 0)
        t.segments.extend([st])
        self.assertEqual(len(t.segments), 1)

        self.assertRaises(IndexError, lambda: t.segments[42])
        self.assertRaises(IndexError, lambda: t.segments[-42])

    def test_charmm_atom_topology_pickle(self):
        """Test (un-)pickle of CHARMMAtomTopology"""
        a1 = IMP.atom.CHARMMAtomTopology("name1")
        a1.set_charmm_type("ct1")
        a1.set_charge(1.0)
        a2 = IMP.atom.CHARMMAtomTopology("name2")
        a2.set_charmm_type("ct2")
        a2.set_charge(2.0)
        a2.foo = 'bar'
        dump = pickle.dumps((a1, a2))

        newa1, newa2 = pickle.loads(dump)
        self.assertEqual(a1.get_name(), newa1.get_name())
        self.assertEqual(a1.get_charmm_type(), newa1.get_charmm_type())
        self.assertAlmostEqual(a1.get_charge(), newa1.get_charge(), delta=1e-4)
        self.assertEqual(a2.get_name(), newa2.get_name())
        self.assertEqual(a2.get_charmm_type(), newa2.get_charmm_type())
        self.assertAlmostEqual(a2.get_charge(), newa2.get_charge(), delta=1e-4)
        self.assertEqual(newa2.foo, 'bar')

        self.assertRaises(TypeError, a1._set_from_binary, 42)

    def test_charmm_bond_parameters_pickle(self):
        """Test (un-)pickle of CHARMMBondParameters"""
        p1 = IMP.atom.CHARMMBondParameters()
        p1.force_constant = 1.0
        p1.ideal = 2.0
        p2 = IMP.atom.CHARMMBondParameters()
        p2.force_constant = 3.0
        p2.ideal = 4.0
        p2.foo = 'bar'
        dump = pickle.dumps((p1, p2))

        newp1, newp2 = pickle.loads(dump)
        self.assertAlmostEqual(p1.force_constant, newp1.force_constant,
                               delta=1e-4)
        self.assertAlmostEqual(p1.ideal, newp1.ideal, delta=1e-4)
        self.assertAlmostEqual(p2.force_constant, newp2.force_constant,
                               delta=1e-4)
        self.assertAlmostEqual(p2.ideal, newp2.ideal, delta=1e-4)
        self.assertEqual(newp2.foo, 'bar')

        self.assertRaises(TypeError, p1._set_from_binary, 42)

    def test_charmm_dihedral_parameters_pickle(self):
        """Test (un-)pickle of CHARMMDihedralParameters"""
        p1 = IMP.atom.CHARMMDihedralParameters()
        p1.force_constant = 1.0
        p1.multiplicity = 2
        p1.ideal = 3.0
        p2 = IMP.atom.CHARMMDihedralParameters()
        p2.force_constant = 4.0
        p2.multiplicity = 5
        p2.ideal = 6.0
        p2.foo = 'bar'
        dump = pickle.dumps((p1, p2))

        newp1, newp2 = pickle.loads(dump)
        self.assertAlmostEqual(p1.force_constant, newp1.force_constant,
                               delta=1e-4)
        self.assertEqual(p1.multiplicity, newp1.multiplicity)
        self.assertAlmostEqual(p1.ideal, newp1.ideal, delta=1e-4)
        self.assertAlmostEqual(p2.force_constant, newp2.force_constant,
                               delta=1e-4)
        self.assertEqual(p2.multiplicity, newp2.multiplicity)
        self.assertAlmostEqual(p2.ideal, newp2.ideal, delta=1e-4)
        self.assertEqual(newp2.foo, 'bar')

        self.assertRaises(TypeError, p1._set_from_binary, 42)


if __name__ == '__main__':
    IMP.test.main()
