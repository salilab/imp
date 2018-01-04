from __future__ import print_function
import IMP.test
import IMP.mmcif

class Tests(IMP.test.TestCase):
    def make_model(self, system, chains=None):
        if chains is None:
            chains = (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                      ('baz', 'ACC', 'C'))
        s = IMP.mmcif.State(system)
        m = s.model
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        for name, seq, cid in chains:
            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            mol = IMP.atom.Molecule.setup_particle(h)
            mol.set_name(name)
            top.add_child(mol)

            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            chain = IMP.atom.Chain.setup_particle(h, cid)
            chain.set_sequence(seq)
            mol.add_child(chain)
        return top, s

    def test_no_chains(self):
        """Trying to add a Hierarchy with no chains should give an error"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, chains=[])
        e = IMP.mmcif.Ensemble(state, "cluster 1")
        self.assertRaises(ValueError, e.add_model, [h], [], "model1")

    def test_remove_duplicates_noop(self):
        """Test remove_duplicate_chain_ids, noop"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, chains=[("foo", "AMT", "X"),
                                                   ("bar", "ACC", "Z")])
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        state._remove_duplicate_chain_ids(True)
        self.assertEqual(self._get_chain_ids(h), ["X", "Z"])

    def test_remove_duplicates(self):
        """Test remove_duplicate_chain_ids"""
        system = IMP.mmcif.System()
        h, state = self.make_model(system, chains=[("foo", "AMT", "X"),
                                                   ("bar", "ACC", "X")])
        state.hiers = [h]
        self.assertEqual(self._get_chain_ids(h), ["X", "X"])
        state._remove_duplicate_chain_ids(True)
        self.assertEqual(self._get_chain_ids(h), ["A", "B"])
        state._remove_duplicate_chain_ids(False)
        self.assertEqual(self._get_chain_ids(h), ["A", "B"])

    def _get_chain_ids(self, hier):
        return [IMP.atom.Chain(c).get_id()
                for c in IMP.atom.get_by_type(hier, IMP.atom.CHAIN_TYPE)]

    def test_get_alpha_chain_ids(self):
        """Test _get_alpha_chain_ids()"""
        system = IMP.mmcif.System()
        state = IMP.mmcif.State(system)
        # Get first 100 entries
        cids = []
        for i, cid in zip(range(100), state._get_alpha_chain_ids()):
            cids.append(cid)
        self.assertEqual(cids[0:4], ['A', 'B', 'C', 'D'])
        self.assertEqual(cids[24:28], ['Y', 'Z', 'AA', 'AB'])
        self.assertEqual(cids[50:54], ['AY', 'AZ', 'BA', 'BB'])

if __name__ == '__main__':
    IMP.test.main()
