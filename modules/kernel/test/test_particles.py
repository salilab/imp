import IMP
import IMP.test


xkey = IMP.FloatKey("x")
ykey = IMP.FloatKey("y")
zkey = IMP.FloatKey("z")
idkey = IMP.IntKey("id")
radkey = IMP.FloatKey("radius")

class Tests(IMP.test.TestCase):
    """Test particles"""

    def setup(self):
        """Build a set of test particles"""
        IMP.test.TestCase.setUp(self)

        model = IMP.Model("particles model")
        particles = []

        # create particles 0 - 11
        for i in range(0,12):
            particles.append(self.create_point_particle(model,
                                                             i*2, i*3, i*4))
            p1 = particles[i]
            p1.add_attribute(radkey, 1.5 * i, False)
            p1.add_attribute(idkey, i)
            p1.add_attribute(IMP.IntKey("six"), 6)
            p1.add_attribute(IMP.StringKey("id_str"), "name_"+str(i))
            p1.add_attribute(IMP.StringKey("six"), "b:0110")

        # add additional attributes to particle 11
        for i in range(0,6):
            p1.add_attribute(IMP.FloatKey("attr_" + str(i)), 3.5 * i, False)
        # clear derivatives
        print model.get_ref_count()
        model.evaluate(True)
        print model.get_ref_count()
        return (model, particles)

    def test_no_model(self):
        """Check that operations fail on particles once the model is gone"""
        refcnt = IMP.test.RefCountChecker(self)
        (model, particles)= self.setup()
        p1 = particles[0]
        self.assertEqual(p1.get_is_active(), True)
        IMP.base.set_log_level(IMP.MEMORY)
        del model
        # Particles left over after a model is deleted should act as if
        # they are inactive
        refcnt.assert_number(12)
        self.assertEqual(p1.get_is_active(), False)
        # this should be a usageexception as it is expensive
        #self.assertRaises(ValueError, p1.add_attribute, IMP.IntKey("Test"), 0)
        #self.assertRaises(ValueError, p1.get_value, xkey)
        #self.assertRaises(ValueError, p1.set_value, xkey, 0.0)

    def test_equality(self):
        """Check particle identity"""
        (model, particles)= self.setup()
        p0 = particles[0]
        p1 = particles[1]
        self.assertTrue(p0 != p1)
        self.assertTrue(p0 == p0)
        self.assertTrue(not p0 != p0)
        # Different SWIG proxies for the same underlying Particle should
        # report equality:
        [m_p0, m_p1] = model.get_particles()[:2]
        self.assertTrue(m_p0 == p0)
        self.assertTrue(not m_p0 != p0)
        # Even particles with equal attributes should not count as equal:
        p0 = self.create_point_particle(model, 0, 0, 0)
        p1 = self.create_point_particle(model, 0, 0, 0)
        self.assertTrue(p0 != p1)

    def test_hashing(self):
        """Check that particles and decorators hash in python"""
        (model, particles)= self.setup()
        p0 = particles[0]
        p1 = particles[1]
        d={}
        d[p0]=1
        d[p1]=2
        print p0.__hash__()
        td= IMP.kernel._TrivialDecorator.setup_particle(p0)
        print td.__hash__()
        print td.get_particle().__hash__()
        d[td]=3
        self.assertEqual(d[p0], 3)

    # no good reason to special case particles, just use UsageExceptions
    def _test_bad_attributes(self):
        """Asking for non-existent attributes should cause an exception"""
        p1 = particles[0]
        self.assertRaises(IndexError, p1.get_value, IMP.FloatKey("notexist"))
        self.assertRaises(IndexError, p1.get_value, IMP.IntKey("notexist"))
        self.assertRaises(IndexError, p1.get_value, IMP.StringKey("notexist"))
        self.assertRaises(IndexError, p1.add_attribute, IMP.FloatKey(), 0)

    def test_get_set_methods(self):
        """Test particle get_ and set_ methods"""
        (model, particles)= self.setup()
        for (i, p) in enumerate(particles):
            #self.assertEqual(p.get_index(), IMP.ParticleIndex(i))
            model = p.get_model()
        p = particles[0]
        self.assertEqual(p.get_is_active(), True)
        model.remove_particle(p)
        self.assertEqual(p.get_is_active(), False)

    def _test_add_remove(self, p, ak, v):
        p.add_attribute(ak, v)
        self.assertTrue(p.has_attribute(ak))
        p.remove_attribute(ak)
        self.assertFalse(p.has_attribute(ak))

    def test_remove_attributes(self):
        """Test that attributes can be removed"""
        (model, particles)= self.setup()
        p=particles[0]
        fk= IMP.FloatKey("to_remove")
        p.add_attribute(fk, 0, False)
        self.assertTrue(p.has_attribute(fk))
        self.assertFalse(p.get_is_optimized(fk))
        p.set_is_optimized(fk, True)
        self.assertTrue(p.get_is_optimized(fk))
        p.set_is_optimized(fk, False)
        self.assertFalse(p.get_is_optimized(fk))
        self._test_add_remove(p, IMP.FloatKey("something"), 1.0)
        self._test_add_remove(p, IMP.StringKey("something"), "Hello")
        self._test_add_remove(p, IMP.IntKey("something"), 1)
        self._test_add_remove(p, IMP.ParticleIndexKey("something"), p)

    def test_derivatives(self):
        """Test get/set of derivatives"""
        (model, particles)= self.setup()
        p = particles[0]
        self.assertEqual(p.get_derivative(xkey), 0.0)
        da = IMP.DerivativeAccumulator()
        p.add_to_derivative(xkey, 10.0, da)
        self.assertEqual(p.get_derivative(xkey), 10.0)
        da = IMP.DerivativeAccumulator(2.0)
        p.add_to_derivative(xkey, 10.0, da)
        self.assertEqual(p.get_derivative(xkey), 30.0)

    def test_browsing(self):
        """Test browsing of particle attributes"""
        (model, particles)= self.setup()
        p=particles[0]
        ict=0
        fct=0
        sct=0
        for s in p.get_string_keys():
            sct += 1
        for s in p.get_float_keys():
            fct += 1
        for s in p.get_int_keys():
            ict += 1
        self.assertEqual(ict, 2)
        self.assertEqual(fct, 4)
        self.assertEqual(sct, 2)

    def test_particles(self):
        """Test that particle attributes are available and correct"""
        (model, particles)= self.setup()
        for (i, p) in enumerate(particles):
            self.assertTrue(p.has_attribute(xkey))
            # A Float "x" exists; make sure that has_attribute doesn't get
            # confused between different types of attribute:
            self.assertFalse(p.has_attribute(IMP.IntKey("x")))
            self.assertFalse(p.has_attribute(IMP.IntKey("notexist")))
            self.assertEqual(p.get_value(xkey), i * 2)
            self.assertEqual(p.get_value(ykey), i * 3)
            self.assertEqual(p.get_value(zkey), i * 4)
            self.assertEqual(p.get_value(idkey), i)
            self.assertEqual(p.get_value(IMP.StringKey("id_str")),
                             "name_" + str(i))
            self.assertEqual(p.get_value(IMP.IntKey("six")), 6)
            self.assertEqual(p.get_value(IMP.StringKey("six")), "b:0110")

        # test additional attributes in particle 11
        p = particles[11]
        for i in range(0,6):
            val = p.get_value(IMP.FloatKey("attr_" + str(i)))
            self.assertEqual(val, 3.5 * i)
    def test_comparisons(self):
        """Test comparisons of particles and decorators"""
        (model, particles)= self.setup()
        p0a= particles[0]
        p0b= model.get_particles()[0]
        self.assertEqual(p0a, p0b)
        td0a= IMP.kernel._TrivialDecorator.setup_particle(p0a)
        td0b= IMP.kernel._TrivialDecorator(p0b)
        self.assertEqual(td0a, td0b)
        self.assertEqual(td0a, p0a)
    def test_many_particle(self):
        """Test that we can allocate many particles"""
        m= IMP.Model("many particles")
        num=20000
        for i in range(0,num):
            p= IMP.Particle(m)
            if i%10000 == 0:
                print i
        print "removing"
        for i in range(0,num):
            if i%1000==0:
                m.remove_particle(m.get_particles()[i])
            if i%10000 ==0:
                print i

if __name__ == '__main__':
    IMP.test.main()
