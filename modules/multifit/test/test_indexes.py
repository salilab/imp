import IMP
import os
import IMP.test
import IMP.multifit
from IMP.multifit import indexes

class Tests(IMP.test.TestCase):

    def test_indexes_help(self):
        """Test indexes module help"""
        self.check_runnable_python_module("IMP.multifit.indexes")

    def test_indexes_usage(self):
        """Test indexes module incorrect usage"""
        r = self.run_python_module("IMP.multifit.indexes", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_indexes_run(self):
        """Test indexes module run"""
        indexes_input = self.get_input_file_name('indexes_test.input')
        input_dir = os.path.dirname(indexes_input)
        self.run_python_module(indexes,
                  ['foo', indexes_input,
                   '10', 'foo.indexes.mapping.input'])
        lines = open('foo.indexes.mapping.input').readlines()
        lines = [x.rstrip('\r\n') for x in lines]
        self.assertEqual(len(lines), 3)
        self.assertEqual(lines[0][:9], '|anchors|')
        self.assertEqual(os.path.realpath(lines[0][9:-1]),
                         os.path.realpath(input_dir))
        self.assertEqual(lines[1], '|protein|A|foo.A.fit.indexes.txt|')
        self.assertEqual(lines[2], '|protein|B|foo.B.fit.indexes.txt|')
        self.assert_indexes_file('foo.A.fit.indexes.txt', range(10))
        self.assert_indexes_file('foo.B.fit.indexes.txt', range(10))
        os.unlink('foo.A.fit.indexes.txt')
        os.unlink('foo.B.fit.indexes.txt')
        os.unlink('foo.indexes.mapping.input')

    def assert_indexes_file(self, fn, contents):
        lines = open(fn).readlines()
        lines = [int(x.rstrip('\r\n')) for x in lines]
        self.assertEqual(lines, list(contents))

if __name__ == '__main__':
    IMP.test.main()
