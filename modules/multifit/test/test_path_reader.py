import sys
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.algebra
import time

class Tests(IMP.test.TestCase):

    def test_read_paths(self):
        paths=IMP.multifit.read_paths(self.get_input_file_name("1z5s_8_18_anchors_5.txt"))
        print len(paths)
        self.assertEqual(len(paths),684)

if __name__ == '__main__':
    IMP.test.main()
