"""Utility classes to handle CIF format.

   This module provides classes to read in and write out mmCIF files. It is
   only concerned with handling syntactically correct CIF - it does not know
   the set of tables or the mapping to ihm objects. For that,
   see :mod:`ihm.dumper` for writing and :mod:`ihm.reader` for reading.

   See also the `stream parser example <https://github.com/ihmwg/python-ihm/blob/main/examples/stream_parser.py>`_.
"""  # noqa: E501

from __future__ import print_function
import sys
import textwrap
import operator
# getargspec is deprecated in Python 3, but getfullargspec has a very
# similar interface
try:
    from inspect import getfullargspec as getargspec
except ImportError:
    from inspect import getargspec
import re
try:
    from . import _format
except ImportError:
    _format = None

# Python 3 has no 'long' type, so use 'int' instead
if sys.version_info[0] >= 3:
    _long_type = int
else:
    _long_type = long   # noqa: F821


def _write_multiline(val, fh):
    fh.write("\n;")
    fh.write(val)
    if not val.endswith('\n'):
        fh.write("\n")
    fh.write(";\n")


class _LineWriter(object):
    def __init__(self, writer, line_len=80):
        self.writer = writer
        self.line_len = line_len
        self.column = 0

    def write(self, val):
        if isinstance(val, str) and '\n' in val:
            _write_multiline(val, self.writer.fh)
            self.column = 0
            return
        val = '.' if val is None else self.writer._repr(val)
        if self.column > 0:
            if self.column + len(val) + 1 > self.line_len:
                self.writer.fh.write("\n")
                self.column = 0
            else:
                self.writer.fh.write(" ")
                self.column += 1
        self.writer.fh.write(val)
        self.column += len(val)


class _CifCategoryWriter(object):
    def __init__(self, writer, category):
        self.writer = writer
        self.category = category

    def write(self, **kwargs):
        self.writer._write(self.category, kwargs)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass


class _CifLoopWriter(object):
    def __init__(self, writer, category, keys):
        self.writer = writer
        self.category = category
        self.keys = keys
        # Remove characters that we can't use in Python identifiers
        self.python_keys = [k.replace('[', '').replace(']', '') for k in keys]
        self._empty_loop = True

    def write(self, **kwargs):
        if self._empty_loop:
            f = self.writer.fh
            f.write("#\nloop_\n")
            for k in self.keys:
                f.write("%s.%s\n" % (self.category, k))
            self._empty_loop = False
        lw = _LineWriter(self.writer)
        for k in self.python_keys:
            lw.write(kwargs.get(k, None))
        self.writer.fh.write("\n")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if not self._empty_loop:
            self.writer.fh.write("#\n")


class _Writer(object):
    """Base class for all writers"""

    omitted = '.'
    unknown = '?'

    _boolmap = {False: 'NO', True: 'YES'}

    def __init__(self, fh):
        self.fh = fh


