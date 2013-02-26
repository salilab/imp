import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def test_perturbed(self):
        """Test reading and writing particles"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            p0= IMP.Particle(m)
            p1= IMP.Particle(m)
            ik= IMP.IntKey("hi int")
            fk= IMP.FloatKey("hi float")
            p0.add_attribute(ik, 1)
            p1.add_attribute(fk, 2.0)
            p1.add_attribute(ik, 3)
            name= self.get_tmp_file_name("particles."+suffix)
            print name
            rmf= RMF.create_rmf_file(name)
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.rmf.add_particles(rmf, [p0, p1])
            IMP.rmf.save_frame(rmf, 0)
            del rmf
            rmf= RMF.open_rmf_file_read_only(name)
            bps= IMP.rmf.create_particles(rmf, m)
            IMP.rmf.load_frame(rmf, 0);
        #IMP.rmf.load_frame(0)
            self.assertEqual(len(bps), 2)
            bps[0].show()
            self.assertTrue(bps[0].has_attribute(ik))
            self.assertEqual(bps[0].get_value(ik), 1);
            self.assertTrue(bps[1].has_attribute(ik))
            self.assertEqual(bps[1].get_value(ik), 3);
            self.assertTrue(bps[1].has_attribute(fk))
            self.assertEqual(bps[1].get_value(fk), 2.0);

    def test_perturbed_frames(self):
        """Test reading and writing particles with frames"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            p0= IMP.Particle(m)
            p1= IMP.Particle(m)
            ik= IMP.IntKey("hi int")
            fk= IMP.FloatKey("hi float")
            p0.add_attribute(ik, 1)
            p1.add_attribute(fk, 2.0)
            p1.add_attribute(ik, 3)
            name= self.get_tmp_file_name("particles."+suffix)
            print name
            rmf= RMF.create_rmf_file(name)
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.rmf.add_particles(rmf, [p0, p1])
            IMP.rmf.save_frame(rmf, 0)
            p1.set_value(ik, 5)
            IMP.rmf.save_frame(rmf, 1)
            del rmf
            rmf= RMF.open_rmf_file_read_only(name)
            cat= rmf.get_category("IMP")
            print cat
            keys= rmf.get_keys(cat)
            print [rmf.get_name(k) for k in keys]
            bps= IMP.rmf.create_particles(rmf, m)
            print "load 0"
            IMP.rmf.load_frame(rmf, 0);
            self.assertEqual(len(bps), 2)
            print "out"
            bps[0].show()
            self.assertTrue(bps[0].has_attribute(ik))
            self.assertEqual(bps[0].get_value(ik), 1);
            self.assertTrue(bps[1].has_attribute(ik))
            self.assertEqual(bps[1].get_value(ik), 3);
            self.assertTrue(bps[1].has_attribute(fk))
            self.assertEqual(bps[1].get_value(fk), 2.0);
            IMP.rmf.load_frame(rmf, 1)
            self.assertTrue(bps[1].has_attribute(ik))
            self.assertEqual(bps[1].get_value(ik), 5);


if __name__ == '__main__':
    unittest.main()
