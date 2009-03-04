import unittest
import IMP.utils
import IMP.core
import IMP.algebra
import IMP.test, IMP

class ParticleTrasnformationTests(IMP.test.TestCase):
    """Test distance restraint and three harmonic score functions"""

    def setUp(self):
        """Set up model and particles"""
        IMP.test.TestCase.setUp(self)

        self.imp_model = IMP.Model()
        self.particles = IMP.Particles()
        self.rsrs = []

        pp=IMP.Particle(self.imp_model)
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         109.882,-50.414,41.302))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         91.965,  42.942,  28.665))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         78.956,58.224,75.592))
        self.particles.append(self.create_point_particle(self.imp_model,
                                                         82.603,46.874,76.53))
    def test_transformation(self):
        """Test the TransformationFunction class"""
        r = IMP.algebra.rotation_from_fixed_xyz(0.2,0.8,-0.4)
        t=IMP.algebra.Transformation3D(r,IMP.algebra.Vector3D(20.0,-12.4,18.6))
        tf=IMP.core.Transform(t)
        for p in self.particles:
            r = tf.apply(p)
        tp=[]
        tp.append(IMP.algebra.Vector3D(47.948, -86.769, 118.648))
        tp.append(IMP.algebra.Vector3D(68.987, 6.399, 110.088))
        tp.append(IMP.algebra.Vector3D(30.447,  28.832, 134.914))
        tp.append(IMP.algebra.Vector3D(29.265,  17.052, 136.600))
        for i in xrange(4):
            xyz = IMP.core.XYZDecorator.cast(self.particles[i]);
            v = xyz.get_coordinates();
            self.assertEqual((v-tp[i]).get_magnitude()<0.01,True)

if __name__ == '__main__':
    unittest.main()
