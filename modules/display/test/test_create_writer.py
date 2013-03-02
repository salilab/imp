import IMP
import IMP.test
import IMP.core
import IMP.display
import os

class Tests(IMP.test.TestCase):
    def test_3(self):
        """Testing the writer creater"""
        wchimera= IMP.display.create_writer(self.get_tmp_file_name("test.py"))
        wcgo= IMP.display.create_writer(self.get_tmp_file_name("test.pym"))
        wbild= IMP.display.create_writer(self.get_tmp_file_name("test.bild"))
        #wvrml= IMP.display.create_writer(self.get_tmp_file_name("test.vrml"))

if __name__ == '__main__':
    IMP.test.main()
