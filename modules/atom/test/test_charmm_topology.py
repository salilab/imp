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


if __name__ == '__main__':
    IMP.test.main()
