import IMP
import IMP.test
import IMP.atom
import IMP.core


class Tests(IMP.test.TestCase):

    def _produce_point_sets(self, tr):
        vs = []
        vsr = []
        for i in range(0, 20):
            vs.append(IMP.algebra.get_random_vector_in(
                IMP.algebra.get_unit_bounding_box_3d()))
            vsr.append(tr.get_transformed(vs[-1]))
        return (vs, vsr)

    def test_alignment_selection(self):
        """Testing rigid alignment of point sets"""
        m=IMP.Model()
        r = IMP.algebra.get_random_rotation_3d()
        t = IMP.algebra.get_random_vector_in(
            IMP.algebra.get_unit_bounding_box_3d())
        tr = IMP.algebra.Transformation3D(r, t)
        (vs, vsr) = self._produce_point_sets(tr)

        hroot1=IMP.atom.Hierarchy(IMP.Particle(m))
        hroot2=IMP.atom.Hierarchy(IMP.Particle(m))

        for v in vs:
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(v)
            d.set_radius(1.0)
            IMP.atom.Mass.setup_particle(p,1.0)
            hroot1.add_child(p)

        for v in vsr:
            p=IMP.Particle(m)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates(v)
            d.set_radius(1.0)
            IMP.atom.Mass.setup_particle(p,1.0)
            hroot2.add_child(p)

        sel1=IMP.atom.Selection(hroot1)
        sel2=IMP.atom.Selection(hroot2)

        tr = IMP.atom.get_transformation_aligning_first_to_second(sel1, sel2)

        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_rotation(), r),
                               0, delta=.1)
        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_translation(),
                                                        t),
                               0, delta=.1)


if __name__ == '__main__':
    IMP.test.main()
