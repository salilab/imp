import unittest
import IMP
import IMP.test

rk = IMP.FloatKey("radius")

class TestBondCover(IMP.test.TestCase):
    """Tests for BondCover"""

    def _set_up_stuff(self, n):
        # return [model, particles, bonds]
        m= IMP.Model()
        ps= IMP.Particles()
        for i in range(0,n):
            p= IMP.Particle()
            m.add_particle(p)
            d= IMP.XYZDecorator.create(p)
            d.set_coordinates(IMP.random_vector_in_box(IMP.Vector3D(0,0,0),
                                                       IMP.Vector3D(10,10,10)))
            ps.append(p)
        bds= []
        bb= IMP.BondedDecorator.create(ps[0])
        for i in range(1,n):
            ba= IMP.BondedDecorator.cast(ps[i-1])
            bb= IMP.BondedDecorator.create(ps[i])
            bds.append(IMP.custom_bond(ba, bb, 10, 1))
        bl= IMP.BondDecoratorListScoreState(ps)
        ss= IMP.CoverBondsScoreState(bl, rk)
        m.add_score_state(bl)
        m.add_score_state(ss)
        return [m, ps, bds]




    def test_set_position(self):
        """Make sure that bond cover coordinates are correct"""
        IMP.set_log_level(IMP.VERBOSE)
        [m,ps, bds]= self._set_up_stuff(10)
        m.evaluate(False)
        for b in bds:
            b.get_particle().show()
            ba= b.get_bonded(0)
            bb= b.get_bonded(1)
            da= IMP.XYZDecorator.cast(ba.get_particle())
            db= IMP.XYZDecorator.cast(bb.get_particle())
            dc= IMP.XYZDecorator.cast(b.get_particle())
            center= [(da.get_x()+ db.get_x()) /2.0,
                     (da.get_y()+ db.get_y()) /2.0,
                     (da.get_z()+ db.get_z()) /2.0]
            r= ((center[0]- da.get_x())**2
                +(center[1]- da.get_y())**2
                +(center[2]- da.get_z())**2)**.5
            print r
            print center
            da.show()
            db.show()
            dc.show()
            self.assertInTolerance(r, b.get_particle().get_value(rk), r*.1,
                                   "Radius is not close enough")
            self.assertInTolerance(center[0], dc.get_x(), .1,
                                   "X is not close enough")
            self.assertInTolerance(center[1], dc.get_y(), .1,
                                   "Y is not close enough")
            self.assertInTolerance(center[2], dc.get_z(), .1,
                                   "Z is not close enough")




if __name__ == '__main__':
    unittest.main()
