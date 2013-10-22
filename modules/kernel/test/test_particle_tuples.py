import IMP
import IMP.test

class Tests(IMP.test.TestCase):
    """Test ParticleTuples"""

    def test_pairs(self):
        """Test ParticlePair objects"""
        m = IMP.kernel.Model("pairs")
        p1 = IMP.kernel.Particle(m)
        p2 = IMP.kernel.Particle(m)
        p = (p1, p2)
        #self.assertEqual(p.get_dimension(), 2)
        self.assertEqual(p[0], p1)
        self.assertEqual(p[1], p2)
        #self.assertEqual(p[0], p.get(0))
        #self.assert_(isinstance(p.get_name(), str))

    def test_triplets(self):
        """Test ParticleTriplet objects"""
        m = IMP.kernel.Model("triplets")
        p1 = IMP.kernel.Particle(m)
        p2 = IMP.kernel.Particle(m)
        p3 = IMP.kernel.Particle(m)
        p = IMP.kernel.ParticleTriplet(p1, p2, p3)
        #self.assertEqual(p.get_dimension(), 3)
        self.assertEqual(p[0], p1)
        self.assertEqual(p[1], p2)
        self.assertEqual(p[2], p3)
        #self.assertEqual(p[0], p.get(0))
        #self.assert_(isinstance(p.get_name(), str))

    def test_quads(self):
        """Test ParticleQuad objects"""
        m = IMP.kernel.Model("quads")
        p1 = IMP.kernel.Particle(m)
        p2 = IMP.kernel.Particle(m)
        p3 = IMP.kernel.Particle(m)
        p4 = IMP.kernel.Particle(m)
        p = IMP.kernel.ParticleQuad(p1, p2, p3, p4)
        #self.assertEqual(p.get_dimension(), 4)
        self.assertEqual(p[0], p1)
        self.assertEqual(p[1], p2)
        self.assertEqual(p[2], p3)
        self.assertEqual(p[3], p4)
        #self.assertEqual(p[0], p.get(0))
        #self.assert_(isinstance(p.get_name(), str))

if __name__ == '__main__':
    IMP.test.main()
