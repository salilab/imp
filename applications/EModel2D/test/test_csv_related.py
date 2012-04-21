import IMP
import IMP.test
import sys
import os
import random
import add_parent
add_parent.add_parent_to_python_path()

import csv_related


class TestCSVrelated(IMP.test.TestCase):

    def test_functions(self):
        """ Test the helper functions in csv_related """
        fn = self.get_input_file_name("csv.txt")
        rows = csv_related.read_csv(fn)
        self.assertEqual(len(rows), 6)
        rows = csv_related.read_csv_keyword(fn, "id")
        self.assertEqual(len(rows), 3)

if __name__ == '__main__':
    IMP.test.main()
