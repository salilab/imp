import IMP
import IMP.test
import IMP.atom
import IMP.multifit

class Tests(IMP.test.TestCase):

    def test_refiner(self):
        """Test the RigidLeavesRefiner"""
        m = IMP.Model()
        sel = IMP.atom.CAlphaPDBSelector()
        mh = IMP.atom.read_pdb(self.get_input_file_name("1tyq_A.pdb"), m, sel)
        rb = IMP.atom.create_rigid_body(mh)
        r = IMP.multifit.RigidLeavesRefiner()
        r.set_was_used(True)
        self.assertTrue(r.get_can_refine(rb))
        self.assertFalse(r.get_can_refine(mh))
        ps = r.get_refined(rb)
        # Should only return leaves (CA atoms) from the rigid body,
        # not residues, chains
        self.assertEqual(len(ps), 399)
        self.assertEqual(ps, IMP.atom.get_leaves(mh))
        for p in ps:
            self.assertTrue(IMP.atom.Atom.get_is_setup(p))
        # Inputs, however, should compromise every particle the refiner looks
        # at, which is everything in the rigid body
        ps = r.get_inputs(m, [rb])
        self.assertEqual(len(ps), 801)

if __name__ == '__main__':
    IMP.test.main()
