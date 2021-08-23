import io
import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_one(self):
        """Test that writing hierarchies to pdb as cas works"""
        m = IMP.Model()
        with self.open_input_file("input.pdb") as fh:
            h = IMP.atom.read_pdb(fh, m)
        hs = IMP.atom.create_simplified_along_backbone(h, 1)
        o = io.BytesIO()
        IMP.atom.write_pdb_of_c_alphas(hs, o)
        back = IMP.atom.read_pdb(io.BytesIO(o.getvalue()), m)
        for (o, b) in zip(IMP.atom.get_leaves(hs), IMP.atom.get_leaves(back)):
            co = IMP.core.XYZ(o).get_coordinates()
            cb = IMP.core.XYZ(b).get_coordinates()
            self.assert_(IMP.algebra.get_distance(co, cb) < .1)

if __name__ == '__main__':
    IMP.test.main()
