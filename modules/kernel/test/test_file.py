import IMP
import IMP.test
import os.path
from StringIO import StringIO

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test that conversions to input files work properly"""
        print "starting"
        IMP.set_log_level(IMP.MEMORY)
        v= IMP.kernel._test_ifile(self.get_input_file_name("text"))
        self.assertEqual(v, "word")
        v= IMP.kernel._test_ifile(self.open_input_file("text"))
        self.assertEqual(v, "word")
        self.assertRaises(IOError, IMP.kernel._test_ifile, "notafile" )
        s=StringIO("hi there")
        v= IMP.kernel._test_ifile(s)
        self.assertEqual(v, "hithere")
        print "done"

    def test_odata_directory(self):
        """Test that conversions to output files work properly"""
        print "starting"
        IMP.set_log_level(IMP.MEMORY)
        IMP.kernel._test_ofile("ofile_test")
        self.assertRaises(IOError, IMP.kernel._test_ofile, "nodir/hi" )
        f= open("hi", "w")
        IMP.kernel._test_ofile(f)
        s = StringIO()
        IMP.kernel._test_ofile(s)
        self.assertTrue(s.getvalue().startswith("hi\n"))
        self.assertRaises(TypeError, IMP.kernel._test_ofile, 1)
        class NoMethods(object):
            pass
        self.assertRaises(TypeError, IMP.kernel._test_ofile, NoMethods)
        del f
        print "unlinking"
        #os.unlink('ofile_test')
        #os.unlink('hi')
        print "done"

    def test_odata_directory_2(self):
        """Test temporary files"""
        simple = IMP.create_temporary_file()
        print simple.get_name()
        prefix = IMP.create_temporary_file("imp")
        print prefix.get_name()
        self.assertIn("imp", prefix.get_name())
        suffix = IMP.create_temporary_file("imp", ".py")
        print suffix.get_name()
        self.assertIn("imp", suffix.get_name())
        self.assertIn(".py", suffix.get_name())

    def test_bad(self):
        """Test bad paths trigger IO exceptions with overloads"""
        self.assertRaises(IOError, IMP.kernel._test_ifile_overloaded, "bad path", "hi")
        s= StringIO()
        # shouldn't raise
        IMP.kernel._test_ifile_overloaded(s, "hi")
if __name__ == '__main__':
    IMP.test.main()
