import RMF
import os.path
import unittest

class GenericTest(unittest.TestCase):
    def _create(self, name):
        rmf= RMF.create_rmf_file(name)
        rt= rmf.get_root_node()

        # now make hierarchy
        rff= RMF.ReferenceFrameFactory(rmf)
        ef= RMF.ExternalFactory(rmf)
            # add a couple transformed copies of simple
        c0= rt.add_child("copy0", RMF.REPRESENTATION)
        rb0= rff.get(c0)
        rb0.set_translation([100,0,0])
        rb0.set_rotation([1,0,0,0])
        external= c0.add_child("simple.pdb", RMF.REPRESENTATION)
        ed= ef.get(external)
        ed.set_path("simple.pdb")

        c1= rt.add_child("copy1", RMF.REPRESENTATION)
        rb1= rff.get(c1)
        rb1.set_translation([0,0,0])
        rb1.set_rotation([0,1,0,0])
        external= c1.add_child("simple.pdb", RMF.REPRESENTATION)
        ed= ef.get(external)
        ed.set_path("simple.pdb")
    def _read(self, name):
        rmf= RMF.open_rmf_file_read_only(name)
        rt= rmf.get_root_node()
        ref0= rt.get_children()[0].get_children()[0]
        ref1= rt.get_children()[1].get_children()[0]
        ef= RMF.ExternalConstFactory(rmf)

        ref0d= ef.get(ref0)
        path0= ref0d.get_path()
        print path0
        self.assert_(os.path.exists(path0))
        path1= ref0d.get_path()
        self.assert_(os.path.exists(path1))
        self.assertEqual(path0, path1)
        simple0= open(path0, "r").read()
        input= RMF._get_test_input_file_path("simple.pdb")
        inputpdb= open(input, "r").read()
        self.assertEqual(simple0, inputpdb)
    def test_data_types(self):
        """Test external file references"""
        for suffix in RMF.suffixes:
            name= RMF._get_temporary_file_path("externals."+suffix)
            print name
            input= RMF._get_test_input_file_path("simple.pdb")
            inputpdb= open(input, "r").read()
            tdir= os.path.split(name)[0]
            sfile= os.path.join(tdir, "simple.pdb")
            open(sfile, "w").write(inputpdb)

            self._create(name)
            self._read(name)
if __name__ == '__main__':
    unittest.main()
