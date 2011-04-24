import unittest
import IMP.rmf
import IMP.test
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test low level usage of hdf5"""
        f= IMP.rmf.HDF5Group(self.get_tmp_file_name("test.hdf5"), True)
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self._show(f)
        del g
        del f
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 1)
        f= IMP.rmf.HDF5Group(self.get_tmp_file_name("test.hdf5"), False)
        print "showing"
        self._show(f)
        del f
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 1)

    def test_ds(self):
        """Test low level usage of hdf5 with datasets"""
        f= IMP.rmf.HDF5Group(self.get_tmp_file_name("testd.hdf5"), True)
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self._show(f)
        ds= f.add_child_float_data_set("x", 3);
        self._show(f)
        ds.set_size([1,1,1])
        ds.set_value([0,0,0], 1)
        print ds.get_value([0,0,0]);
        self.assertEqual(ds.get_value([0,0,0]), 1)
        ds= f.add_child_string_data_set("str", 3);
        self._show(f)
        ds.set_size([2,1,1])
        ds.set_value([1,0,0],"there")
        print ds.get_value([1,0,0]);
        self.assertEqual(ds.get_value([1,0,0]), "there")
        ds= f.add_child_int_data_set("int", 3);
        self._show(f)
        ds.set_size([1,1,1])
        ds.set_value([0,0,0],1)
        print ds.get_value([0,0,0]);
        self.assertEqual(ds.get_value([0,0,0]), 1)
        del f
        del g
        del ds
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 1)
    def test_dsgrow(self):
        """Test low level usage of hdf5 with datasets that grow"""
        f= IMP.rmf.HDF5Group(self.get_tmp_file_name("testdg.hdf5"), True)
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self._show(f)
        ds= f.add_child_float_data_set("coords",
                                 3);
        print "setting num"
        ds.set_size([1,3,1])
        print "setting values",[0,0,0]
        ds.set_value([0,0,0], 1)
        print "getting", [0,0,0]
        print ds.get_value([0,0,0]);
        self.assertEqual(ds.get_value([0,0,0]), 1)
        print "setting",0,2
        ds.set_value([0,2,0], 2)
        print "getting",0,2
        print ds.get_value([0,2,0])
        self.assertEqual(ds.get_value([0,2,0]), 2)
        ds.set_size([4,5,6])
        print "setting",3,4,5
        ds.set_value([3,4,5], 4)
        print "getting",3,4,5
        print ds.get_value([3,4,5])
        self.assertEqual(ds.get_value([3,4,5]), 4)
        print "getting",1,1,1
        print ds.get_value([1,1,1])

        del ds
        del f
        del g
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 1)


    def test_as(self):
        """Test low level usage of hdf5 with attributes"""
        f= IMP.rmf.HDF5Group(self.get_tmp_file_name("testa.hdf5"), True)
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 2)
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 3)
        print "float"
        f.set_float_attribute("at", [2.0])
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 3)
        print f.get_float_attribute("at")
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 3)
        self.assertEqual(f.get_float_attribute("at"), [2.0])
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 3)
        print "string"
        f.set_string_attribute("str", ["there", "not there"])
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 3)
        self._show(f)
        print f.get_string_attribute("str")
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 3)
        self.assertEqual(f.get_string_attribute("str"), ["there", "not there"])
        del g
        del f
        print "done"
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), 1)

if __name__ == '__main__':
    unittest.main()
