import IMP
import IMP.test
import IMP.algebra
import IMP.core

class Tests(IMP.test.TestCase):
    """Test rigid transformations"""

    def _produce_point_sets(self, tr):
        vs= []
        vsr= []
        for i in range(0, 20):
            vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d()))
            print vs[-1]
            vsr.append(tr.get_transformed(vs[-1]))
            print vsr[-1]
        return (vs, vsr)
    def _convert_to_xyzs(self, m, vs):
        ret=[]
        for v in vs:
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p, v)
            ret.append(d)
        return ret

    def test_align(self):
        """Testing rigid alignment of xyz sets"""
        m=IMP.Model()
        r= IMP.algebra.get_random_rotation_3d()
        t= IMP.algebra.Vector3D(0,0,0) #IMP.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        tr= IMP.algebra.Transformation3D(r, t)
        (vs, vsr)= self._produce_point_sets(tr)
        (ds, dsr)= (self._convert_to_xyzs(m, vs), self._convert_to_xyzs(m,vsr))
        #tr0= IMP.algebra.get_transformation_aligning_first_to_second(vs, vsr)
        #print "tr", tr
        tr1= IMP.core.get_transformation_aligning_first_to_second(ds, dsr)
        print "tr1", tr1
        tr2= IMP.core.get_transformation_aligning_first_to_second(vs, dsr)
        print "tr2", tr2
        self.assertAlmostEqual(IMP.algebra.get_distance(tr.get_rotation(), r),
                               0, delta=.1)


if __name__ == '__main__':
    IMP.test.main()
