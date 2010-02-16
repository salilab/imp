import unittest
import IMP
import IMP.test
import os.path
from StringIO import StringIO

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test that conversions to input files work properly"""
        IMP.set_log_level(IMP.MEMORY)
        v= IMP.test_ifile(self.get_input_file_name("text"))
        self.assertEqual(v, "word")
        v= IMP.test_ifile(self.open_input_file("text"))
        self.assertEqual(v, "word")
        self.assertRaises(TypeError, IMP.test_ifile, "notafile" )
        s=StringIO("hi there")
        v= IMP.test_ifile(s)
        self.assertEqual(v, "hithere")

    def test_odata_directory(self):
        """Test that conversions to output files work properly"""
        IMP.set_log_level(IMP.MEMORY)
        IMP.test_ofile("ofile_test")
        self.assertRaises(IOError, IMP.test_ofile, "nodir/hi" )
        f= open("hi", "w")
        IMP.test_ofile(f)
        s = StringIO()
        IMP.test_ofile(s)
        self.assert_(s.getvalue().startswith("hi\n"))
        self.assertRaises(TypeError, IMP.test_ofile, 1)
        class NoMethods(object):
            pass
        self.assertRaises(TypeError, IMP.test_ofile, NoMethods)
        del f
        os.unlink('ofile_test')
        os.unlink('hi')

if __name__ == '__main__':
    unittest.main()
