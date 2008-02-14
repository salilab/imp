import unittest
import IMP
import IMP.utils
import IMP.test

xkey = IMP.FloatKey("x")
ykey = IMP.FloatKey("y")
zkey = IMP.FloatKey("z")
idkey = IMP.IntKey("id")
radkey = IMP.FloatKey("radius")

class ParticleTests(IMP.test.TestCase):
    """Test particles"""

    def setUp(self):
        """Build a set of test particles"""

        self.model = IMP.Model()
        self.particles = []

        # create particles 0 - 11
        for i in range(0,12):
            self.particles.append(IMP.utils.XYZParticle(self.model,
                                                        i*2, i*3, i*4))
            p1 = self.particles[i]
            p1.add_attribute(radkey, 1.5 * i, False)
            p1.add_attribute(idkey, i)
            p1.add_attribute(IMP.IntKey("six"), 6)
            p1.add_attribute(IMP.StringKey("id_str"), "name_"+str(i))
            p1.add_attribute(IMP.StringKey("six"), "b:0110")

        # add additional attributes to particle 11
        for i in range(0,6):
            p1.add_attribute(IMP.FloatKey("attr_" + str(i)), 3.5 * i, False)

    def test_inactive(self):
        """Check that operations fail on inactivated particles"""
        p0 = self.particles[0]
        p1 = self.particles[1]
        r = IMP.DistanceRestraint(p0, p1, IMP.Harmonic(10.0, 0.1))
        self.model.add_restraint(r)
        p0.set_is_active(False)
        self.assertRaises(ValueError, p0.get_value, xkey)
        self.assertRaises(ValueError, p0.set_value, xkey, 0.0)
        self.assertRaises(ValueError, self.model.evaluate, False)
        # Making the particle active again should fix everything:
        p0.set_is_active(True)
        dummy = p0.get_value(xkey)
        p0.set_value(xkey, 0.0)
        self.model.evaluate(False)

    def test_bad_attributes(self):
        """Asking for non-existent attributes should cause an error"""
        p1 = self.particles[0]
        self.assertRaises(IndexError, p1.get_value, IMP.FloatKey("notexist"))
        self.assertRaises(IndexError, p1.get_value, IMP.IntKey("notexist"))
        self.assertRaises(IndexError, p1.get_value, IMP.StringKey("notexist"))
        self.assertRaises(IndexError, p1.add_attribute, IMP.FloatKey(), 0)

    def test_get_set_methods(self):
        """Test particle get_ and set_ methods"""
        for (i, p) in enumerate(self.particles):
            self.assertEqual(p.get_index(), IMP.ParticleIndex(i))
            model = p.get_model()
        p = self.particles[0]
        self.assertEqual(p.get_is_active(), True)
        p.set_is_active(False)
        self.assertEqual(p.get_is_active(), False)

    def test_derivatives(self):
        """Test get/set of derivatives"""
        p = self.particles[0]
        self.assertEqual(p.get_derivative(xkey), 0.0)
        da = IMP.DerivativeAccumulator()
        p.add_to_derivative(xkey, 10.0, da)
        self.assertEqual(p.get_derivative(xkey), 10.0)
        da = IMP.DerivativeAccumulator(2.0)
        p.add_to_derivative(xkey, 10.0, da)
        self.assertEqual(p.get_derivative(xkey), 30.0)

    def test_browsing(self):
        """Test browsing of particle attributes"""
        p=self.particles[0]
        ict=0
        fct=0
        sct=0
        for s in p.get_string_attributes():
            sct += 1
        for s in p.get_float_attributes():
            fct += 1
        for s in p.get_int_attributes():
            ict += 1
        self.assertEqual(ict, 2)
        self.assertEqual(fct, 4)
        self.assertEqual(sct, 2)

    def test_particles(self):
        """Test that particle attributes are available and correct"""
        for (i, p) in enumerate(self.particles):
            self.assert_(p.has_attribute(xkey))
            # A Float "x" exists; make sure that has_attribute doesn't get
            # confused between different types of attribute:
            self.assert_(not p.has_attribute(IMP.IntKey("x")))
            self.assert_(not p.has_attribute(IMP.IntKey("notexist")))
            self.assertEqual(p.get_value(xkey), i * 2)
            self.assertEqual(p.get_value(ykey), i * 3)
            self.assertEqual(p.get_value(zkey), i * 4)
            self.assertEqual(p.get_value(idkey), i)
            self.assertEqual(p.get_value(IMP.StringKey("id_str")),
                             "name_" + str(i))
            self.assertEqual(p.get_value(IMP.IntKey("six")), 6)
            self.assertEqual(p.get_value(IMP.StringKey("six")), "b:0110")

        # test additional attributes in particle 11
        p = self.particles[11]
        for i in range(0,6):
            val = p.get_value(IMP.FloatKey("attr_" + str(i)))
            self.assertEqual(val, 3.5 * i)

if __name__ == '__main__':
    unittest.main()
