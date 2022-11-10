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

        # Invalid one-letter codes should cause an exception and add no
        # segments
        t = IMP.atom.CHARMMTopology(ff)
        self.assertRaises(ValueError, t.add_sequence, 'a')
        self.assertEqual(t.get_number_of_segments(), 0)


if __name__ == '__main__':
    IMP.test.main()
