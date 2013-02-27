#!/usr/bin/env python
import unittest
import RMF


I1= RMF.HDF5.DataSetIndex1D
I2= RMF.HDF5.DataSetIndex2D
I3= RMF.HDF5.DataSetIndex3D
class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)

    def _do_touch_types(self, f, pccc):
        ds= f.add_child_float_data_set_2d("ds"+str(pccc))
        dsi= f.add_child_index_data_set_2d("dsi"+str(pccc))
        lst=[(f.add_child_int_data_set_1d, 4),
             (f.add_child_float_data_set_1d, 3.1415),
             (f.add_child_string_data_set_1d, "there"),
             (f.add_child_index_data_set_1d, 3),
             #(f.add_child_float_data_set_2d_data_set_1d, ds),
             #(f.add_child_index_data_set_2d_data_set_1d, dsi),
             (f.add_child_ints_data_set_1d, [3,4,5,6]),
             #(f.add_child_node_ids_data_set_1d, [nh]),
             ]
        # swig can't get the node ids array right
        self.assertEqual(len(lst),len(RMF.HDF5.get_data_types())-3)
        for i,p in enumerate(lst):
            mds=p[0](str(i))
            mds.set_size([1])
            mds.set_value([0], p[1])
            vo= mds.get_value([0])
            self.assertEqual(vo, p[1])
    def _touch_all_types(self, nm):
        """touch all types so all static hids are created"""
        print "touching"
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path(nm+"_types.hdf5"))
        self._do_touch_types(f, False)
        print "done touching"

    """Test the python code"""
    def test_perturbed(self):
        """Test low level usage of hdf5"""
        self._touch_all_types("pert")
        num_base_handles=RMF.HDF5.get_number_of_open_handles()
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("test.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child_group("hi")
        ff= g.get_file()
        self.assertEqual(ff.get_name(), f.get_name())
        self._show(f)
        del g
        del f
        del ff
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles)
        f= RMF.HDF5.open_file(RMF.HDF5._get_temporary_file_path("test.hdf5"))
        print "showing"
        self._show(f)
        del f
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles)

    def test_ds(self):
        """Test low level usage of hdf5 with datasets"""
        self._touch_all_types("datasets")
        num_base_handles=RMF.HDF5.get_number_of_open_handles()
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testd.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child_group("hi")
        self._show(f)
        ds= f.add_child_float_data_set_3d("x")
        print "name is", ds.get_name()
        self._show(f)
        ds.set_size(I3(1,1,1))
        ds.set_value(I3(0,0,0), 1)
        print ds.get_value(I3(0,0,0))
        self.assertEqual(ds.get_value(I3(0,0,0)), 1)
        ds= f.add_child_string_data_set_3d("str")
        self._show(f)
        ds.set_size(I3(2,1,1))
        ds.set_value([1,0,0],"there")
        print ds.get_value([1,0,0])
        self.assertEqual(ds.get_value(I3(1,0,0)), "there")
        ds= f.add_child_int_data_set_3d("int")
        self._show(f)
        ds.set_size(I3(1,1,1))
        ds.set_value(I3(0,0,0),1)
        print ds.get_value(I3(0,0,0))
        self.assertEqual(ds.get_value(I3(0,0,0)), 1)
        del f
        del g
        del ds
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles)
    def test_dsb(self):
        """Test writing of blocks with data sets"""
        self._touch_all_types("block_datasets")
        num_base_handles=RMF.HDF5.get_number_of_open_handles()
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testdb.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child_group("hi")
        self._show(f)
        ds= f.add_child_index_data_set_3d("x")
        print "name is", ds.get_name()
        self._show(f)
        ds.set_size(I3(10,10,10))
        ds.set_block(I3(2,3,4), I3(3,4,5), range(0,3*4*5))
        got= ds.get_block(I3(2,3,4), I3(3,4,5))
        self.assertEqual(got, range(0,3*4*5))
        del g
        del f
        del ds
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles)
    def test_dsgrow(self):
        """Test low level usage of hdf5 with datasets that grow"""
        self._touch_all_types("growing_datasets")
        num_base_handles=RMF.HDF5.get_number_of_open_handles()
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testdg.hdf5"))
        self._show(f)
        print "adding"
        g= f.add_child_group("hi")
        self._show(f)
        ds= f.add_child_float_data_set_3d("coords")
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
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles)


    def test_arrays(self):
        """Test arrays of integers"""
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testadg.hdf5"))
        self._show(f)
        print "adding"
        ds= f.add_child_ints_data_set_2d("coords")
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
        f= RMF.HDF5.open_file(RMF.HDF5._get_temporary_file_path("testadg.hdf5"))
        ds= f.get_child_ints_data_set_2d("coords")
        in2=ds.get_value([0,0])
        in3=ds.get_value([1,1])
        self.assertEqual(in2, out)
        self.assertEqual(in3, [])
    def test_arrays_strings(self):
        """Test strings data set 2d"""
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testadgs.hdf5"))
        self._show(f)
        print "adding"
        ds= f.add_child_string_data_set_2d("strings")
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
        f= RMF.HDF5.open_file(RMF.HDF5._get_temporary_file_path("testadgs.hdf5"))
        ds= f.get_child_string_data_set_2d("strings")
        in2=ds.get_value([0,0])
        in3=ds.get_value([1,1])
        self.assertEqual(in2, out)
        self.assertEqual(in3, '')
    def test_arrays_strings_1d(self):
        """Test strings data set 1d"""
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testadgs1.hdf5"))
        self._show(f)
        print "adding"
        ds= f.add_child_string_data_set_1d("strings")
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
        f= RMF.HDF5.open_file(RMF.HDF5._get_temporary_file_path("testadgs1.hdf5"))
        ds= f.get_child_string_data_set_1d("strings")
        in2=ds.get_value([0])
        in3=ds.get_value([1])
        self.assertEqual(in2, out)
        self.assertEqual(in3, 'adfjhslak')
    def test_as(self):
        """Test low level usage of hdf5 with attributes"""
        self._touch_all_types("attributes")
        num_base_handles=RMF.HDF5.get_number_of_open_handles()
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("testa.hdf5"))
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+1)
        self._show(f)
        print "adding"
        g= f.add_child_group("hi")
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+2)
        print "float"
        f.set_float_attribute("at", [2.0])
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+2)
        print f.get_float_attribute("at")
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+2)
        self.assertEqual(f.get_float_attribute("at"), [2.0])
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+2)
        print "string"
        #f.set_string_attribute("str", ["there", "not there"])
        #self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+2)
        self._show(f)
        #print f.get_string_attribute("str")
        #self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles+2)
        #self.assertEqual(f.get_string_attribute("str"), ["there", "not there"])
        del g
        del f
        print "done"
        self.assertEqual(RMF.HDF5.get_number_of_open_handles(), num_base_handles)
    def test_get_groups(self):
        """Test getting of child groups"""
        f= RMF.HDF5.create_file(RMF.HDF5._get_temporary_file_path("test_get_groups.hdf5"))
        ch= f.add_child_group("hi");
        chb= f.get_child_group(0)
        self.assertEqual(ch.get_name(), chb.get_name())
if __name__ == '__main__':
    unittest.main()
