import IMP
import IMP.test
if IMP.IMP_KERNEL_HAS_NUMPY:
    import numpy
    import numpy.testing


xkey = IMP.FloatKey("x")
ykey = IMP.FloatKey("y")
zkey = IMP.FloatKey("z")
idkey = IMP.IntKey("id")
radkey = IMP.FloatKey("radius")


class IntLike(object):
    def __init__(self, val):
        self.val = val
    def __int__(self):
        return self.val
    def __index__(self):
        return self.val


class Tests(IMP.test.TestCase):

    """Test particles"""

    def test_no_model(self):
        """Check access of attributes from python"""
        m = IMP.Model()
        p = IMP.Particle(m)
        ik = IMP.IntKey("hi")
        m.add_attribute(ik, p.get_index(), 1)
        self.assertEqual(m.get_attribute(ik, p.get_index()), 1)
        pisk = IMP.ParticleIndexesKey("hi")
        m.add_attribute(pisk, p.get_index(), [p.get_index()])
        self.assertEqual(m.get_attribute(pisk, p.get_index()), [p.get_index()])
        pik = IMP.ParticleIndexKey("hi")
        m.add_attribute(pik, p.get_index(), p.get_index())
        self.assertEqual(m.get_attribute(pik, p.get_index()), p.get_index())

    def test_intlike(self):
        """Test int-like objects as attributes"""
        m = IMP.Model()
        p = IMP.Particle(m)
        ik = IMP.IntKey("hi")
        isk = IMP.IntsKey("hi")
        m.add_attribute(ik, p.get_index(), IntLike(1))
        self.assertEqual(m.get_attribute(ik, p.get_index()), 1)
        m.add_attribute(isk, p.get_index(), [IntLike(9)])
        self.assertEqual(m.get_attribute(isk, p.get_index()), [9])

    @IMP.test.skipIf(not IMP.IMP_KERNEL_HAS_NUMPY, "No numpy support")
    def test_numpy_int(self):
        """Test using numpy arrays as input/output for Ints attributes"""
        m = IMP.Model()
        p = IMP.Particle(m)
        isk = IMP.IntsKey("hi")
        m.add_attribute(isk, p.get_index(), [1])

        # 32-bit int should be copied directly from numpy to IMP; 64-bit should
        # use the normal per-element typemaps
        for dtype in (numpy.int32, numpy.int64):
            n = numpy.array([1,2,3,4,5,6], dtype=dtype)
            p.set_value(isk, n)
            val = p.get_value(isk)
            self.assertIsInstance(val, numpy.ndarray)
            numpy.testing.assert_array_equal(val, [1,2,3,4,5,6])

    @IMP.test.skipIf(not IMP.IMP_KERNEL_HAS_NUMPY, "No numpy support")
    def test_numpy_float(self):
        """Test using numpy arrays as input/output for Floats attributes"""
        m = IMP.Model()
        p = IMP.Particle(m)
        fsk = IMP.FloatsKey("hi")
        m.add_attribute(fsk, p.get_index(), [1.])

        # double should be copied directly from numpy to IMP; float should
        # use the normal per-element typemaps
        for dtype in (numpy.float32, numpy.float64):
            n = numpy.array([1.,2.,3.,4.,5.,6.], dtype=dtype)
            p.set_value(fsk, n)
            val = p.get_value(fsk)
            self.assertIsInstance(val, numpy.ndarray)
            numpy.testing.assert_allclose(val, [1.,2.,3.,4.,5.,6.])


if __name__ == '__main__':
    IMP.test.main()
