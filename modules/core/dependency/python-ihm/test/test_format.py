import utils
import os
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.format

class StringWriter(object):
    def __init__(self):
        self.fh = StringIO()
    def _repr(self, val):
        return repr(val)
    def getvalue(self):
        return self.fh.getvalue()


class Tests(unittest.TestCase):
    def test_line_writer_wrap(self):
        """Test LineWriter class line wrap"""
        writer = StringWriter()
        lw = ihm.format._LineWriter(writer, line_len=15)
        lw.write("foo")
        self.assertEqual(writer.getvalue(), "'foo'")
        lw.write("bar")
        self.assertEqual(writer.getvalue(), "'foo' 'bar'")
        lw.write("baz")
        self.assertEqual(writer.getvalue(), "'foo' 'bar'\n'baz'")

    def test_line_writer_multiline(self):
        """Test LineWriter class given a multiline string"""
        writer = StringWriter()
        lw = ihm.format._LineWriter(writer, line_len=15)
        lw.write("foo\nbar\nbaz")
        self.assertEqual(writer.getvalue(), "\n;foo\nbar\nbaz\n;\n")

    def test_line_writer_multiline_nl_term(self):
        """Test LineWriter class given a newline-terminated multiline string"""
        writer = StringWriter()
        lw = ihm.format._LineWriter(writer, line_len=15)
        lw.write("foo\nbar\nbaz\n")
        self.assertEqual(writer.getvalue(), "\n;foo\nbar\nbaz\n;\n")

    def test_category(self):
        """Test CategoryWriter class"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as l:
            l.write(bar='baz')
        self.assertEqual(fh.getvalue(), "foo.bar baz\n")

    def test_category_none(self):
        """Test CategoryWriter class with value=None"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as l:
            l.write(bar=None)
        self.assertEqual(fh.getvalue(), "foo.bar .\n")

    def test_empty_loop(self):
        """Test LoopWriter class with no values"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.loop('foo', ["bar", "baz"]) as l:
            pass
        self.assertEqual(fh.getvalue(), "")

    def test_loop(self):
        """Test LoopWriter class"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.loop('foo', ["bar", "baz"]) as l:
            l.write(bar='x')
            l.write(bar=None, baz='z')
            l.write(baz='y')
        self.assertEqual(fh.getvalue(), """#
loop_
foo.bar
foo.baz
x .
. z
. y
#
""")

    def test_loop_special_chars(self):
        """Test LoopWriter class with keys containing special characters"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.loop('foo', ["matrix[1][1]"]) as l:
            l.write(matrix11='x')
        self.assertEqual(fh.getvalue(), """#
loop_
foo.matrix[1][1]
x
#
""")

    def test_write_comment(self):
        """Test CifWriter.write_comment()"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        writer.write_comment('X' * 85)
        self.assertEqual(fh.getvalue(), "# " + "X" * 78 + '\n# ' + "X" * 7
                         + '\n')

    def test_repr(self):
        """Test CifWriter._repr()"""
        w = ihm.format.CifWriter(None)
        self.assertEqual(w._repr('foo'), 'foo')
        self.assertEqual(w._repr('fo"o'), "'fo\"o'")
        self.assertEqual(w._repr("fo'o"), '"fo\'o"')
        self.assertEqual(w._repr('foo bar'), "'foo bar'")
        self.assertEqual(w._repr(42.123456), '42.123')
        self.assertEqual(w._repr(False), 'NO')
        self.assertEqual(w._repr(True), 'YES')
        if sys.version_info[0] == 2:
            self.assertEqual(w._repr(long(4)), '4')
        # data_ should be quoted to distinguish from data blocks
        self.assertEqual(w._repr('data_foo'), "'data_foo'")
        self.assertEqual(w._repr('data_'), "'data_'")
        # [ is a reserved character and cannot start a nonquoted string
        self.assertEqual(w._repr('[foo'), "'[foo'")
        # Reserved words must be quoted (but just a prefix is OK)
        for word in ('save', 'loop', 'stop', 'global'):
            self.assertEqual(w._repr('%s_foo' % word), '%s_foo' % word)
            self.assertEqual(w._repr('%s_' % word), "'%s_'" % word)


if __name__ == '__main__':
    unittest.main()
