import unittest
import IMP
import IMP.test

fk0= IMP.FloatKey("f0");
fk1= IMP.FloatKey("f1");
fk2= IMP.FloatKey("f2");
ik0= IMP.IntKey("i0");
ik1= IMP.IntKey("i1");
ik2= IMP.IntKey("i2");
sk0= IMP.StringKey("s0");
sk1= IMP.StringKey("s1");
sk2= IMP.StringKey("s2");
pk0= IMP.ParticleKey("p0");
pk1= IMP.ParticleKey("p1");
pk2= IMP.ParticleKey("p2");

class ParticleTests(IMP.test.TestCase):
    """Test particles"""
    def _add_attributes(self, p, n, op):
        p.add_attribute(fk0, n, True)
        p.add_attribute(fk1, n+1, True)
        p.add_attribute(ik0, 100*n)
        p.add_attribute(ik1, 100*n+1)
        p.add_attribute(sk0, str(100*n))
        p.add_attribute(sk1, str(100*n+1))
        p.add_attribute(pk0, p)
        p.add_attribute(pk1, op)
    def _test_base(self, p, n, op):
        self.assertEqual(p.get_value(fk0), n)
        self.assertEqual(p.get_value(fk1), n+1)
        self.assertEqual(p.get_value(ik0), 100*n)
        self.assertEqual(p.get_value(ik1), 100*n+1)
        self.assertEqual(p.get_value(sk0), str(100*n))
        self.assertEqual(p.get_value(sk1), str(100*n+1))
        self.assertEqual(p.get_value(pk0), p)
        self.assertEqual(p.get_value(pk1), op)
    def _set_attributes(self, p, n, op):
        p.set_value(fk0, -3)
        if p.has_attribute(fk1):
            p.remove_attribute(fk1)
        if !p.has_attribute(fk2):
            p.add_attribute(fk2, 666)
        p.set_value(ik0, -4)
        p.remove_attribute(ik1)
        p.add_attribute(ik2, -5)
        p.set_value(sk0, "hi")
        p.remove_attribute(sk1)
        p.add_attribute(sk2, "there")
        p.set_value(pk0, p.get_value(pk1))
        p.remove_attribute(pk1)
        p.add_attribute(pk2, p)
    def _test_set(self, p, n, op):
        self.assertEqual(p.get_value(fk0), n)
        self.assertEqual(p.get_value(fk1), n+1)
        self.assertEqual(p.get_value(ik0), 100*n)
        self.assertEqual(p.get_value(ik1), 100*n+1)
        self.assertEqual(p.get_value(sk0), str(100*n))
        self.assertEqual(p.get_value(sk1), str(100*n+1))
        self.assertEqual(p.get_value(pk0), p)
        self.assertEqual(p.get_value(pk1), op)
    def _scramble(self, p):
        p.set_value(fk0, -3)
        if p.has_attribute(fk1):
            p.remove_attribute(fk1)
        if !p.has_attribute(fk2):
            p.add_attribute(fk2, 666)
        p.set_value(ik0, -4)
        if p.has_attribute(ik1):
            p.remove_attribute(ik1)
        if !p.has_attribute(ik2):
            p.add_attribute(ik2, -5)
        p.set_value(sk0, "hi")
        if p.has_attribute(sk1):
            p.remove_attribute(sk1)
        if !p.has_attribute(sk2):
            p.add_attribute(sk2, "there")
        p.set_value(pk0, p.get_value(pk1))
        if p.has_attribute(pk1):
            p.remove_attribute(pk1)
        if !p.has_attribute(pk2):
            p.add_attribute(pk2, p)
    def _make_things(self):
        m= IMP.Model()
        ps=[IMP.Particle(m), IMP.Particle(m), IMP.Particle(m)]
        self._add_attributes(ps[0], 0, ps[1])
        self._add_attributes(ps[1], 1, ps[2])
        self._add_attributes(ps[2], 2, ps[0])
        m.remove_particle(ps[2])
        return (m, ps)
    def test_noop(self):
        """Testing no-ops with the ConfigurationSet"""
        (m, ps)= self._make_things()
        cs= IMP.ConfigurationSet(m)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[2])
        #no-op
        cs.set_configuration(-1)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[2])
    def test_scramble(self):
        """Testing restore with ConfigurationSet"""
        (m, ps)= self._make_things()
        cs= IMP.ConfigurationSet(m)
        #self._test_base(ps[2], 2, ps[0])
        self._scramble(ps[0])
        self._scramble(ps[1])
        cs.set_configuration(-1)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[2])
    def test_diff(self):
        """Testing restore a know state with ConfigurationSet"""
        (m, ps)= self._make_things()
        cs= IMP.ConfigurationSet(m)
        #self._test_base(ps[2], 2, ps[0])
        self._set_attributes(ps[0], 0, ps[2])
        self._set_attributes(ps[1], 1, ps[0])
        cs.save_configuration()
        self._scramble(ps[0])
        self._scramble(ps[1])
        cs.set_configuration(0)
        self._test_set(ps[0], 0, ps[2])
        self._test_set(ps[1], 1, ps[0])
        cs.set_configuration(-1)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[2])

if __name__ == '__main__':
    unittest.main()