class CifWriter(_Writer):
    """Write information to a CIF file.
       The constructor takes a single argument - a Python filelike object
       to write to - and provides methods to write Python objects to
       that file. Most simple Python types are supported (string, float,
       bool, int). The Python bool type is mapped to CIF strings
       'NO' and 'YES'. Floats are always represented with 3 decimal places
       (or in scientific notation with 3 digits of precision if smaller
       than 1e-3); if a different amount of precision is desired, convert
       the float to a string first."""

    def flush(self):
        # noop - data is written as it is encountered
        pass

    def start_block(self, name):
        """Start a new data block in the file with the given name."""
        self.fh.write('data_%s\n' % name)

    def end_block(self):
        # noop - mmCIF has no end-of-block indicator
        pass

    def category(self, category):
        """Return a context manager to write a CIF category.
           A CIF category is a simple list of key:value pairs.

           :param str category: the name of the category
                                (e.g. "_struct_conf_type").
           :return: an object with a single method `write` which takes
                    keyword arguments.

           For example::

               with writer.category("_struct_conf_type") as l:
                   l.write(id='HELX_P', criteria=writer.unknown)
           """
        return _CifCategoryWriter(self, category)

    def loop(self, category, keys):
        """Return a context manager to write a CIF loop.

           :param str category: the name of the category
                                (e.g. "_struct_conf")
           :param list keys: the field keys in that category
           :return: an object with a single method `write` which takes
                    keyword arguments; this can be called any number of
                    times to add entries to the loop. Any field keys in `keys`
                    that are not provided as arguments to `write`, or values
                    that are the Python value `None`, will get the CIF
                    omitted value ('.'), while arguments to `write` that
                    are not present in `keys` will be ignored.

           For example::

               with writer.loop("_struct_conf", ["id", "conf_type_id"]) as l:
                   for i in range(5):
                       l.write(id='HELX_P1%d' % i, conf_type_id='HELX_P')
           """
        return _CifLoopWriter(self, category, keys)

    def write_comment(self, comment):
        """Write a simple comment to the CIF file.
           The comment will be wrapped if necessary for readability."""
        for line in textwrap.wrap(comment, 78):
            self.fh.write('# ' + line + '\n')

    def _write(self, category, kwargs):
        for key, val in sorted(kwargs.items(), key=operator.itemgetter(0)):
            if isinstance(val, str) and '\n' in val:
                self.fh.write("%s.%s" % (category, key))
                _write_multiline(val, self.fh)
            else:
                self.fh.write("%s.%s %s\n" % (category, key,
                                              self.omitted if val is None
                                              else self._repr(val)))

    def _repr(self, obj):
        if isinstance(obj, str) and '"' not in obj \
           and "'" not in obj and " " not in obj \
           and len(obj) > 0 \
           and not obj.startswith('_') \
           and not obj.startswith('global_') \
           and not obj.startswith('[') \
           and obj[:5] not in ('data_', 'save_', 'loop_', 'stop_', '?', '.'):
            return obj
        elif isinstance(obj, float):
            if abs(obj) < 1e-3:
                return "%.3g" % obj
            else:
                return "%.3f" % obj
        elif isinstance(obj, bool):
            return self._boolmap[obj]
        # Don't use repr(x) if type(x) == long since that adds an 'L' suffix,
        # which isn't valid mmCIF syntax. _long_type = long only on Python 2.
        elif isinstance(obj, _long_type):
            return "%d" % obj
        else:
            return repr(obj)


# Acceptable 'whitespace' characters in CIF
_WHITESPACE = set(" \t")


class CifParserError(Exception):
    """Exception raised for invalid format mmCIF files"""
    pass


class _Token(object):
    """A token in an mmCIF file"""
    pass


class _ValueToken(_Token):
    """The value of a variable in mmCIF"""
    pass


class _OmittedValueToken(_ValueToken):
    """A value that is deliberately omitted (the '.' string in mmCIF)"""
    pass


class _UnknownValueToken(_ValueToken):
    """A value that is unknown (the '?' string in mmCIF)"""
    pass


class _TextValueToken(_ValueToken):
    """The value of a variable in mmCIF as a piece of text"""
    __slots__ = ['txt']

    def __init__(self, txt):
        self.txt = txt


class _VariableToken(_Token):
    """A variable name, e.g. _entry.id, in mmCIF"""

    __slots__ = ['category', 'keyword']

    def __init__(self, val, linenum):
        # mmCIF categories and keywords are case insensitive, so make
        # everything lowercase
        self.category, _, self.keyword = val.lower().partition('.')
        if not self.category or not self.keyword:
            raise CifParserError("Malformed mmCIF variable name "
                                 "(%s) on line %d" % (val, linenum))


class _DataToken(_Token):
    """A data_* keyword in mmCIF, denoting a new data block"""
    pass


class _LoopToken(_Token):
    """A loop_ keyword in mmCIF, denoting the start of a loop construct"""
    pass


class _SaveToken(_Token):
    """A save_* keyword in mmCIF, denoting the start or end of a save frame"""
    pass


class _Reader(object):
    """Base class for reading a file and extracting some or all of its data."""

    def _add_category_keys(self):
        """Populate _keys for each category by inspecting its __call__
           method"""
        def python_to_cif(field):
            # Map valid Python identifiers to mmCIF keywords
            if field.startswith('tr_vector') or field.startswith('rot_matrix'):
                return re.sub(r'(\d)', r'[\1]', field)
            else:
                return field
        for h in self.category_handler.values():
            if not hasattr(h, '_keys'):
                h._keys = [python_to_cif(x)
                           for x in getargspec(h.__call__)[0][1:]]


