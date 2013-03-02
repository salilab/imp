import IMP
import IMP.test
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Test eigen analysis"""

    def _produce_point_set(self):
        vs= []
        vs.append(IMP.algebra.Vector3D(2.5,2.4,0.0))
        vs.append(IMP.algebra.Vector3D(0.5,0.7,0.0))
        vs.append(IMP.algebra.Vector3D(2.2,2.9,0.0))
        vs.append(IMP.algebra.Vector3D(1.9,2.2,0.0))
        vs.append(IMP.algebra.Vector3D(3.1,3.0,0.0))
        vs.append(IMP.algebra.Vector3D(2.3,2.7,0.0))
        vs.append(IMP.algebra.Vector3D(2.0,1.6,0.0))
        vs.append(IMP.algebra.Vector3D(1.0,1.1,0.0))
        vs.append(IMP.algebra.Vector3D(1.5,1.6,0.0))
        vs.append(IMP.algebra.Vector3D(1.1,0.9,0.0))

        return vs

    def test_pca(self):
        """Testing eigen analysis with known point set"""
        vs = self._produce_point_set()
        ed = IMP.algebra.get_principal_components(vs)
        self.assertAlmostEqual(ed.get_principal_value(0),1.28, delta=.1)
        self.assertAlmostEqual(ed.get_principal_value(1),0.049, delta=.1)
        self.assertAlmostEqual(ed.get_principal_component(0)[0],0.677873, delta=.01)
        self.assertAlmostEqual(ed.get_principal_component(0)[1],0.735179, delta=.01)
        self.assertAlmostEqual(ed.get_principal_component(1)[0],-0.735179, delta=.01)
        self.assertAlmostEqual(ed.get_principal_component(1)[1],0.677873, delta=.01)
        ed.show()

    def test_pca_from_random_points(self):
        """Testing eigen analysis with random points"""
        vs = []
        bb= IMP.algebra.BoundingBox3D(
              IMP.algebra.Vector3D(-100.,-100.,-100.),
              IMP.algebra.Vector3D(100.,100.,100.))
        for i in range(50):
            vs.append(IMP.algebra.get_random_vector_in(bb))
        vs_mean=IMP.algebra.Vector3D(0.,0.,0.)
        for i in range(50):
            vs_mean = vs_mean+vs[i]
        vs_mean = vs_mean/50

        ed = IMP.algebra.get_principal_components(vs)
        self.assertGreater(ed.get_principal_value(0), ed.get_principal_value(1))
        self.assertGreater(ed.get_principal_value(1), ed.get_principal_value(2))
        self.assertAlmostEqual(IMP.algebra.get_distance(vs_mean,ed.get_centroid()),0., delta=.1)


    def test_that_pca_is_invariant_to_rotation(self):
        """Testing that the eigen values are invariant to rotation """
        vs = []
        bb= IMP.algebra.BoundingBox3D(
              IMP.algebra.Vector3D(-100.,-100.,-100.),
              IMP.algebra.Vector3D(100.,100.,100.))
        for i in range(50):
            vs.append(IMP.algebra.get_random_vector_in(bb))
        vs_mean=IMP.algebra.Vector3D(0.,0.,0.)
        for i in range(50):
            vs_mean = vs_mean+vs[i]
        vs_mean = vs_mean/50
        ed1 = IMP.algebra.get_principal_components(vs)
        rand_rot = IMP.algebra.get_random_rotation_3d()
        rand_trans=IMP.algebra.Transformation3D(rand_rot,IMP.algebra.Vector3D(0.,0.,0.))
        for i in range(50):
            vs[i] = rand_trans.get_transformed(vs[i])
        ed2 = IMP.algebra.get_principal_components(vs)
        self.assertAlmostEqual(ed1.get_principal_value(0),ed2.get_principal_value(0), delta=0.2)
        self.assertAlmostEqual(ed1.get_principal_value(1),ed2.get_principal_value(1), delta=0.2)
        self.assertAlmostEqual(ed1.get_principal_value(2),ed2.get_principal_value(2), delta=0.2)

if __name__ == '__main__':
    IMP.test.main()
