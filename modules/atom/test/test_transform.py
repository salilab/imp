import IMP
import IMP.test
import IMP.core
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_bonded(self):
        """Check close and destroy Hierarchy """
        m = IMP.kernel.Model()
        rt = IMP.atom.Hierarchy.setup_particle(m, m.add_particle("root"))
        mh = IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        rt.add_child(mh)
        mh2 = IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        rb2 = IMP.atom.create_rigid_body(mh2)
        rt.add_child(mh2)
        mh3 = IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        IMP.atom.setup_as_rigid_body(mh3)
        rt.add_child(mh3)

        leaves = IMP.atom.get_leaves(rt)
        coords = [IMP.core.XYZ(l).get_coordinates() for l in leaves]
        tr = IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                          IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_sphere_3d()))
        IMP.atom.transform(rt, tr)
        m.update()
        for c, l in zip(coords, leaves):
            self.assertAlmostEqual(
                IMP.algebra.get_distance(tr.get_transformed(c),
                                         IMP.core.XYZ(l).get_coordinates()),
                0,
                delta=.1)


if __name__ == '__main__':
    IMP.test.main()
