import IMP
import IMP.test
import IMP.em2d
import os

class Tests(IMP.test.TestCase):
    def test_reading_selection_files(self):
        """ Test reading selection files"""
        fn_selection=self.get_input_file_name("test_selection_file.sel")
        selection=IMP.em2d.read_selection_file(fn_selection)
        self.assertEqual(len(selection),5,"Wrong amount of " \
        "files read from selection file")
        names=["file1","file4","file5","file6","file9"]
        for i in range(0,len(names)):
            self.assertEqual(names[i],selection[i],
            "file %s does not match %s" % (names[i],selection[i]))

if __name__ == '__main__':
    IMP.test.main()
