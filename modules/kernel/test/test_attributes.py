import IMP
import IMP.test
import sys


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


if sys.version_info[0] == 2:
    class LongLike(object):
        def __init__(self, val):
            self.val = val
        def __index__(self):
            return self.val
        def __long__(self):
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
        if sys.version_info[0] == 2:
            m.set_attribute(ik, p.get_index(), LongLike(42))
            self.assertEqual(m.get_attribute(ik, p.get_index()), 42)

        m.add_attribute(isk, p.get_index(), [IntLike(9)])
        self.assertEqual(m.get_attribute(isk, p.get_index()), [9])
        if sys.version_info[0] == 2:
            m.set_attribute(isk, p.get_index(), [LongLike(99)])
            self.assertEqual(m.get_attribute(isk, p.get_index()), [99])


if __name__ == '__main__':
    IMP.test.main()
