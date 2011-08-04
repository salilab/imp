import unittest
import IMP.rmf
import IMP.test
from IMP.algebra import *

num_base_handles=3

class GenericTest(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test low level usage of hdf5"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("test.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self._show(f)
        del g
        del f
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles)
        f= IMP.rmf.open_hdf5_file(self.get_tmp_file_name("test.hdf5"))
        print "showing"
        self._show(f)
        del f
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles)

    def test_ds(self):
        """Test low level usage of hdf5 with datasets"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("testd.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self._show(f)
        ds= f.add_child_float_data_set("x", 3)
        self._show(f)
        ds.set_size([1,1,1])
        ds.set_value([0,0,0], 1)
        print ds.get_value([0,0,0])
        self.assertEqual(ds.get_value([0,0,0]), 1)
        ds= f.add_child_string_data_set("str", 3)
        self._show(f)
        ds.set_size([2,1,1])
        ds.set_value([1,0,0],"there")
        print ds.get_value([1,0,0])
        self.assertEqual(ds.get_value([1,0,0]), "there")
        ds= f.add_child_int_data_set("int", 3)
        self._show(f)
        ds.set_size([1,1,1])
        ds.set_value([0,0,0],1)
        print ds.get_value([0,0,0])
        self.assertEqual(ds.get_value([0,0,0]), 1)
        del f
        del g
        del ds
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles)
    def test_dsgrow(self):
        """Test low level usage of hdf5 with datasets that grow"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("testdg.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self._show(f)
        ds= f.add_child_float_data_set("coords",
                                 3)
        print "setting num"
        ds.set_size([1,3,1])
        print "setting values",[0,0,0]
        ds.set_value([0,0,0], 1)
        print "getting", [0,0,0]
        print ds.get_value([0,0,0])
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
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles)


    def test_arrays(self):
        """Test arrays of integers"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("testadg.hdf5"))
        self._show(f)
        print "adding"
        ds= f.add_child_ints_data_set("coords",
                                      2)
        print "setting num"
        ds.set_size([1,3])
        ind=[0,1,2,3]
        ds.set_value([0,0], ind)
        ds.set_value([0,1], [4,5,6,7,8,9])
        ds.set_size([2,3])
        out=ds.get_value([0,0])
        print out
        print ds.get_value([0,1])
        self.assertEqual(ind,out)

        del ds
        del f
        f= IMP.rmf.open_hdf5_file(self.get_tmp_file_name("testadg.hdf5"))
        ds= f.get_child_ints_data_set("coords")
        in2=ds.get_value([0,0])
        in3=ds.get_value([1,1])
        self.assertEqual(in2, out)
        self.assertEqual(in3, [])
    def test_arrays_strings(self):
        """Test strings data set"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("testadgs.hdf5"))
        self._show(f)
        print "adding"
        ds= f.add_child_string_data_set("strings",
                                        2)
        print "setting num"
        ds.set_size([1,3])
        ind="0123"
        ds.set_value([0,0], ind)
        ds.set_value([0,1], "adfjhslak")
        ds.set_size([2,3])
        out=ds.get_value([0,0])
        print out
        print ds.get_value([0,1])
        self.assertEqual(ind,out)

        del ds
        del f
        f= IMP.rmf.open_hdf5_file(self.get_tmp_file_name("testadgs.hdf5"))
        ds= f.get_child_string_data_set("strings")
        in2=ds.get_value([0,0])
        in3=ds.get_value([1,1])
        self.assertEqual(in2, out)
        self.assertEqual(in3, '')
    def test_arrays_strings_1d(self):
        """Test strings data set"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("testadgs1.hdf5"))
        self._show(f)
        print "adding"
        ds= f.add_child_string_data_set("strings",
                                        1)
        print "setting num"
        ds.set_size([1])
        ind="0123"
        ds.set_value([0], ind)
        ds.set_size([2])
        ds.set_value([1], "adfjhslak")
        ds.set_size([3])
        out=ds.get_value([0])
        print out
        print ds.get_value([1])
        self.assertEqual(ind,out)

        del ds
        del f
        f= IMP.rmf.open_hdf5_file(self.get_tmp_file_name("testadgs1.hdf5"))
        ds= f.get_child_string_data_set("strings")
        in2=ds.get_value([0])
        in3=ds.get_value([1])
        self.assertEqual(in2, out)
        self.assertEqual(in3, 'adfjhslak')
    def test_as(self):
        """Test low level usage of hdf5 with attributes"""
        f= IMP.rmf.create_hdf5_file(self.get_tmp_file_name("testa.hdf5"))
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+1)
        self._show(f)
        print "adding"
        g= f.add_child("hi")
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+2)
        print "float"
        f.set_float_attribute("at", [2.0])
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+2)
        print f.get_float_attribute("at")
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+2)
        self.assertEqual(f.get_float_attribute("at"), [2.0])
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+2)
        print "string"
        #f.set_string_attribute("str", ["there", "not there"])
        #self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+2)
        self._show(f)
        #print f.get_string_attribute("str")
        #self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles+2)
        #self.assertEqual(f.get_string_attribute("str"), ["there", "not there"])
        del g
        del f
        print "done"
        self.assertEqual(IMP.rmf.get_number_of_open_hdf5_handles(), num_base_handles)

if __name__ == '__main__':
    unittest.main()
