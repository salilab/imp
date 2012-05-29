import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def test_perturbed(self):
        """Test reading and writing particles"""
        m= IMP.Model()
        p0= IMP.Particle(m)
        p1= IMP.Particle(m)
        ik= IMP.IntKey("hi int")
        fk= IMP.FloatKey("hi float")
        p0.add_attribute(ik, 1)
        p1.add_attribute(fk, 2.0)
        p1.add_attribute(ik, 3)
        name= self.get_tmp_file_name("particles.rmf")
        rmf= RMF.create_rmf_file(name)
        IMP.set_log_level(IMP.VERBOSE)
        IMP.rmf.add_particles(rmf, [p0, p1])
        IMP.rmf.save_frame(rmf, 0)
        IMP.set_log_level(IMP.MEMORY)
        onames= RMF.get_open_hdf5_handle_names()
        print "before", onames
        del rmf
        IMP.set_log_level(IMP.VERBOSE)
        onames= RMF.get_open_hdf5_handle_names()
        print "closed", onames
        self.assertEqual(len(onames), 0)
        rmf= RMF.open_rmf_file_read_only(name)
        bps= IMP.rmf.create_particles(rmf, m)
        #IMP.rmf.load_frame(0)
        self.assertEqual(len(bps), 2)
        self.assertTrue(bps[0].has_attribute(ik))
        self.assertEqual(bps[0].get_value(ik), 1);
        self.assertTrue(bps[1].has_attribute(ik))
        self.assertEqual(bps[1].get_value(ik), 3);
        self.assertTrue(bps[1].has_attribute(fk))
        self.assertEqual(bps[1].get_value(fk), 2.0);

    def test_perturbed_frames(self):
        """Test reading and writing particles with frames"""
        m= IMP.Model()
        p0= IMP.Particle(m)
        p1= IMP.Particle(m)
        ik= IMP.IntKey("hi int")
        fk= IMP.FloatKey("hi float")
        p0.add_attribute(ik, 1)
        p1.add_attribute(fk, 2.0)
        p1.add_attribute(ik, 3)
        name= self.get_tmp_file_name("particles.rmf")
        rmf= RMF.create_rmf_file(name)
        IMP.set_log_level(IMP.VERBOSE)
        IMP.rmf.add_particles(rmf, [p0, p1])
        IMP.rmf.save_frame(rmf, 0)
        p1.set_value(ik, 5)
        IMP.rmf.save_frame(rmf, 1)
        IMP.set_log_level(IMP.MEMORY)
        onames= RMF.get_open_hdf5_handle_names()
        print "before", onames
        del rmf
        IMP.set_log_level(IMP.VERBOSE)
        onames= RMF.get_open_hdf5_handle_names()
        print >> sys.stderr, "closed", onames
        self.assertEqual(len(onames), 0)
        rmf= RMF.open_rmf_file_read_only(name)
        bps= IMP.rmf.create_particles(rmf, m)
        #IMP.rmf.load_frame(0)
        self.assertEqual(len(bps), 2)
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
