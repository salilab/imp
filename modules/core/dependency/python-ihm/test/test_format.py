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

class GenericHandler(object):
    """Capture mmCIF data as a simple list of dicts"""
    def __init__(self):
        self.data = []

    def __call__(self, data):
        self.data.append(data)


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

    def _check_bad_cif(self, cif, category_handlers={}):
        """Ensure that the given bad cif results in a parser error"""
        r = ihm.format.CifReader(StringIO(cif), category_handlers)
        self.assertRaises(ihm.format.CifParserError, r.read_file)

    def test_comments_skipped(self):
        """Make sure that comments are skipped"""
        self._read_cif("# _exptl.method\n# ;foo\n", {})

    def test_missing_semicolon(self):
        """Make sure that missing semicolon is handled in multiline strings"""
        self._check_bad_cif("_exptl.method\n;foo\n")

    def test_missing_single_quote(self):
        """Make sure that missing single quote is handled"""
        self._check_bad_cif("_exptl.method 'foo\n")
        self._check_bad_cif("_exptl.method\n'foo'bar\n")
        self._check_bad_cif("loop_\n_exptl.method\n'foo\n")

    def test_missing_double_quote(self):
        """Make sure that missing double quote is handled"""
        self._check_bad_cif('_exptl.method "foo\n')
        self._check_bad_cif('_exptl.method "foo"bar\n')
        self._check_bad_cif('loop_\n_exptl.method\n"foo\n')

    def test_nested_loop(self):
        """Loop constructs cannot be nested"""
        self._check_bad_cif('loop_ loop_\n')

    def test_malformed_key(self):
        """Keys must be of the form _abc.xyz"""
        self._check_bad_cif('_category\n')
        self._check_bad_cif('loop_\n_atom_site\n')

    def test_missing_value(self):
        """Key without a value should be an error"""
        h = GenericHandler()
        # Checks aren't done unless we have a handler for the category
        self._check_bad_cif('_exptl.method\n', {'_exptl':h})

    def test_loop_mixed_categories(self):
        """Test bad mmCIF loop with a mix of categories"""
        self._check_bad_cif('loop_\n_atom_site.id\n_foo.bar\n')
        self._check_bad_cif('loop_\n_foo.bar\n_atom_site.id\n')

    def _read_cif(self, cif, category_handlers):
        r = ihm.format.CifReader(StringIO(cif), category_handlers)
        r.read_file()

    def test_category_case_insensitive(self):
        """Categories and keywords should be case insensitive"""
        for cat in ('_exptl.method', '_Exptl.METHod'):
            h = GenericHandler()
            self._read_cif(cat + ' foo', {'_exptl':h})
        self.assertEqual(h.data, [{'method':'foo'}])

    def test_omitted_ignored(self):
        """CIF omitted value ('.') should be ignored"""
        h = GenericHandler()
        self._read_cif("_foo.bar 1\n_foo.baz .\n", {'_foo':h})
        self.assertEqual(h.data, [{'bar':'1'}])

        h = GenericHandler()
        self._read_cif("loop_\n_foo.bar\n_foo.baz\n1 .\n", {'_foo':h})
        self.assertEqual(h.data, [{'bar':'1'}])

    def test_multiline(self):
        """Check that multiline strings are handled correctly"""
        self._check_bad_cif("_struct_keywords.pdbx_keywords\n"
                            ";COMPLEX \n(HYDROLASE/PEPTIDE)\n")

        # multiline in category
        h = GenericHandler()
        self._read_cif("_struct_keywords.pdbx_keywords\n"
                       ";COMPLEX \n(HYDROLASE/PEPTIDE)\n;",
                       {'_struct_keywords':h})
        self.assertEqual(h.data,
                [{'pdbx_keywords':'COMPLEX \n(HYDROLASE/PEPTIDE)'}])

        # multiline in loop
        h = GenericHandler()
        self._read_cif("loop_ _struct_keywords.pdbx_keywords\n"
                       ";COMPLEX \n(HYDROLASE/PEPTIDE)\n;",
                       {'_struct_keywords':h})
        self.assertEqual(h.data,
                [{'pdbx_keywords':'COMPLEX \n(HYDROLASE/PEPTIDE)'}])

    def test_ignored_loop(self):
        """Check that loops are ignored if they don't have a handler"""
        h = GenericHandler()
        self._read_cif("loop_\n_struct_keywords.pdbx_keywords\nfoo",
                       {'_atom_site':h})
        self.assertEqual(h.data, [])

    def test_quotes_in_strings(self):
        """Check that quotes in strings are handled"""
        h = GenericHandler()
        self._read_cif("_struct_keywords.pdbx_keywords 'foo'bar'",
                       {'_struct_keywords':h})
        self.assertEqual(h.data, [{'pdbx_keywords':"foo'bar"}])

        h = GenericHandler()
        self._read_cif('_struct_keywords.pdbx_keywords "foo"bar"  ',
                       {'_struct_keywords':h})
        self.assertEqual(h.data, [{'pdbx_keywords':'foo"bar'}])

    def test_wrong_loop_data_num(self):
        """Check wrong number of loop data elements"""
        h = GenericHandler()
        self._check_bad_cif("""
loop_
_atom_site.x
_atom_site.y
oneval
""", {'_atom_site':h})

    def test_first_data_block(self):
        """Only information from the first data block should be read"""
        h = GenericHandler()
        cif = StringIO("""
_foo.var1 test1
data_model
_foo.var2 test2
data_model2
_foo.var3 test3
""")

        r = ihm.format.CifReader(cif, {'_foo':h})
        # Read to end of first data block
        self.assertTrue(r.read_file())
        self.assertEqual(h.data, [{'var1':'test1', 'var2':'test2'}])

        # Read to end of second data block
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [{'var3':'test3'}])

        # No more data blocks
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [])


if __name__ == '__main__':
    unittest.main()
