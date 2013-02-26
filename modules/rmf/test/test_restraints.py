import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def _write_restraint(self, name):
        f= RMF.create_rmf_file(name)
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.rmf.add_particles(f, [p]);
        r= IMP.kernel._ConstRestraint(1, [p])
        r.set_model(m)
        r.evaluate(False)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, 0)
    def _read_restraint(self, name):
        IMP.base.add_to_log(IMP.base.TERSE, "Starting reading back\n")
        f= RMF.open_rmf_file_read_only(name)
        m= IMP.Model()
        ps=IMP.rmf.create_particles(f, m);
        rs=IMP.RestraintSet.get_from(IMP.rmf.create_restraints(f, m)[0])
        IMP.rmf.load_frame(f, 0)
        r= rs.get_restraints()[0]
        print [IMP.Particle.get_from(x).get_index() for x in r.get_inputs()]
        print [x.get_index() for x in ps]
        self.assertEqual(r.get_inputs(), ps)
    def test_0(self):
        """Test writing restraints rmf"""
        RMF.set_log_level("Off")
        for suffix in RMF.suffixes:
            name=self.get_tmp_file_name("restr."+suffix)
            self._write_restraint(name)
            self._read_restraint(name)
    def test_1(self):
        for suffix in RMF.suffixes:
            """Test writing restraints to rmf with no particles"""
            nm=self.get_tmp_file_name("restrnp."+suffix)
            print nm
            f= RMF.create_rmf_file(nm)
            m= IMP.Model()
            p= IMP.Particle(m)
            r= IMP.kernel._ConstRestraint(1)
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
        for suffix in RMF.suffixes:
            RMF.HDF5.set_show_errors(True)
            path=self.get_tmp_file_name("dynamic_restraints."+suffix)
            print path
            f= RMF.create_rmf_file(path)
            IMP.base.set_log_level(IMP.base.SILENT)
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

            del f
            del r
            f= RMF.open_rmf_file_read_only(path)
            bps= IMP.rmf.create_particles(f, m)
            rr= IMP.rmf.create_restraints(f, m)
            #print scores
            for i in range(0,11):
                IMP.rmf.load_frame(f, i)
                #print i
                self.assertAlmostEqual(scores[i], rr[0].evaluate(False), delta=.01)

if __name__ == '__main__':
    unittest.main()
