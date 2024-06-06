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

try:
    from ihm import _format
except ImportError:
    _format = None


class GenericHandler(object):
    """Capture mmCIF data as a simple list of dicts"""
    not_in_file = None
    omitted = None
    unknown = ihm.unknown

    _keys = ('method', 'foo', 'bar', 'baz', 'pdbx_keywords', 'var1',
             'var2', 'var3')

    def __init__(self):
        self.data = []

    def __call__(self, *args):
        d = {}
        for k, v in zip(self._keys, args):
            if v is not None:
                d[k] = v
        self.data.append(d)

    def end_save_frame(self):
        self.data.append('SAVE')


class _TestFinalizeHandler(GenericHandler):
    if _format is not None:
        _add_c_handler = _format._test_finalize_callback


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
        with writer.category('foo') as loc:
            loc.write(bar='baz')
        self.assertEqual(fh.getvalue(), "foo.bar baz\n")

    def test_category_none(self):
        """Test CategoryWriter class with value=None"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as loc:
            loc.write(bar=None)
        self.assertEqual(fh.getvalue(), "foo.bar .\n")

    def test_category_literal_dot(self):
        """Test CategoryWriter class with literal value=."""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as loc:
            loc.write(bar='.')
        self.assertEqual(fh.getvalue(), "foo.bar '.'\n")

    def test_category_unknown(self):
        """Test CategoryWriter class with value=unknown"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as loc:
            loc.write(bar=ihm.unknown)
        self.assertEqual(fh.getvalue(), "foo.bar ?\n")

    def test_category_literal_question(self):
        """Test CategoryWriter class with literal value=?"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as loc:
            loc.write(bar='?')
        self.assertEqual(fh.getvalue(), "foo.bar '?'\n")

    def test_category_multiline(self):
        """Test CategoryWriter class with multiline value"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.category('foo') as loc:
            loc.write(bar='line1\nline2')
        self.assertEqual(fh.getvalue(), "foo.bar\n;line1\nline2\n;\n")

    def test_empty_loop(self):
        """Test LoopWriter class with no values"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.loop('foo', ["bar", "baz"]):
            pass
        self.assertEqual(fh.getvalue(), "")

    def test_loop(self):
        """Test LoopWriter class"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.loop('foo', ["bar", "baz"]) as loc:
            loc.write(bar='x')
            loc.write(bar=None, baz='z')
            loc.write(baz='y')
            loc.write(bar=ihm.unknown, baz='z')
            loc.write(bar="?", baz=".")
        self.assertEqual(fh.getvalue(), """#
loop_
foo.bar
foo.baz
x .
. z
. y
? z
'?' '.'
#
""")

    def test_loop_special_chars(self):
        """Test LoopWriter class with keys containing special characters"""
        fh = StringIO()
        writer = ihm.format.CifWriter(fh)
        with writer.loop('foo', ["matrix[1][1]"]) as loc:
            loc.write(matrix11='x')
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

    def test_write_comment_unwrapped(self):
        """Test CifWriter.write_comment() with line wrapping disabled"""
        fh = StringIO()
        try:
            ihm.format.CifWriter._set_line_wrap(False)
            writer = ihm.format.CifWriter(fh)
            writer.write_comment('X' * 85)
        finally:
            ihm.format.CifWriter._set_line_wrap(True)
        self.assertEqual(fh.getvalue(), "# " + "X" * 85 + '\n')

    def test_repr(self):
        """Test CifWriter._repr()"""
        w = ihm.format.CifWriter(None)
        self.assertEqual(w._repr('foo'), 'foo')
        self.assertEqual(w._repr('fo"o'), "'fo\"o'")
        self.assertEqual(w._repr("fo'o"), '"fo\'o"')
        self.assertEqual(w._repr('foo bar'), "'foo bar'")
        self.assertEqual(w._repr(42.123456), '42.123')
        self.assertEqual(w._repr(0.000123456), '0.000123')
        self.assertEqual(w._repr(0.00000123456), '1.23e-06')
        self.assertEqual(w._repr(False), 'NO')
        self.assertEqual(w._repr(True), 'YES')
        if sys.version_info[0] == 2:
            self.assertEqual(w._repr(long(4)), '4')    # noqa: F821
        # data_ should be quoted to distinguish from data blocks
        self.assertEqual(w._repr('data_foo'), "'data_foo'")
        self.assertEqual(w._repr('data_'), "'data_'")
        # [ is a reserved character and cannot start a nonquoted string
        self.assertEqual(w._repr('[foo'), "'[foo'")
        # _ indicates an identifier and cannot start a nonquoted string
        self.assertEqual(w._repr('_foo'), "'_foo'")
        # Empty string must be quoted
        self.assertEqual(w._repr(""), "''")
        # Reserved words cannot start a nonquoted string
        for word in ('save', 'loop', 'stop', 'global'):
            self.assertEqual(w._repr('%s_foo' % word), "'%s_foo'" % word)
            self.assertEqual(w._repr('%s_' % word), "'%s_'" % word)
        # Literal ? must be quoted to distinguish from the unknown value
        self.assertEqual(w._repr('?foo'), "?foo")
        self.assertEqual(w._repr('?'), "'?'")
        # Literal . must be quoted to distinguish from the omitted value
        self.assertEqual(w._repr('.foo'), ".foo")
        self.assertEqual(w._repr('.'), "'.'")

    def test_reader_base(self):
        """Test Reader base class"""
        _ = ihm.format._Reader()  # noop

    def _check_bad_cif(self, cif, real_file, category_handlers={}):
        """Ensure that the given bad cif results in a parser error"""
        if real_file:
            with utils.temporary_directory() as tmpdir:
                fname = os.path.join(tmpdir, 'test')
                with open(fname, 'w') as fh:
                    fh.write(cif)
                with open(fname) as fh:
                    r = ihm.format.CifReader(fh, category_handlers)
                    self.assertRaises(ihm.format.CifParserError, r.read_file)
        else:
            r = ihm.format.CifReader(StringIO(cif), category_handlers)
            self.assertRaises(ihm.format.CifParserError, r.read_file)

    def test_comments_start_line_skipped(self):
        """Make sure that comments at start of line are skipped"""
        for real_file in (True, False):
            self._read_cif("# _exptl.method\n# ;foo\n", real_file, {})

    def test_comments_mid_line_skipped(self):
        """Make sure that comments part way through line are skipped"""
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif('_exptl.method #bar baz\nfoo', real_file,
                           {'_exptl': h})
            self.assertEqual(h.data, [{'method': 'foo'}])

    def test_missing_semicolon(self):
        """Make sure that missing semicolon is handled in multiline strings"""
        for real_file in (True, False):
            self._check_bad_cif("_exptl.method\n;foo\n", real_file)

    def test_missing_single_quote(self):
        """Make sure that missing single quote is handled"""
        for real_file in (True, False):
            self._check_bad_cif("_exptl.method 'foo\n", real_file)
            self._check_bad_cif("_exptl.method\n'foo'bar\n", real_file)
            self._check_bad_cif("loop_\n_exptl.method\n'foo\n", real_file)

    def test_missing_double_quote(self):
        """Make sure that missing double quote is handled"""
        for real_file in (True, False):
            self._check_bad_cif('_exptl.method "foo\n', real_file)
            self._check_bad_cif('_exptl.method "foo"bar\n', real_file)
            self._check_bad_cif('loop_\n_exptl.method\n"foo\n', real_file)

    def test_nested_loop(self):
        """Loop constructs cannot be nested"""
        for real_file in (True, False):
            self._check_bad_cif('loop_ loop_\n', real_file)

    def test_malformed_key(self):
        """Keys must be of the form _abc.xyz"""
        for real_file in (True, False):
            self._check_bad_cif('_category\n', real_file)
            self._check_bad_cif('loop_\n_atom_site\n', real_file)

    def test_missing_value(self):
        """Key without a value should be an error"""
        for real_file in (True, False):
            h = GenericHandler()
            # Checks aren't done unless we have a handler for the category
            self._check_bad_cif('_exptl.method\n', real_file, {'_exptl': h})

    def test_loop_mixed_categories(self):
        """Test bad mmCIF loop with a mix of categories"""
        for real_file in (True, False):
            h = GenericHandler()
            self._check_bad_cif('loop_\n_atom_site.id\n_foo.bar\n',
                                real_file, {'_atom_site': h})
            self._check_bad_cif('loop_\n_foo.bar\n_atom_site.id\n',
                                real_file, {'_foo': h})

    def _read_cif(self, cif, real_file, category_handlers,
                  unknown_category_handler=None,
                  unknown_keyword_handler=None):
        if real_file:
            with utils.temporary_directory() as tmpdir:
                fname = os.path.join(tmpdir, 'test')
                with open(fname, 'w') as fh:
                    fh.write(cif)
                with open(fname) as fh:
                    r = ihm.format.CifReader(fh, category_handlers,
                                             unknown_category_handler,
                                             unknown_keyword_handler)
                    r.read_file()
        else:
            r = ihm.format.CifReader(StringIO(cif), category_handlers,
                                     unknown_category_handler,
                                     unknown_keyword_handler)
            r.read_file()

    def test_category_case_insensitive(self):
        """Categories and keywords should be case insensitive"""
        for real_file in (True, False):
            for cat in ('_exptl.method', '_Exptl.METHod'):
                h = GenericHandler()
                self._read_cif(cat + ' foo', real_file, {'_exptl': h})
            self.assertEqual(h.data, [{'method': 'foo'}])

    def test_duplicated_key(self):
        """If a key is duplicated, we take the final value"""
        cif = "_exptl.method foo\n_exptl.method bar\n"
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif(cif, real_file, {'_exptl': h})
            self.assertEqual(h.data, [{'method': 'bar'}])

    def test_duplicated_key_omitted(self):
        """If a key is duplicated, we take the final (omitted) value"""
        cif = "_exptl.method foo\n_exptl.method .\n"
        for real_file in (True, False):
            h = GenericHandler()
            h.omitted = 'OMIT'
            self._read_cif(cif, real_file, {'_exptl': h})
            self.assertEqual(h.data, [{'method': 'OMIT'}])

    def test_duplicated_key_unknown(self):
        """If a key is duplicated, we take the final (unknown) value"""
        cif = "_exptl.method foo\n_exptl.method ?\n"
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif(cif, real_file, {'_exptl': h})
            self.assertEqual(h.data, [{'method': ihm.unknown}])

    def test_save_frames(self):
        """Category handlers should be called for each save frame"""
        cif = """
save_foo
_exptl.method foo
save_

save_bar
_exptl.method bar
save_
"""
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif(cif, real_file, {'_exptl': h})
            self.assertEqual(h.data, [{'method': 'foo'}, 'SAVE',
                                      {'method': 'bar'}, 'SAVE'])

    def test_omitted_ignored(self):
        """CIF omitted value ('.') should be ignored"""
        for real_file in (True, False):
            h = GenericHandler()
            # Omitted value is a literal . - anything else (quoted, or
            # a longer string) should be reported as a string
            self._read_cif("_foo.bar .1\n_foo.baz .\n"
                           "_foo.var1 '.'\n_foo.var2 \".\"\n",
                           real_file, {'_foo': h})
            self.assertEqual(h.data, [{'bar': '.1', 'var1': '.', 'var2': '.'}])

            h = GenericHandler()
            self._read_cif("loop_\n_foo.bar\n_foo.baz\n_foo.var1\n_foo.var2\n"
                           ".1 . '.' \".\"\n", real_file, {'_foo': h})
            self.assertEqual(h.data, [{'bar': '.1', 'var1': '.', 'var2': '.'}])

    def test_omitted_explicit(self):
        """Check explicit handling of CIF omitted value ('.')"""
        for real_file in (True, False):
            h = GenericHandler()
            h.omitted = 'OMIT'
            self._read_cif("_foo.bar .1\n_foo.baz .\n"
                           "_foo.var1 '.'\n_foo.var2 \".\"\n",
                           real_file, {'_foo': h})
            self.assertEqual(h.data, [{'baz': 'OMIT', 'bar': '.1',
                                       'var1': '.', 'var2': '.'}])

            h = GenericHandler()
            h.omitted = 'OMIT'
            self._read_cif("loop_\n_foo.bar\n_foo.baz\n_foo.var1\n_foo.var2\n"
                           ".1 . '.' \".\"\n", real_file, {'_foo': h})
            self.assertEqual(h.data, [{'baz': 'OMIT', 'bar': '.1',
                                       'var1': '.', 'var2': '.'}])

    def test_not_in_file_explicit(self):
        """Check explicit handling of keywords not in the file"""
        for real_file in (True, False):
            h = GenericHandler()
            h.not_in_file = 'NOT'
            self._read_cif("_foo.bar .1\n_foo.baz x\n", real_file, {'_foo': h})
            self.assertEqual(
                h.data,
                [{'var1': 'NOT', 'var3': 'NOT', 'var2': 'NOT',
                  'pdbx_keywords': 'NOT', 'bar': '.1', 'foo': 'NOT',
                  'method': 'NOT', 'baz': 'x'}])

            h = GenericHandler()
            h.not_in_file = 'NOT'
            self._read_cif("loop_\n_foo.bar\n_foo.baz\n.1 x\n", real_file,
                           {'_foo': h})
            self.assertEqual(
                h.data,
                [{'var1': 'NOT', 'var3': 'NOT', 'var2': 'NOT',
                  'pdbx_keywords': 'NOT', 'bar': '.1', 'foo': 'NOT',
                  'method': 'NOT', 'baz': 'x'}])

    def test_loop_linebreak(self):
        """Make sure that linebreaks are ignored in loop data"""
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif("loop_\n_foo.bar\n_foo.baz\n1\n2\n", real_file,
                           {'_foo': h})
            self.assertEqual(h.data, [{'bar': '1', 'baz': '2'}])

    def test_keyword_free(self):
        """Make sure keyword data is cleaned up"""
        for real_file in (True, False):
            h = GenericHandler()
            # The unterminated single quote will cause an exception so
            # the _exptl category is never handled, so the C parser relies
            # on ihm_keyword_free to free the memory
            self._check_bad_cif("_exptl.method foo\n'", real_file,
                                {'_exptl': h})

    def test_unknown(self):
        """CIF unknown value ('?') should be reported as-is"""
        for real_file in (True, False):
            h = GenericHandler()
            # Unknown value is a literal ? - anything else (quoted, or
            # a longer string) should be reported as a string
            self._read_cif("_foo.bar ?1\n_foo.baz ?\n"
                           "_foo.var1 '?'\n_foo.var2 \"?\"\n",
                           real_file, {'_foo': h})
            self.assertEqual(h.data, [{'bar': '?1', 'baz': ihm.unknown,
                                       'var1': '?', 'var2': '?'}])

            h = GenericHandler()
            self._read_cif("loop_\n_foo.bar\n_foo.baz\n_foo.var1\n_foo.var2\n"
                           "?1 ? '?' \"?\"\n", real_file,
                           {'_foo': h})
            self.assertEqual(h.data, [{'bar': '?1', 'baz': ihm.unknown,
                                       'var1': '?', 'var2': '?'}])

    def test_unknown_explicit(self):
        """Check explicit handling of CIF unknown value"""
        for real_file in (True, False):
            h = GenericHandler()
            h.unknown = 'UNK'
            self._read_cif("_foo.bar ?1\n_foo.baz ?\n"
                           "_foo.var1 '?'\n_foo.var2 \"?\"\n",
                           real_file, {'_foo': h})
            self.assertEqual(h.data, [{'bar': '?1', 'baz': 'UNK',
                                       'var1': '?', 'var2': '?'}])

            h = GenericHandler()
            h.unknown = 'UNK'
            self._read_cif("loop_\n_foo.bar\n_foo.baz\n_foo.var1\n_foo.var2\n"
                           "?1 ? '?' \"?\"\n", real_file,
                           {'_foo': h})
            self.assertEqual(h.data, [{'bar': '?1', 'baz': 'UNK',
                                       'var1': '?', 'var2': '?'}])

    def test_multiline(self):
        """Check that multiline strings are handled correctly"""
        for real_file in (True, False):
            self._check_bad_cif("_struct_keywords.pdbx_keywords\n"
                                ";COMPLEX \n(HYDROLASE/PEPTIDE)\n", real_file)

            # multiline in category
            h = GenericHandler()
            self._read_cif("_struct_keywords.pdbx_keywords\n"
                           ";COMPLEX \n(HYDROLASE/PEPTIDE)\n;",
                           real_file, {'_struct_keywords': h})
            self.assertEqual(
                h.data, [{'pdbx_keywords': 'COMPLEX \n(HYDROLASE/PEPTIDE)'}])

            # multiline in loop
            h = GenericHandler()
            self._read_cif("loop_ _struct_keywords.pdbx_keywords\n"
                           "_struct_keywords.foo\n"
                           ";COMPLEX \n(HYDROLASE/PEPTIDE)\n;\nbar\n",
                           real_file, {'_struct_keywords': h})
            self.assertEqual(
                h.data,
                [{'pdbx_keywords': 'COMPLEX \n(HYDROLASE/PEPTIDE)',
                  'foo': 'bar'}])

    def test_ignored_loop(self):
        """Check that loops are ignored if they don't have a handler"""
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif("loop_\n_struct_keywords.pdbx_keywords\nfoo",
                           real_file, {'_atom_site': h})
            self.assertEqual(h.data, [])

    def test_quotes_in_strings(self):
        """Check that quotes in strings are handled"""
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif("_struct_keywords.pdbx_keywords 'foo'bar'",
                           real_file, {'_struct_keywords': h})
            self.assertEqual(h.data, [{'pdbx_keywords': "foo'bar"}])

            h = GenericHandler()
            self._read_cif('_struct_keywords.pdbx_keywords "foo"bar"  ',
                           real_file, {'_struct_keywords': h})
            self.assertEqual(h.data, [{'pdbx_keywords': 'foo"bar'}])

    def test_wrong_loop_data_num(self):
        """Check wrong number of loop data elements"""
        for real_file in (True, False):
            h = GenericHandler()
            self._check_bad_cif("""
loop_
_atom_site.x
_atom_site.y
oneval
""", real_file, {'_atom_site': h})

    def test_first_data_block(self):
        """Only information from the first data block should be read"""
        cif = """
_foo.var1 test1
data_model
_foo.var2 test2
data_model2
_foo.var3 test3
"""
        h = GenericHandler()
        r = ihm.format.CifReader(StringIO(cif), {'_foo': h})
        self._check_first_data(r, h)

        with utils.temporary_directory() as tmpdir:
            fname = os.path.join(tmpdir, 'test')
            with open(fname, 'w') as fh:
                fh.write(cif)
            with open(fname) as fh:
                h = GenericHandler()
                r = ihm.format.CifReader(fh, {'_foo': h})
                self._check_first_data(r, h)

    def _check_first_data(self, r, h):
        # Read to end of first data block
        self.assertTrue(r.read_file())
        self.assertEqual(h.data, [{'var1': 'test1', 'var2': 'test2'}])

        # Read to end of second data block
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [{'var3': 'test3'}])

        # No more data blocks
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [])

    def test_eof_after_loop_data(self):
        """Make sure EOF after loop data is handled"""
        for real_file in (True, False):
            h = GenericHandler()
            self._read_cif("""
loop_
_foo.bar
_foo.baz
x y
#
""", real_file, {'_foo': h})

    def test_finalize_handler(self):
        """Make sure that C parser finalize callback works"""
        for real_file in (True, False):
            h = _TestFinalizeHandler()
            self._read_cif("# _exptl.method foo\n", real_file, {'_exptl': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_file_new_python_no_read_method(self):
        """Test ihm_file_new_from_python with object with no read method"""
        self.assertRaises(AttributeError, _format.ihm_file_new_from_python,
                          None)

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_python_read_exception(self):
        """Test exception in read callback is handled"""
        class MyError(Exception):
            pass

        class MyFileLike(object):
            def read(self, numbytes):
                raise MyError("some error")
        fh = MyFileLike()
        f = _format.ihm_file_new_from_python(fh)
        reader = _format.ihm_reader_new(f)
        self.assertRaises(MyError, _format.ihm_read_file, reader)
        _format.ihm_reader_free(reader)

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_python_read_not_string(self):
        """Test that read() returning an invalid type is handled"""
        class MyFileLike(object):
            def read(self, numbytes):
                return 42
        fh = MyFileLike()
        f = _format.ihm_file_new_from_python(fh)
        reader = _format.ihm_reader_new(f)
        self.assertRaises(ValueError, _format.ihm_read_file, reader)
        _format.ihm_reader_free(reader)

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_python_read_too_long(self):
        """Test that read() returning too many bytes is handled"""
        class MyFileLike(object):
            def read(self, numbytes):
                return " " * (numbytes * 4 + 10)
        fh = MyFileLike()
        f = _format.ihm_file_new_from_python(fh)
        reader = _format.ihm_reader_new(f)
        self.assertRaises(ValueError, _format.ihm_read_file, reader)
        _format.ihm_reader_free(reader)

    @unittest.skipIf(_format is None or sys.platform == 'win32',
                     "No C tokenizer, or Windows")
    def test_fd_read_failure(self):
        """Test handling of C read() failure"""
        f = open('/dev/null')
        os.close(f.fileno())  # Force read from file descriptor to fail
        r = ihm.format.CifReader(f, {})
        self.assertRaises(IOError, r.read_file)

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_python_read_bytes(self):
        """Test read() returning bytes (binary file)"""
        class MyFileLike(object):
            def __init__(self):
                self.calls = 0

            def read(self, numbytes):
                self.calls += 1
                if self.calls == 1:
                    return b"_exptl.method foo"
                else:
                    return b""
        h = GenericHandler()
        r = ihm.format.CifReader(MyFileLike(), {'_exptl': h})
        r.read_file()
        self.assertEqual(h.data, [{'method': 'foo'}])

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_python_read_unicode(self):
        """Test read() returning Unicode (text file)"""
        class MyFileLike(object):
            def __init__(self):
                self.calls = 0

            def read(self, numbytes):
                self.calls += 1
                if self.calls == 1:
                    return u"_exptl.method foo"
                else:
                    return u""
        h = GenericHandler()
        r = ihm.format.CifReader(MyFileLike(), {'_exptl': h})
        r.read_file()
        self.assertEqual(h.data, [{'method': 'foo'}])

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_line_endings(self):
        """Check that C tokenizer works with different line endings"""
        # todo: the Python tokenizer should handle the same endings

        for end in ('\n', '\r', '\r\n', '\0'):
            h = GenericHandler()
            self._read_cif("_struct_keywords.pdbx_keywords\n"
                           ";COMPLEX %s(HYDROLASE/PEPTIDE)%s;" % (end, end),
                           False, {'_struct_keywords': h})
            self.assertEqual(
                h.data, [{'pdbx_keywords': 'COMPLEX \n(HYDROLASE/PEPTIDE)'}])

    def test_unknown_category_ignored(self):
        """Test that unknown categories are just ignored"""
        h = GenericHandler()
        self._read_cif("""
_cat1.foo baz
_cat2.bar test
#
loop_
_foo.bar
_foo.baz
x y
""", False, {'_cat1': h})
        self.assertEqual(h.data, [{'foo': 'baz'}])

    def test_unknown_category_handled(self):
        """Test that unknown categories are handled if requested"""
        class CatHandler(object):
            def __init__(self):
                self.warns = []

            def __call__(self, cat, line):
                self.warns.append((cat, line))

        ch = CatHandler()
        h = GenericHandler()
        self._read_cif("""
_cat1.foo baz
_cat2.bar test
#
loop_
_foo.bar
_foo.baz
x y
""", False, {'_cat1': h}, unknown_category_handler=ch)
        self.assertEqual(h.data, [{'foo': 'baz'}])
        self.assertEqual(ch.warns, [('_cat2', 3), ('_foo', 6)])

    def test_unknown_keyword_ignored(self):
        """Test that unknown keywords are just ignored"""
        h = GenericHandler()
        self._read_cif("""
_cat1.foo baz
_cat1.unknown_keyword1 test
#
loop_
_foo.bar
_foo.unknown_keyword2
x y
""", False, {'_cat1': h, '_foo': h})
        self.assertEqual(h.data, [{'bar': 'x'}, {'foo': 'baz'}])

    def test_unknown_keyword_handled(self):
        """Test that unknown keywords are handled if requested"""
        class KeyHandler(object):
            def __init__(self):
                self.warns = []

            def __call__(self, cat, key, line):
                self.warns.append((cat, key, line))

        kh = KeyHandler()
        h = GenericHandler()
        self._read_cif("""
_cat1.foo baz
_cat1.unknown_keyword1 test
#
loop_
_foo.bar
_foo.unknown_keyword2
x y
""", False, {'_cat1': h, '_foo': h}, unknown_keyword_handler=kh)
        self.assertEqual(h.data, [{'bar': 'x'}, {'foo': 'baz'}])
        self.assertEqual(kh.warns,
                         [('_cat1', 'unknown_keyword1', 3),
                          ('_foo', 'unknown_keyword2', 7)])

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_multiple_set_unknown_handler(self):
        """Test setting unknown handler multiple times"""
        class Handler(object):
            def __call__(self):
                pass
        uc = Handler()
        fh = StringIO()
        c_file = _format.ihm_file_new_from_python(fh)
        reader = _format.ihm_reader_new(c_file)
        # Handler must be a callable object
        self.assertRaises(ValueError, _format.add_unknown_category_handler,
                          reader, None)
        self.assertRaises(ValueError, _format.add_unknown_keyword_handler,
                          reader, None)
        _format.add_unknown_category_handler(reader, uc)
        _format.add_unknown_category_handler(reader, uc)
        _format.add_unknown_keyword_handler(reader, uc)
        _format.add_unknown_keyword_handler(reader, uc)
        _format.ihm_reader_remove_all_categories(reader)
        _format.ihm_reader_remove_all_categories(reader)
        _format.ihm_reader_free(reader)
        fh.close()

    def test_preserving_tokenizer_get_token(self):
        """Test _PreservingCifTokenizer._get_token()"""
        cif = """
# Full line comment
_cat1.Foo baz    # End of line comment
"""
        t = ihm.format._PreservingCifTokenizer(StringIO(cif))
        tokens = [t._get_token() for _ in range(11)]
        self.assertIsInstance(tokens[0], ihm.format._EndOfLineToken)
        self.assertIsInstance(tokens[1], ihm.format._CommentToken)
        self.assertEqual(tokens[1].txt, ' Full line comment')
        self.assertIsInstance(tokens[2], ihm.format._EndOfLineToken)
        self.assertIsInstance(tokens[3], ihm.format._PreservingVariableToken)
        self.assertEqual(tokens[3].category, '_cat1')
        self.assertEqual(tokens[3].keyword, 'foo')
        self.assertEqual(tokens[3].orig_keyword, 'Foo')
        self.assertIsInstance(tokens[4], ihm.format._WhitespaceToken)
        self.assertEqual(tokens[4].txt, ' ')
        self.assertIsInstance(tokens[5], ihm.format._TextValueToken)
        self.assertEqual(tokens[5].txt, 'baz')
        self.assertIsInstance(tokens[6], ihm.format._WhitespaceToken)
        self.assertEqual(tokens[6].txt, '    ')
        self.assertIsInstance(tokens[7], ihm.format._CommentToken)
        self.assertEqual(tokens[7].txt, ' End of line comment')
        self.assertIsInstance(tokens[8], ihm.format._EndOfLineToken)
        self.assertIsNone(tokens[9])
        self.assertIsNone(tokens[10])

        # Make sure we can reconstruct the original mmCIF from the tokens
        new_cif = "".join(x.as_mmcif() for x in tokens[:-2])
        self.assertEqual(new_cif, cif)

    def test_preserving_tokenizer_reconstruct(self):
        """Make sure _PreservingCifTokenizer can reconstruct original mmCIF"""
        cif = """
data_foo_bar
#
_cat1.foo ?
#
_cat2.BaR .
#
loop_
foo.bar
foo.baz
foo.single
foo.double
foo.multi
x . 'single' "double"
;multi
;
"""
        t = ihm.format._PreservingCifTokenizer(StringIO(cif))
        tokens = []
        while True:
            tok = t._get_token()
            if tok is None:
                break
            else:
                tokens.append(tok)
        new_cif = "".join(x.as_mmcif() for x in tokens)
        self.assertEqual(new_cif, cif)

    def test_preserving_variable_token(self):
        """Test _PreservingVariableToken class"""
        t = ihm.format._PreservingVariableToken("foo.BAR", 1)
        self.assertEqual(t.keyword, 'bar')
        self.assertEqual(t.orig_keyword, 'BAR')
        self.assertEqual(t.as_mmcif(), 'foo.BAR')
        t.keyword = 'baz'
        self.assertEqual(t.as_mmcif(), 'foo.baz')

    def test_preserving_cif_reader(self):
        """Test _PreservingCifReader class"""
        cif = """
data_foo_bar
#
_cat1.foo ?
#
loop_
_foo.bar
_foo.baz
a b c d
x y
"""
        r = ihm.format._PreservingCifReader(StringIO(cif))
        tokens = list(r.read_file())
        self.assertIsInstance(tokens[5], ihm.format._CategoryTokenGroup)
        self.assertIsInstance(tokens[8], ihm.format._LoopHeaderTokenGroup)
        self.assertIsInstance(tokens[9], ihm.format._LoopRowTokenGroup)
        self.assertIsInstance(tokens[10], ihm.format._LoopRowTokenGroup)
        self.assertIsInstance(tokens[11], ihm.format._LoopRowTokenGroup)
        new_cif = "".join(x.as_mmcif() for x in tokens)
        self.assertEqual(new_cif, cif)

    def test_preserving_cif_reader_filter(self):
        """Test _PreservingCifReader class with filters"""
        cif = """
data_foo_bar
#
_cat1.bar old
#
loop_
_foo.bar
_foo.baz
a b c d
x y
"""
        r = ihm.format._PreservingCifReader(StringIO(cif))
        filters = [ihm.format._ChangeValueFilter(".bar", old='old', new='new'),
                   ihm.format._ChangeValueFilter(".bar", old='a', new='newa'),
                   ihm.format._ChangeValueFilter(".foo", old='old', new='new')]
        tokens = list(r.read_file(filters))
        new_cif = "".join(x.as_mmcif() for x in tokens)
        self.assertEqual(new_cif, """
data_foo_bar
#
_cat1.bar new
#
loop_
_foo.bar
_foo.baz
newa b c d
x y
""")

    def test_category_token_group(self):
        """Test CategoryTokenGroup class"""
        var = ihm.format._PreservingVariableToken("_foo.bar", 1)
        space = ihm.format._WhitespaceToken("   ")
        val = ihm.format._TextValueToken("baz", quote=None)
        tg = ihm.format._CategoryTokenGroup(
            var, ihm.format._SpacedToken([space], val))
        self.assertEqual(str(tg), "<_CategoryTokenGroup(_foo.bar, baz)>")
        self.assertEqual(tg.as_mmcif(), '_foo.bar   baz\n')
        self.assertEqual(tg.category, "_foo")
        self.assertEqual(tg.keyword, "bar")
        self.assertEqual(tg.value, "baz")
        tg.value = None
        self.assertIsNone(tg.value)

    def test_spaced_token(self):
        """Test SpacedToken class"""
        space = ihm.format._WhitespaceToken("   ")
        val = ihm.format._TextValueToken("baz", quote=None)
        sp = ihm.format._SpacedToken([space], val)
        self.assertEqual(sp.as_mmcif(), "   baz")
        self.assertEqual(sp.value, 'baz')
        sp.value = None
        self.assertIsNone(sp.value)
        self.assertEqual(sp.as_mmcif(), '   .')
        sp.value = ihm.unknown
        self.assertIs(sp.value, ihm.unknown)
        self.assertEqual(sp.as_mmcif(), '   ?')
        sp.value = "test value"
        self.assertEqual(sp.as_mmcif(), '   "test value"')

    def test_loop_header_token_group(self):
        """Test LoopHeaderTokenGroup class"""
        cif = """
loop_
_foo.bar
_foo.baz
x y
"""
        r = ihm.format._PreservingCifReader(StringIO(cif))
        token = list(r.read_file())[1]
        self.assertIsInstance(token, ihm.format._LoopHeaderTokenGroup)
        self.assertEqual(str(token),
                         "<_LoopHeaderTokenGroup(_foo, ['bar', 'baz'])>")
        self.assertEqual(token.keyword_index("bar"), 0)
        self.assertEqual(token.keyword_index("baz"), 1)
        self.assertRaises(ValueError, token.keyword_index, "foo")

    def test_change_value_filter_init(self):
        """Test ChangeValueFilter constructor"""
        f = ihm.format._ChangeValueFilter("_citation.id", old='1', new='2')
        self.assertEqual(f.category, '_citation')
        self.assertEqual(f.keyword, 'id')
        f = ihm.format._ChangeValueFilter(".bar", old='1', new='2')
        self.assertIsNone(f.category)
        self.assertEqual(f.keyword, 'bar')
        f = ihm.format._ChangeValueFilter("bar", old='1', new='2')
        self.assertIsNone(f.category)
        self.assertEqual(f.keyword, 'bar')

    def test_change_value_filter_category(self):
        """Test ChangeValueFilter.filter_category"""
        var = ihm.format._PreservingVariableToken("_foo.bar", 1)
        space = ihm.format._WhitespaceToken("   ")
        val = ihm.format._TextValueToken("baz", quote=None)
        tg = ihm.format._CategoryTokenGroup(
            var, ihm.format._SpacedToken([space], val))
        # Value does not match
        f = ihm.format._ChangeValueFilter("_foo.bar", old='old', new='new')
        new_tg = f.filter_category(tg)
        self.assertEqual(new_tg.value, 'baz')

        # Keyword does not match
        f = ihm.format._ChangeValueFilter("_foo.foo", old='baz', new='new')
        new_tg = f.filter_category(tg)
        self.assertEqual(new_tg.value, 'baz')

        # Category does not match
        f = ihm.format._ChangeValueFilter("_bar.bar", old='baz', new='new')
        new_tg = f.filter_category(tg)
        self.assertEqual(new_tg.value, 'baz')

        # Category matches exactly
        f = ihm.format._ChangeValueFilter("_foo.bar", old='baz', new='new')
        new_tg = f.filter_category(tg)
        self.assertEqual(new_tg.value, 'new')

        # All-category match
        f = ihm.format._ChangeValueFilter(".bar", old='new', new='new2')
        new_tg = f.filter_category(tg)
        self.assertEqual(new_tg.value, 'new2')

    def test_change_value_filter_loop(self):
        """Test ChangeValueFilter.get_loop_filter"""
        cif = """
loop_
_foo.bar
_foo.baz
x y
"""
        r = ihm.format._PreservingCifReader(StringIO(cif))
        tokens = list(r.read_file())
        header = tokens[1]
        row = tokens[2]
        # Keyword does not match
        f = ihm.format._ChangeValueFilter("_foo.foo", old='x', new='new')
        self.assertIsNone(f.get_loop_filter(header))

        # Category does not match
        f = ihm.format._ChangeValueFilter("_bar.bar", old='x', new='new')
        self.assertIsNone(f.get_loop_filter(header))

        # Value does not match
        f = ihm.format._ChangeValueFilter("_foo.bar", old='notx', new='new')
        lf = f.get_loop_filter(header)
        self.assertEqual(lf(row).as_mmcif(), "x y")

        # Category matches exactly
        f = ihm.format._ChangeValueFilter("_foo.bar", old='x', new='new')
        lf = f.get_loop_filter(header)
        self.assertEqual(lf(row).as_mmcif(), "new y")

        # All-category match
        f = ihm.format._ChangeValueFilter(".bar", old='new', new='new2')
        lf = f.get_loop_filter(header)
        self.assertEqual(lf(row).as_mmcif(), "new2 y")


if __name__ == '__main__':
    unittest.main()
