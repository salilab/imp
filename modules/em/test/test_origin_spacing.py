import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""
    def test_origin_spacing_data_is_kept_in_mrc_format1(self):
        scene = IMP.em.read_map(self.get_input_file_name("in.mrc"), self.mrw)
        scene.set_origin(-100,-100,-100)
        IMP.em.write_map(scene,"test1.mrc",self.mrw)
        scene2 = IMP.em.read_map("test1.mrc", self.mrw)
        os.unlink("test1.mrc")
        origin2=scene2.get_origin()
        self.assertEqual(-100, origin2[0])
        self.assertEqual(-100, origin2[1])
        self.assertEqual(-100, origin2[2])
        self.assertEqual(1, scene2.get_spacing())

    def test_origin_spacing_data_is_kept_in_mrc_format2(self):
        mrw = IMP.em.MRCReaderWriter()
        scene = IMP.em.read_map(self.get_input_file_name("in.mrc"), self.mrw)
        scene.set_origin(-100,-100,-100)
        scene.update_voxel_size(10)
        self.assertEqual(10, scene.get_spacing())
        IMP.em.write_map(scene,"test2.mrc",self.mrw)
        scene2 = IMP.em.read_map("test2.mrc", self.mrw)
        os.unlink("test2.mrc")
        origin2=scene2.get_origin()
        self.assertEqual(-100, origin2[0])
        self.assertEqual(-100, origin2[1])
        self.assertEqual(-100, origin2[2])
        self.assertEqual(10, scene2.get_spacing())


    def test_origin_spacing_data_is_kept_in_mrc_format3(self):
        mrw = IMP.em.MRCReaderWriter()
        scene = IMP.em.read_map(self.get_input_file_name("in.mrc"), self.mrw)
        scene.update_voxel_size(10)
        scene.set_origin(-100,-100,-100)
        self.assertEqual(10, scene.get_spacing())
        scene.get_header().show()
        IMP.em.write_map(scene,"test3.mrc",self.mrw)
        scene2 = IMP.em.read_map("test3.mrc", self.mrw)
        os.unlink("test3.mrc")
        origin2=scene2.get_origin()
        self.assertEqual(-100, origin2[0])
        self.assertEqual(-100, origin2[1])
        self.assertEqual(-100, origin2[2])
        self.assertEqual(10, scene2.get_spacing())


    def test_origin_spacing_data_is_kept_in_mrc_format4(self):
        mrw = IMP.em.MRCReaderWriter()
        scene = IMP.em.read_map(self.get_input_file_name("in.mrc"), self.mrw)
        scene.update_voxel_size(10)
        self.assertEqual(10, scene.get_spacing())
        scene.get_header().show()
        IMP.em.write_map(scene,"test4.mrc",self.mrw)
        scene2 = IMP.em.read_map("test4.mrc", self.mrw)
        os.unlink("test4.mrc")
        origin2=scene2.get_origin()
        self.assertEqual(10, scene2.get_spacing())




    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        self.mrw = IMP.em.MRCReaderWriter()
if __name__ == '__main__':
    IMP.test.main()
