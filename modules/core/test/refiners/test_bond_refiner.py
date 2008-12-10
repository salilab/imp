import unittest
import IMP
import IMP.test
import IMP.utils
import IMP.core
import math

class AllPairsContainerTest(IMP.test.TestCase):
    """Tests for bond refined"""

    def test_allp(self):
        """Check that BondRefiner works"""
        m=IMP.Model()
        p0= IMP.Particle()
        m.add_particle(p0)
        b0= IMP.core.BondedDecorator.create(p0)
        p1= IMP.Particle()
        m.add_particle(p1)
        b1= IMP.core.BondedDecorator.create(p1)
        b= IMP.core.custom_bond(b0, b1, 1)
        r= IMP.core.BondEndpointsParticleRefiner()
        self.assert_(not r.get_can_refine(p0))
        self.assert_(not r.get_can_refine(p1))
        self.assert_(r.get_can_refine(b.get_particle()))
        ps= r.get_refined(b.get_particle())
        self.assertEqual(ps[0], p0)
        self.assertEqual(ps[1], p1)
        mps= IMP.Particles(ps)
        r.cleanup_refined(b.get_particle(), mps)
