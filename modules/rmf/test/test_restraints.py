import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def test_0(self):
        """Test writing restraints rmf"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("restr.hdf5"))
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.rmf.add_particle(f, p);
        r= IMP._ConstRestraint(1, [p])
        r.set_model(m)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, 0, r)
    def test_1(self):
        """Test writing restraints to rmf with no particles"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("restrnp.hdf5"))
        m= IMP.Model()
        p= IMP.Particle(m)
        r= IMP._ConstRestraint(1)
        r.set_model(m)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, 0, r)
    def test_2(self):
        """Test writing dynamic restraints"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("restrnp.hdf5"))
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ds= [IMP.core.XYZR.setup_particle(p) for p in ps]
        for d in ds:
            d.set_radius(1)
            IMP.rmf.add_particle(f, d)
        cpc= IMP.container.ClosePairContainer(ps, 0)
        r= IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), cpc)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        IMP.rmf.add_restraint(f, r)
        for i in range(0,10):
            for d in ds:
                d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            m.update()
            IMP.rmf.save_frame(f, i, r)
        for i,d in enumerate(ds):
            d.set_x(i*10)
        m.update()
        IMP.rmf.save_frame(f, 10, r)

if __name__ == '__main__':
    unittest.main()
