import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):

    def add_residues(self, model, parent, num):
        for i in range(num):
            r = IMP.atom.Residue.setup_particle(IMP.kernel.Particle(model),
                                                IMP.atom.ALA)
            parent.add_child(r)

    def setup_hierarchy(self):
        m = IMP.kernel.Model()
        ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
        h = IMP.atom.Hierarchy.setup_particle(IMP.kernel.Particle(m))
        return m, ff, h

    def make_fragment(self, m, indexes):
        f = IMP.atom.Fragment.setup_particle(IMP.kernel.Particle(m))
        self.add_residues(m, f, len(indexes))
        f.set_residue_indexes(indexes)
        return f

    def get_num_res(self, t):
        return [s.get_number_of_residues() for s in t.get_segments()]

    def test_connected_fragments(self):
        """Test that create_topology works with connected fragments"""
        m, ff, h = self.setup_hierarchy()
        h.add_child(self.make_fragment(m, [0,1,2]))
        h.add_child(self.make_fragment(m, [3,4]))
        t = ff.create_topology(h)
        self.assertEqual(self.get_num_res(t), [5])

    def test_disconnected_fragments(self):
        """Test that create_topology works with disconnected fragments"""
        m, ff, h = self.setup_hierarchy()
        h.add_child(self.make_fragment(m, [0,1,2]))
        h.add_child(self.make_fragment(m, [4,5]))
        t = ff.create_topology(h)
        self.assertEqual(self.get_num_res(t), [3, 2])

    def test_mix_fragments_residues(self):
        "Test that create_topology works with a mix of fragments and residues"
        m, ff, h = self.setup_hierarchy()
        h.add_child(self.make_fragment(m, [0,1,2]))
        self.add_residues(m, h, 4)
        h.add_child(self.make_fragment(m, [3,4]))
        t = ff.create_topology(h)
        self.assertEqual(self.get_num_res(t), [3, 4, 2])

    def test_chains(self):
        "Test that create_topology works with particles in chains"
        m, ff, h = self.setup_hierarchy()
        c1 = IMP.atom.Chain.setup_particle(IMP.Particle(m), "A")
        h.add_child(c1)
        c1.add_child(self.make_fragment(m, [0,1,2]))
        c2 = IMP.atom.Chain.setup_particle(IMP.Particle(m), "B")
        h.add_child(c2)
        c2.add_child(self.make_fragment(m, [3,4]))
        t = ff.create_topology(h)
        self.assertEqual(self.get_num_res(t), [3, 2])

    def test_mix_chains_fragments(self):
        "Test that create_topology works with a mix of chains and fragments"
        m, ff, h = self.setup_hierarchy()
        c1 = IMP.atom.Chain.setup_particle(IMP.Particle(m), "A")
        h.add_child(c1)
        c1.add_child(self.make_fragment(m, [0,1,2]))
        h.add_child(self.make_fragment(m, [3,4]))
        t = ff.create_topology(h)
        self.assertEqual(self.get_num_res(t), [3, 2])

if __name__ == '__main__':
    IMP.test.main()
