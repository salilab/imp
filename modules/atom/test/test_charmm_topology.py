from __future__ import print_function
import IMP
import IMP.test
import IMP.atom
import string


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
        t.segments.remove(st)
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


if __name__ == '__main__':
    IMP.test.main()
