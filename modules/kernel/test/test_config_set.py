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
pk0= IMP.ParticleIndexKey("p0");
pk1= IMP.ParticleIndexKey("p1");
pk2= IMP.ParticleIndexKey("p2");

class Tests(IMP.test.TestCase):
    """Test particles"""
    def _force_set(self, p, k, v):
        if p.has_attribute(k):
            p.set_value(k, v)
        else:
            p.add_attribute(k,v)
    def _force_remove(self, p, k):
        if p.has_attribute(k):
            p.remove_attribute(k)
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
        print "testing"
        self.assertEqual(p.get_value(fk0), n)
        self.assertEqual(p.get_value(fk1), n+1)
        self.assertEqual(p.get_value(ik0), 100*n)
        self.assertEqual(p.get_value(ik1), 100*n+1)
        self.assertEqual(p.get_value(sk0), str(100*n))
        self.assertEqual(p.get_value(sk1), str(100*n+1))
        self.assertEqual(p.get_value(pk0), p)
        self.assertEqual(p.get_value(pk1), op)
    def _set_attributes(self, p, n, op):
        p.set_value(fk1, n*3)
        p.remove_attribute(fk0)
        p.add_attribute(fk2, n*7)
        p.set_value(ik1, -4*n)
        p.remove_attribute(ik0)
        p.add_attribute(ik2, n*7+2)
        p.set_value(sk1, "setit")
        p.remove_attribute(sk0)
        p.add_attribute(sk2, "alreadythere")
        p.set_value(pk1, p)
        p.remove_attribute(pk0)
        p.add_attribute(pk2, op)
    def _test_set(self, p, n, op):
        self.assertEqual(p.get_value(fk1), n*3)
        self.assertFalse(p.has_attribute(fk0))
        self.assertEqual(p.get_value(fk2), n*7)
        self.assertEqual(p.get_value(ik1), -4*n)
        self.assertFalse(p.has_attribute(ik0))
        self.assertEqual(p.get_value(ik2), n*7+2)
        self.assertEqual(p.get_value(sk1), "setit")
        self.assertFalse(p.has_attribute(sk0))
        self.assertEqual(p.get_value(sk2), "alreadythere")
        self.assertEqual(p.get_value(pk1), p)
        self.assertFalse(p.has_attribute(pk0))
        self.assertEqual(p.get_value(pk2), op)
    def _scramble(self, p):
        self._force_set(p, fk0, -3)
        self._force_remove(p, fk1)
        self._force_set(p, fk2, 666)
        self._force_set(p, ik0, -4)
        self._force_remove(p, ik1)
        self._force_set(p, ik2, -5)
        self._force_set(p,sk0, "hi")
        self._force_remove(p, sk1)
        self._force_set(p, sk2, "there")
        self._force_set(p, pk0, p)
        self._force_remove(p, pk1)
        self._force_set(p, pk2, p)
    def _make_things(self):
        IMP.base.set_log_level(IMP.MEMORY)
        print "starting"
        m= IMP.Model("config set")
        print "adding"
        ps=[IMP.Particle(m), IMP.Particle(m), IMP.Particle(m)]
        print "adding attribute"
        self._add_attributes(ps[0], 0, ps[1])
        self._add_attributes(ps[1], 1, ps[1])
        self._add_attributes(ps[2], 2, ps[0])
        print "removing"
        m.remove_particle(ps[2])
        print "returning"
        return (m, ps)
    def test_noop(self):
        """Testing no-ops with the ConfigurationSet"""
        (m, ps)= self._make_things()
        print "create"
        cs= IMP.ConfigurationSet(m)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[1])
        #no-op
        print "load"
        cs.load_configuration(-1)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[1])
    def test_scramble(self):
        """Testing restore with ConfigurationSet"""
        (m, ps)= self._make_things()
        cs= IMP.ConfigurationSet(m)
        #self._test_base(ps[2], 2, ps[0])
        self._scramble(ps[0])
        self._scramble(ps[1])
        cs.load_configuration(-1)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[1])
    def test_diff(self):
        """Testing restore a known state with ConfigurationSet"""
        (m, ps)= self._make_things()
        cs= IMP.ConfigurationSet(m)
        #self._test_base(ps[2], 2, ps[0])
        self._set_attributes(ps[0], 0, ps[1])
        self._set_attributes(ps[1], 1, ps[0])
        cs.save_configuration()
        self._scramble(ps[0])
        self._scramble(ps[1])
        print "load 0"
        cs.load_configuration(0)
        print "testing"
        self._test_set(ps[0], 0, ps[1])
        print "testing"
        self._test_set(ps[1], 1, ps[0])
        print "load -1"
        cs.load_configuration(-1)
        self._test_base(ps[0], 0, ps[1])
        self._test_base(ps[1], 1, ps[1])
        for p in m.get_particles():
            p.show()

if __name__ == '__main__':
    IMP.test.main()
