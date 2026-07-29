import IMP
import IMP.test

fk0 = IMP.FloatKey("f0")
fk1 = IMP.FloatKey("f1")
fk2 = IMP.FloatKey("f2")
fsk0 = IMP.FloatsKey("fs0")
ik0 = IMP.IntKey("i0")
ik1 = IMP.IntKey("i1")
ik2 = IMP.IntKey("i2")
isk0 = IMP.IntsKey("is0")
sk0 = IMP.StringKey("s0")
sk1 = IMP.StringKey("s1")
sk2 = IMP.StringKey("s2")
pk0 = IMP.ParticleIndexKey("p0")
pk1 = IMP.ParticleIndexKey("p1")
pk2 = IMP.ParticleIndexKey("p2")
v30 = IMP.Vector3DKey("v30")
v40 = IMP.Vector4DKey("v40")
v3d0 = IMP.Vector3DDerivKey("v3d0")
v4d0 = IMP.Vector4DDerivKey("v4d0")

sp_sk0 = IMP.SparseStringKey("sp_s0")
sp_ik0 = IMP.SparseIntKey("sp_i0")
sp_fk0 = IMP.SparseFloatKey("sp_f0")
sp_pk0 = IMP.SparseParticleIndexKey("p0")


class Tests(IMP.test.TestCase):

    """Test particles"""

    def _force_set(self, p, k, v):
        if p.has_attribute(k):
            p.set_value(k, v)
        else:
            p.add_attribute(k, v)

    def _force_remove(self, p, k):
        if p.has_attribute(k):
            p.remove_attribute(k)

    def _add_attributes(self, m, p, n, op):
        p.add_attribute(fk0, n, True)
        p.add_attribute(sp_fk0, n + 10)
        p.add_attribute(fk1, n + 1, True)
        p.add_attribute(fsk0, [n, 2.0 * n, 3.0 * n])
        p.add_attribute(ik0, 100 * n)
        p.add_attribute(sp_ik0, 100 * n + 10)
        p.add_attribute(isk0, [1 * n, 2 * n, 3 * n])
        p.add_attribute(ik1, 100 * n + 1)
        p.add_attribute(sk0, str(100 * n))
        p.add_attribute(sp_sk0, str(100 * n + 10))
        p.add_attribute(sk1, str(100 * n + 1))
        p.add_attribute(pk0, p)
        p.add_attribute(sp_pk0, p)
        p.add_attribute(pk1, op)
        m.add_attribute(v30, p, IMP.Vector3D(n, 2.0 * n, 3.0 * n))
        m.add_attribute(v3d0, p, IMP.Vector3D(n, 2.0 * n, 3.0 * n))
        m.add_attribute(v40, p, IMP.Vector4D(n, 2.0 * n, 3.0 * n, 4.0 * n))
        m.add_attribute(v4d0, p, IMP.Vector4D(n, 2.0 * n, 3.0 * n, 4.0 * n))

    def _test_base(self, m, p, n, op):
        print("testing")
        self.assertEqual(p.get_value(fk0), n)
        self.assertAlmostEqual(p.get_value(sp_fk0), n + 10, delta=1e-4)
        self.assertEqual(list(p.get_value(fsk0)), [n, 2.0 * n, 3.0 * n])
        self.assertEqual(p.get_value(fk1), n + 1)
        self.assertEqual(p.get_value(ik0), 100 * n)
        self.assertEqual(p.get_value(sp_ik0), 100 * n + 10)
        self.assertEqual(list(p.get_value(isk0)), [1 * n, 2 * n, 3 * n])
        self.assertEqual(p.get_value(ik1), 100 * n + 1)
        self.assertEqual(p.get_value(sk0), str(100 * n))
        self.assertEqual(p.get_value(sp_sk0), str(100 * n + 10))
        self.assertEqual(p.get_value(sk1), str(100 * n + 1))
        self.assertEqual(p.get_value(pk0), p)
        self.assertEqual(p.get_value(sp_pk0), p.get_index())
        self.assertEqual(p.get_value(pk1), op)
        self.assertSequenceAlmostEqual(
            list(m.get_attribute(v30, p)), [n, 2.0 * n, 3.0 * n], delta=1e-4)
        self.assertSequenceAlmostEqual(
            list(m.get_attribute(v3d0, p)), [n, 2.0 * n, 3.0 * n], delta=1e-4)
        self.assertSequenceAlmostEqual(
            list(m.get_attribute(v40, p)), [n, 2.0 * n, 3.0 * n, 4.0 * n],
            delta=1e-4)
        self.assertSequenceAlmostEqual(
            list(m.get_attribute(v4d0, p)), [n, 2.0 * n, 3.0 * n, 4.0 * n],
            delta=1e-4)

    def _set_attributes(self, m, p, n, op):
        p.set_value(fk1, n * 3)
        p.remove_attribute(fk0)
        p.remove_attribute(sp_fk0)
        p.add_attribute(fk2, n * 7)
        p.remove_attribute(fsk0)
        p.set_value(ik1, -4 * n)
        p.remove_attribute(ik0)
        p.remove_attribute(sp_ik0)
        p.add_attribute(ik2, n * 7 + 2)
        p.set_value(sk1, "setit")
        p.remove_attribute(sk0)
        p.remove_attribute(sp_sk0)
        p.add_attribute(sk2, "alreadythere")
        p.set_value(pk1, p)
        p.remove_attribute(pk0)
        p.remove_attribute(sp_pk0)
        p.add_attribute(pk2, op)
        m.remove_attribute(v30, p)
        m.remove_attribute(v3d0, p)
        m.set_attribute(v40, p, IMP.Vector4D(n * 2, n * 3, n * 4, n * 5))
        m.set_attribute(v4d0, p, IMP.Vector4D(n * 2, n * 3, n * 4, n * 5))

    def _test_set(self, m, p, n, op):
        self.assertEqual(p.get_value(fk1), n * 3)
        self.assertFalse(p.has_attribute(fk0))
        self.assertFalse(p.has_attribute(sp_fk0))
        self.assertFalse(p.has_attribute(fsk0))
        self.assertEqual(p.get_value(fk2), n * 7)
        self.assertEqual(p.get_value(ik1), -4 * n)
        self.assertFalse(p.has_attribute(ik0))
        self.assertFalse(p.has_attribute(sp_ik0))
        self.assertEqual(p.get_value(ik2), n * 7 + 2)
        self.assertEqual(p.get_value(sk1), "setit")
        self.assertFalse(p.has_attribute(sk0))
        self.assertFalse(p.has_attribute(sp_sk0))
        self.assertEqual(p.get_value(sk2), "alreadythere")
        self.assertEqual(p.get_value(pk1), p)
        self.assertFalse(p.has_attribute(pk0))
        self.assertFalse(p.has_attribute(sp_pk0))
        self.assertEqual(p.get_value(pk2), op)
        self.assertFalse(m.get_has_attribute(v30, p))
        self.assertFalse(m.get_has_attribute(v3d0, p))
        self.assertSequenceAlmostEqual(
            list(m.get_attribute(v40, p)), [n*2, n*3, n*4, n*5], delta=1e-4)
        self.assertSequenceAlmostEqual(
            list(m.get_attribute(v4d0, p)), [n*2, n*3, n*4, n*5], delta=1e-4)

    def _scramble(self, p):
        self._force_set(p, fk0, -3)
        self._force_set(p, sp_fk0, -3)
        self._force_remove(p, fk1)
        self._force_set(p, fk2, 666)
        self._force_set(p, ik0, -4)
        self._force_set(p, sp_ik0, -4)
        self._force_remove(p, ik1)
        self._force_set(p, ik2, -5)
        self._force_set(p, sk0, "hi")
        self._force_set(p, sp_sk0, "hi")
        self._force_remove(p, sk1)
        self._force_set(p, sk2, "there")
        self._force_set(p, pk0, p)
        self._force_set(p, sp_pk0, p)
        self._force_remove(p, pk1)
        self._force_set(p, pk2, p)

    def _make_things(self):
        IMP.set_log_level(IMP.MEMORY)
        print("starting")
        m = IMP.Model("config set")
        print("adding")
        ps = [IMP.Particle(
            m),
            IMP.Particle(m),
            IMP.Particle(m)]
        print("adding attribute")
        self._add_attributes(m, ps[0], 0, ps[1])
        self._add_attributes(m, ps[1], 1, ps[1])
        self._add_attributes(m, ps[2], 2, ps[0])
        print("removing")
        m.remove_particle(ps[2].get_index())
        print("returning")
        return (m, ps)

    def test_noop(self):
        """Testing no-ops with the ConfigurationSet"""
        (m, ps) = self._make_things()
        print("create")
        cs = IMP.ConfigurationSet(m)
        self._test_base(m, ps[0], 0, ps[1])
        self._test_base(m, ps[1], 1, ps[1])
        # no-op
        print("load")
        cs.load_configuration(-1)
        self._test_base(m, ps[0], 0, ps[1])
        self._test_base(m, ps[1], 1, ps[1])

    def test_scramble(self):
        """Testing restore with ConfigurationSet"""
        (m, ps) = self._make_things()
        cs = IMP.ConfigurationSet(m)
        #self._test_base(ps[2], 2, ps[0])
        self._scramble(ps[0])
        self._scramble(ps[1])
        cs.load_configuration(-1)
        self._test_base(m, ps[0], 0, ps[1])
        self._test_base(m, ps[1], 1, ps[1])

    def test_diff(self):
        """Testing restore a known state with ConfigurationSet"""
        (m, ps) = self._make_things()
        cs = IMP.ConfigurationSet(m)
        #self._test_base(ps[2], 2, ps[0])
        self._set_attributes(m, ps[0], 0, ps[1])
        self._set_attributes(m, ps[1], 1, ps[0])
        cs.save_configuration()
        self._scramble(ps[0])
        self._scramble(ps[1])
        print("load 0")
        cs.load_configuration(0)
        print("testing")
        self._test_set(m, ps[0], 0, ps[1])
        print("testing")
        self._test_set(m, ps[1], 1, ps[0])
        print("load -1")
        cs.load_configuration(-1)
        self._test_base(m, ps[0], 0, ps[1])
        self._test_base(m, ps[1], 1, ps[1])


if __name__ == '__main__':
    IMP.test.main()
