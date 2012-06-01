import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def test_0(self):
        """Test writing restraints rmf"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("restr.rmf"))
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.rmf.add_particles(f, [p]);
        r= IMP._ConstRestraint(1, [p])
        r.set_model(m)
        r.evaluate(False)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, 0)
    def test_1(self):
        """Test writing restraints to rmf with no particles"""
        f= RMF.create_rmf_file(self.get_tmp_file_name("restrnp.rmf"))
        m= IMP.Model()
        p= IMP.Particle(m)
        r= IMP._ConstRestraint(1)
        r.set_name("R")
        r.set_model(m)
        r.evaluate(False)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, 0)
        rr= IMP.rmf.create_restraints(f, m)
        IMP.rmf.load_frame(f, 0)
        self.assertEqual(rr[0].evaluate(False), r.evaluate(False))
    def test_2(self):
        """Test writing dynamic restraints"""
        RMF.set_show_hdf5_errors(True)
        f= RMF.create_rmf_file(self.get_tmp_file_name("dynamic_restraints.rmf"))
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ds= [IMP.core.XYZR.setup_particle(p) for p in ps]
        for d in ds:
            d.set_radius(1)
        IMP.rmf.add_particles(f, ds)
        cpc= IMP.container.ClosePairContainer(ps, 0)
        r= IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), cpc)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10,10,10))
        r.set_model(m)
        r.evaluate(False)
        IMP.rmf.add_restraint(f, r)
        scores=[]
        for i in range(0,10):
            for d in ds:
                d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            scores.append(r.evaluate(False))
            IMP.rmf.save_frame(f, i)
        for i,d in enumerate(ds):
            d.set_x(i*10)
        scores.append(r.evaluate(False))
        IMP.rmf.save_frame(f, 10)

        bps= IMP.rmf.create_particles(f, m)
        rr= IMP.rmf.create_restraints(f, m)
        print scores
        for i in range(0,11):
            IMP.rmf.load_frame(f, i)
            print i
            self.assertAlmostEqual(scores[i], rr[0].evaluate(False), delta=.01)

if __name__ == '__main__':
    unittest.main()
