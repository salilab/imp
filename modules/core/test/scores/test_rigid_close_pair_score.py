import unittest
import IMP
import IMP.test
import IMP.core

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Checking rigid body pair score"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        threshold=.3
        psa= IMP.core.create_xyzr_particles(m, 50, 3)
        psb= IMP.core.create_xyzr_particles(m, 50, 3)
        rba= IMP.core.RigidBodyDecorator.create(IMP.Particle(m), psa)
        rbb= IMP.core.RigidBodyDecorator.create(IMP.Particle(m), psb)
        cp= IMP.Particle(m)
        cpd= IMP.core.XYZRDecorator.create(cp, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 1))
        tps= IMP.test.LogPairScore(1)
        rps= IMP.core.RigidClosePairScore(tps, threshold)
        vc= rps.evaluate(rba.get_particle(), rbb.get_particle(), None)
        for a in psa:
            for b in psb:
                da= IMP.core.XYZRDecorator(a)
                db= IMP.core.XYZRDecorator(b)
                dist=IMP.core.distance(da, db)
                if dist < threshold -.1:
                    da.show()
                    db.show()
                    print ": "+str(dist)
                    tps.get_log().index((a,b))
                    print "Found " + a.get_name() + " " + b.get_name()
        tps.clear_log()
        vc=rps.evaluate(cp, rba.get_particle(), None)
        for a in psa:
            da= IMP.core.XYZRDecorator(a)
            dist=IMP.core.distance(da, cpd)
            if dist < threshold -.1:
                tps.get_log().index((cpd.get_particle(), a))
                print "Found " + a.get_name() + " " + b.get_name()



if __name__ == '__main__':
    unittest.main()
