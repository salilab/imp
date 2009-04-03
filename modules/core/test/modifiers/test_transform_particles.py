import unittest
import IMP
import IMP.core
import IMP.algebra
import IMP.test

class ParticleTransformationTests(IMP.test.TestCase):
    """Test particle transformations"""
    def test_transformation(self):
        """Test the TransformationFunction class"""
        imp_model = IMP.Model()
        particles = IMP.core.create_xyzr_particles(imp_model, 4, 1.0);

        coords= [IMP.core.XYZDecorator(x).get_coordinates() for x in particles]
        r = IMP.algebra.rotation_from_fixed_xyz(0.2,0.8,-0.4)
        t=IMP.algebra.Transformation3D(r,IMP.algebra.Vector3D(20.0,-12.4,18.6))
        tf=IMP.core.Transform(t)
        for p in particles:
            r = tf.apply(p)
        for i in range(0,len(particles)):
            xyz = IMP.core.XYZDecorator(particles[i])
            v = xyz.get_coordinates()
            self.assertInTolerance((v-t.transform(coords[i])).get_magnitude(), 0, 0.01)

    def test_transformation2(self):
        """Test the TransformationFunction class with map"""
        imp_model = IMP.Model()
        particles = IMP.core.create_xyzr_particles(imp_model, 4, 1.0);

        coords= [IMP.core.XYZDecorator(x).get_coordinates() for x in particles]
        r = IMP.algebra.rotation_from_fixed_xyz(0.2,0.8,-0.4)
        t=IMP.algebra.Transformation3D(r,IMP.algebra.Vector3D(20.0,-12.4,18.6))
        tf=IMP.core.Transform(t)
        map( IMP.SingletonFunctor(tf), particles)
        for i in range(0,len(particles)):
            xyz = IMP.core.XYZDecorator(particles[i])
            v = xyz.get_coordinates()
            self.assertInTolerance((v-t.transform(coords[i])).get_magnitude(), 0, 0.01)

if __name__ == '__main__':
    unittest.main()