class CifReader(_Reader):
    """Class to read an mmCIF file and extract some or all of its data.

       Use :meth:`read_file` to actually read the file.

       :param file fh: Open handle to the mmCIF file
       :param dict category_handler: A dict to handle data
              extracted from the file. Keys are category names
              (e.g. "_entry") and values are objects that have a `__call__`
              method and `not_in_file`, `omitted`, and `unknown` attributes.
              The names of the arguments to this `__call__` method
              are mmCIF keywords that are extracted from the file (for the
              keywords tr_vector[N] and rot_matrix[N][M] simply omit the [
              and ] characters, since these are not valid for Python
              identifiers). The object will be called with the data from
              the file as a set of strings, or `not_in_file`, `omitted` or
              `unkonwn` for any keyword that is not present in the file,
              the mmCIF omitted value (.), or mmCIF unknown value (?)
              respectively. (mmCIF keywords are case insensitive, so this
              class always treats them as lowercase regardless of the
              file contents.)
       :param unknown_category_handler: A callable (or `None`) that is called
              for each category in the file that isn't handled; it is given
              two arguments: the name of the category, and the line in the
              file at which the category was encountered (if known, otherwise
              None).
       :param unknown_keyword_handler: A callable (or `None`) that is called
              for each keyword in the file that isn't handled (within a
              category that is handled); it is given three arguments:
              the names of the category and keyword, and the line in the
              file at which the keyword was encountered (if known,
              otherwise None).
    """
    def __init__(self, fh, category_handler, unknown_category_handler=None,
                 unknown_keyword_handler=None):
        if _format is not None:
            c_file = _format.ihm_file_new_from_python(fh)
            self._c_format = _format.ihm_reader_new(c_file)
        self.category_handler = category_handler
        self.unknown_category_handler = unknown_category_handler
        self.unknown_keyword_handler = unknown_keyword_handler
        self._category_data = {}
        self.fh = fh
        self._tokens = []
        self._token_index = 0
        self._linenum = 0

    # Read a line from the file. Treat it as ASCII (not Unicode)
    # but be tolerant of 8-bit characters by assuming latin-1 encoding
    if sys.version_info[0] == 2:
        def _read_line(self):
            return self.fh.readline()
    else:
        def _read_line(self):
            line = self.fh.readline()
            if isinstance(line, bytes):
                return line.decode('latin-1')
            else:
                return line

    def __del__(self):
        if hasattr(self, '_c_format'):
            _format.ihm_reader_free(self._c_format)

    def _read_multiline_token(self, first_line, ignore_multiline):
        """Read a semicolon-delimited (multiline) token"""
        lines = [first_line[1:]]  # Skip initial semicolon
        start_linenum = self._linenum
        while True:
            self._linenum += 1
            nextline = self._read_line()
            if nextline == '':
                raise CifParserError(
                    "End of file while reading multiline "
                    "string which started on line %d" % start_linenum)
            elif nextline.startswith(';'):
                # Strip last newline
                lines[-1] = lines[-1].rstrip('\r\n')
                self._tokens = [_TextValueToken("".join(lines))]
                return
            elif not ignore_multiline:
                lines.append(nextline)

    def _handle_quoted_token(self, line, strlen, start_pos, quote_type):
        """Given the start of a quoted string, find the end and add a token
           for it"""
        quote = line[start_pos]
        # Get the next quote that is followed by whitespace (or line end).
        # In mmCIF a quote within a string is not considered an end quote as
        # long as it is not followed by whitespace.
        end = start_pos
        while True:
            end = line.find(quote, end + 1)
            if end == -1:
                raise CifParserError("%s-quoted string not terminated "
                                     "at line %d"
                                     % (quote_type, self._linenum))
            elif end == strlen - 1 or line[end + 1] in _WHITESPACE:
                # A quoted string is always a literal string, even if it is
                # "?" or ".", not an unknown/omitted value
                self._tokens.append(_TextValueToken(line[start_pos + 1:end]))
                return end + 1  # Step past the closing quote

    def _extract_line_token(self, line, strlen, start_pos):
        """Extract the next token from the given line starting at start_pos,
           populating self._tokens. The new start_pos is returned."""
        # Skip initial whitespace
        while start_pos < strlen and line[start_pos] in _WHITESPACE:
            start_pos += 1
        if start_pos >= strlen:
            return strlen
        if line[start_pos] == '"':
            return self._handle_quoted_token(line, strlen, start_pos, "Double")
        elif line[start_pos] == "'":
            return self._handle_quoted_token(line, strlen, start_pos, "Single")
        elif line[start_pos] == "#":
            # Comment - discard the rest of the line
            return strlen
        else:
            # Find end of token (whitespace or end of line)
            end_pos = start_pos
            while end_pos < strlen and line[end_pos] not in _WHITESPACE:
                end_pos += 1
            val = line[start_pos:end_pos]
            if val == 'loop_':
                tok = _LoopToken()
            elif val.startswith('data_'):
                tok = _DataToken()
            elif val.startswith('save_'):
                tok = _SaveToken()
            elif val.startswith('_'):
                tok = _VariableToken(val, self._linenum)
            elif val == '.':
                tok = _OmittedValueToken()
            elif val == '?':
                tok = _UnknownValueToken()
            else:
                # Note that we do no special processing for other reserved
                # words (global_, save_, stop_). But the probability of
                # them occurring where we expect a value is pretty small.
                tok = _TextValueToken(val)  # don't alter case of values
            self._tokens.append(tok)
            return end_pos

    def _tokenize(self, line):
        """Break up a line into tokens, populating self._tokens"""
        self._tokens = []
        if line.startswith('#'):
            return  # Skip comment lines
        start_pos = 0
        strlen = len(line)
        while start_pos < strlen:
            start_pos = self._extract_line_token(line, strlen, start_pos)

    def _unget_token(self):
        """Push back the last token returned by _get_token() so it can
           be read again"""
        self._token_index -= 1

    def _get_token(self, ignore_multiline=False):
        """Get the next :class:`_Token` from an mmCIF file, or None
           on end of file.
           If ignore_multiline is TRUE, the string contents of any multiline
           value tokens (those that are semicolon-delimited) are not stored
           in memory.
        """
        while len(self._tokens) <= self._token_index:
            # No tokens left - read the next non-blank line in
            self._linenum += 1
            line = self._read_line()
            if line == '':  # End of file
                return
            if line.startswith(';'):
                self._read_multiline_token(line, ignore_multiline)
            else:
                self._tokenize(line.rstrip('\r\n'))
            self._token_index = 0
        self._token_index += 1
        return self._tokens[self._token_index - 1]

    def _read_value(self, vartoken):
        """Read a line that sets a single value, e.g. "_entry.id   1YTI"""
        # Only read the value if we're interested in this category and key
        if vartoken.category in self.category_handler:
            if vartoken.keyword \
               in self.category_handler[vartoken.category]._keys:
                valtoken = self._get_token()
                if isinstance(valtoken, _ValueToken):
                    ch = self.category_handler[vartoken.category]
                    if vartoken.category not in self._category_data:
                        self._category_data[vartoken.category] = {}
                    if isinstance(valtoken, _OmittedValueToken):
                        val = ch.omitted
                    elif isinstance(valtoken, _UnknownValueToken):
                        val = ch.unknown
                    else:
                        val = valtoken.txt
                    self._category_data[vartoken.category][vartoken.keyword] \
                        = val
                else:
                    raise CifParserError(
                        "No valid value found for %s.%s on line %d"
                        % (vartoken.category, vartoken.keyword, self._linenum))
            elif self.unknown_keyword_handler is not None:
                self.unknown_keyword_handler(vartoken.category,
                                             vartoken.keyword, self._linenum)
        elif self.unknown_category_handler is not None:
            self.unknown_category_handler(vartoken.category, self._linenum)

    def _read_loop_keywords(self):
        """Read the set of keywords for a loop_ construct"""
        category = None
        keywords = []
        first_line = None
        keyword_lines = []
        while True:
            token = self._get_token()
            if isinstance(token, _VariableToken):
                if category is None:
                    category = token.category
                    first_line = self._linenum
                elif category != token.category:
                    raise CifParserError(
                        "mmCIF files cannot contain multiple "
                        "categories within a single loop at line %d"
                        % self._linenum)
                keywords.append(token.keyword)
                keyword_lines.append(self._linenum)
            elif isinstance(token, _ValueToken):
                # OK, end of keywords; proceed on to values
                self._unget_token()
                return category, keywords, keyword_lines, first_line
            else:
                raise CifParserError("Was expecting a keyword or value for "
                                     "loop at line %d" % self._linenum)

    def _read_loop_data(self, handler, num_wanted_keys, keyword_indices):
        """Read the data for a loop_ construct"""
        data = [handler.not_in_file] * num_wanted_keys
        while True:
            for i, index in enumerate(keyword_indices):
                token = self._get_token()
                if isinstance(token, _ValueToken):
                    if index >= 0:
                        if isinstance(token, _OmittedValueToken):
                            data[index] = handler.omitted
                        elif isinstance(token, _UnknownValueToken):
                            data[index] = handler.unknown
                        else:
                            data[index] = token.txt
                elif i == 0:
                    # OK, end of the loop
                    self._unget_token()
                    return
                else:
                    raise CifParserError(
                        "Wrong number of data values in loop "
                        "(should be an exact multiple of the number "
                        "of keys) at line %d" % self._linenum)
            handler(*data)

    def _read_loop(self):
        """Handle a loop_ construct"""
        (category, keywords,
            keyword_lines, first_line) = self._read_loop_keywords()
        # Skip data if we don't have a handler for it
        if category in self.category_handler:
            ch = self.category_handler[category]
            wanted_key_index = {}
            for i, k in enumerate(ch._keys):
                wanted_key_index[k] = i
            indices = [wanted_key_index.get(k, -1) for k in keywords]
            if self.unknown_keyword_handler is not None:
                for k, i, line in zip(keywords, indices, keyword_lines):
                    if i == -1:
                        self.unknown_keyword_handler(category, k, line)
            self._read_loop_data(ch, len(ch._keys), indices)
        elif self.unknown_category_handler is not None:
            self.unknown_category_handler(category, first_line)

    def read_file(self):
        """Read the file and extract data.
           Category handlers will be called as data becomes available -
           for ``loop_`` constructs, this will be once for each row in the
           loop; for categories (e.g. ``_entry.id model``), this will be once
           at the very end of the file.

           If the C-accelerated _format module is available, then it is used
           instead of the (much slower) Python tokenizer.

           :exc:`CifParserError` will be raised if the file cannot be parsed.

           :return: True iff more data blocks are available to be read.
        """
        self._add_category_keys()
        if hasattr(self, '_c_format'):
            return self._read_file_c()

        def call_all_categories():
            for cat, data in self._category_data.items():
                ch = self.category_handler[cat]
                ch(*[data.get(k, ch.not_in_file) for k in ch._keys])
            # Clear category data for next call to read_file()
            self._category_data = {}
        ndata = 0
        in_save = False
        while True:
            token = self._get_token(ignore_multiline=True)
            if token is None:
                break
            if isinstance(token, _VariableToken):
                self._read_value(token)
            elif isinstance(token, _DataToken):
                ndata += 1
                # Only read the first data block
                if ndata > 1:
                    # Allow reading the next data block
                    self._unget_token()
                    break
            elif isinstance(token, _LoopToken):
                self._read_loop()
                # Did we hit the end of the file?
                if self._token_index < 0:
                    break
            elif isinstance(token, _SaveToken):
                in_save = not in_save
                if not in_save:
                    call_all_categories()
                    for handler in self.category_handler.values():
                        handler.end_save_frame()
        call_all_categories()
        return ndata > 1

    def _read_file_c(self):
        """Read the file using the C parser"""
        _format.ihm_reader_remove_all_categories(self._c_format)
        for category, handler in self.category_handler.items():
            func = getattr(handler, '_add_c_handler', None) \
                or _format.add_category_handler
            func(self._c_format, category, handler._keys, handler)
        if self.unknown_category_handler is not None:
            _format.add_unknown_category_handler(self._c_format,
                                                 self.unknown_category_handler)
        if self.unknown_keyword_handler is not None:
            _format.add_unknown_keyword_handler(self._c_format,
                                                self.unknown_keyword_handler)
        try:
            eof, more_data = _format.ihm_read_file(self._c_format)
        except _format.FileFormatError as exc:
            # Convert to the same exception used by the Python code
            raise CifParserError(str(exc))
        return more_data != 0
