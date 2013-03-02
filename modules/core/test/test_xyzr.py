import IMP
import IMP.test
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):
    def test_xyzr(self):
        """Testing XYZR decorators"""
        m = IMP.Model()
        pa=IMP.Particle(m)
        pb=IMP.Particle(m)
        da= IMP.core.XYZR.setup_particle(pa)
        db= IMP.core.XYZR.setup_particle(pb)
        da.set_radius(1.0)
        db.set_radius(1.5)
        da.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        db.set_coordinates(IMP.algebra.Vector3D(6,6,6))
        d= IMP.core.get_distance(da, db)
        self.assertAlmostEqual(d, 10.3-2.5, delta=.5)
        nrda= IMP.core.XYZ(pa)
        nrdb= IMP.core.XYZ(pb)
    def test_xyzr_overload(self):
        """Testing XYZR overload decorators"""
        m = IMP.Model()
        pa=IMP.Particle(m)
        pb=IMP.Particle(m)
        da= IMP.core.XYZR.setup_particle(pa)
        db= IMP.core.XYZR.setup_particle(pb)
        da.set_radius(1.0)
        db.set_radius(1.5)
        da.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        db.set_coordinates(IMP.algebra.Vector3D(6,6,6))
        d= IMP.core.get_distance(da, db)
        self.assertAlmostEqual(d, 10.3-2.5, delta=.5)
        nrda= IMP.core.XYZ(pa)
        nrdb= IMP.core.XYZ(pb)
        dbr= IMP.core.get_distance(nrda, nrdb)
        print d, dbr
        self.assertLess(d+2.4, dbr)


if __name__ == '__main__':
    IMP.test.main()
