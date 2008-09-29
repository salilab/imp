import unittest
import IMP
import IMP.test
import IMP.misc

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""

    def _set_up_stuff(self, n):
        # return [model, particles, bonds]
        m= IMP.Model()
        ps= self.create_particles_in_box(m)
        bds= self.create_chain(ps, 10)
        bl= IMP.BondDecoratorListScoreState(ps)
        ss= IMP.misc.CoverBondsScoreState(bl, rk)
        m.add_score_state(bl)
        m.add_score_state(ss)
        return [m, ps, bds]


    def test_set_position(self):
        """Make sure that bond cover coordinates are correct"""
        IMP.set_log_level(IMP.VERBOSE)
        [m,ps, bds]= self._set_up_stuff(8)
        vk= IMP.FloatKey("volume")
        rk= IMP.FloatKey("radius")
        bc= IMP.misc.BondCoverParticleRefiner(rk, vk)
        for p in ps:
            self.assert_(not bc.get_can_refine(p))
        for b in bds:
            b.get_particle().add_attribute(vk, 1, False)
            self.assert_(bc.get_can_refine(b.get_particle()))
            rps= bc.get_refined(b.get_particle())

            d0= IMP.XYZDecorator.cast(b.get_bonded(0).get_particle())
            d1= IMP.XYZDecorator.cast(b.get_bonded(1).get_particle())

            vol=0.0
            print "Bond from "
            d0.show()
            print " to "
            d1.show()
            print
            for p in rps:
                r= p.get_value(rk)
                print "Cover particle is has radius "+str(r)
                dc= IMP.XYZDecorator.cast(p)
                dc.show()
                print
                v= 4.0/3.0 * 3.14 * r**3
                vol = vol + v
            n= len(rps)
            print vol*(n-1.0)/n
            print vol
            print vol*(n+1.0)/n
            print n
            # crap check
            self.assert_(.5*vol* (n-1.0)/n < 1 and 2*vol *(n+1.0)/n > 1)




if __name__ == '__main__':
    unittest.main()
