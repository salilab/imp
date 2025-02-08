import utils
import os
import unittest
import sys
import struct
from io import BytesIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.format_bcif

try:
    from ihm import _format
except ImportError:
    _format = None


# Provide dummy implementations of msgpack.unpack() and msgpack.pack() which
# just return the data unchanged. We can use these to test the Python BinaryCIF
# parser with Python objects rather than having to install msgpack and
# generate real binary files
class MockMsgPack:
    @staticmethod
    def unpack(fh, raw=False):
        return fh

    @staticmethod
    def pack(data, fh, use_bin_type=True):
        fh.data = data


class MockFh:
    pass


class GenericHandler:
    """Capture BinaryCIF data as a simple list of dicts"""
    not_in_file = None
    omitted = None
    unknown = "?"

    _keys = ('method', 'foo', 'bar', 'baz', 'pdbx_keywords', 'var1',
             'var2', 'var3', 'intkey1', 'intkey2', 'floatkey1', 'floatkey2')
    _int_keys = frozenset(('intkey1', 'intkey2'))
    _float_keys = frozenset(('floatkey1', 'floatkey2'))

    def __init__(self):
        self.data = []

    def __call__(self, *args):
        d = {}
        for k, v in zip(self._keys, args):
            if v is not None:
                d[k] = v
        self.data.append(d)


def _encode_int(rows):
    d = {'data': struct.pack('%db' % len(rows), *rows),
         'encoding': [{'kind': 'ByteArray',
                       'type': ihm.format_bcif._Uint8}]}
    return d, None


def _encode_float(rows):
    d = {'data': struct.pack('<%dd' % len(rows), *rows),
         'encoding': [{'kind': 'ByteArray',
                       'type': ihm.format_bcif._Float64}]}
    return d, None


def _encode(rows):
    # Assume rows is a list of strings; make simple BinaryCIF encoding
    mask = [0] * len(rows)
    need_mask = False
    for i, row in enumerate(rows):
        if row is None:
            need_mask = True
            mask[i] = 1
        elif row == '?':
            need_mask = True
            mask[i] = 2
    if need_mask:
        rows = ['' if r == '?' or r is None else r for r in rows]
        mask = {'data': ''.join(chr(i) for i in mask).encode('ascii'),
                'encoding': [{'kind': 'ByteArray',
                              'type': ihm.format_bcif._Uint8}]}
    else:
        mask = None
    string_data = "".join(rows)

    offsets = []
    total_len = 0
    for row in rows:
        offsets.append(total_len)
        total_len += len(row)
    offsets.append(total_len)
    offsets = ''.join(chr(i) for i in offsets).encode('ascii')
    indices = ''.join(chr(i) for i in range(len(rows))).encode('ascii')
    string_array_encoding = {
        'kind': 'StringArray',
        'dataEncoding': [{'kind': 'ByteArray',
                          'type': ihm.format_bcif._Uint8}],
        'stringData': string_data,
        'offsetEncoding': [{'kind': 'ByteArray',
                            'type': ihm.format_bcif._Uint8}],
        'offsets': offsets}
    d = {'data': indices,
         'encoding': [string_array_encoding]}
    return d, mask


class Category:
    def __init__(self, name, data):
        self.name = name
        self.data = data

    def _encode_rows(self, rows):
        if len(rows) and isinstance(rows[0], int):
            return _encode_int(rows)
        elif len(rows) and isinstance(rows[0], float):
            return _encode_float(rows)
        else:
            return _encode(rows)

    def get_bcif(self):
        nrows = 0
        cols = []
        for name, rows in self.data.items():
            nrows = len(rows)
            data, mask = self._encode_rows(rows)
            cols.append({'mask': mask, 'name': name,
                         'data': data})
        return {'name': self.name,
                'columns': cols, 'rowCount': nrows}


class Block(list):
    pass


class _BadMsgPackType:
    pass


BAD_MSGPACK_TYPE = _BadMsgPackType()


def _add_msgpack(d, fh):
    """Add `d` to filelike object `fh` in msgpack format"""
    if isinstance(d, dict):
        fh.write(struct.pack('>Bi', 0xdf, len(d)))
        for key, val in d.items():
            _add_msgpack(key, fh)
            _add_msgpack(val, fh)
    elif isinstance(d, list):
        fh.write(struct.pack('>Bi', 0xdd, len(d)))
        for val in d:
            _add_msgpack(val, fh)
    elif isinstance(d, str):
        b = d.encode('utf8')
        fh.write(struct.pack('>Bi', 0xdb, len(b)))
        fh.write(b)
    elif isinstance(d, bytes):
        fh.write(struct.pack('>Bi', 0xc6, len(d)))
        fh.write(d)
    elif isinstance(d, int):
        fh.write(struct.pack('>Bi', 0xce, d))
    elif d is None:
        fh.write(b'\xc0')
    elif d is BAD_MSGPACK_TYPE:
        # 0xc1 is not used in msgpack
        fh.write(b'\xc1')
    else:
        raise TypeError("Cannot handle %s" % type(d))


def _make_bcif_file(blocks):
    blocks = [{'header': 'ihm',
               'categories': [c.get_bcif() for c in block]}
              for block in blocks]
    d = {'version': '0.1', 'encoder': 'python-ihm test suite',
         'dataBlocks': blocks}
    return _python_to_msgpack(d)


def _python_to_msgpack(d):
    if _format:
        # Convert Python object `d` into msgpack format for the C-accelerated
        # parser
        fh = BytesIO()
        _add_msgpack(d, fh)
        fh.seek(0)
        return fh
    else:
        # Pure Python reader uses mocked-out msgpack to work on `d` directly
        return d


class Tests(unittest.TestCase):
    def test_decoder_base(self):
        """Test Decoder base class"""
        d = ihm.format_bcif._Decoder()
        self.assertIsNone(d._kind)
        d(enc=None, data=None)  # noop

    def test_string_array_decoder(self):
        """Test StringArray decoder"""
        d = ihm.format_bcif._StringArrayDecoder()
        self.assertEqual(d._kind, 'StringArray')

        # Int8 is signed char (so FF is -1)
        enc = {'stringData': 'aAB',
               'dataEncoding': [{'kind': 'ByteArray',
                                 'type': ihm.format_bcif._Int8}],
               'offsetEncoding': [{'kind': 'ByteArray',
                                   'type': ihm.format_bcif._Int8}],
               'offsets': b'\x00\x01\x03'}
        data = b'\x00\x01\x00\xFF'

        data = d(enc, data)
        self.assertEqual(list(data), ['a', 'AB', 'a', None])

    def test_byte_array_decoder(self):
        """Test ByteArray decoder"""
        d = ihm.format_bcif._ByteArrayDecoder()
        self.assertEqual(d._kind, 'ByteArray')

        # type 1 (signed char)
        data = d({'type': ihm.format_bcif._Int8}, b'\x00\x01\xFF')
        self.assertEqual(list(data), [0, 1, -1])

        # type 2 (signed short)
        data = d({'type': ihm.format_bcif._Int16}, b'\x00\x01\x01\xAC')
        self.assertEqual(list(data), [256, -21503])

        # type 3 (signed int)
        data = d({'type': ihm.format_bcif._Int32}, b'\x00\x01\x01\x05')
        self.assertEqual(list(data), [83951872])

        # type 4 (unsigned char)
        data = d({'type': ihm.format_bcif._Uint8}, b'\x00\xFF')
        self.assertEqual(list(data), [0, 255])

        # type 5 (unsigned short)
        data = d({'type': ihm.format_bcif._Uint16}, b'\x00\x01\x01\xAC')
        self.assertEqual(list(data), [256, 44033])

        # type 6 (unsigned int)
        data = d({'type': ihm.format_bcif._Uint32}, b'\x00\x01\x01\xFF')
        self.assertEqual(list(data), [4278255872])

        # type 32 (32-bit float)
        data = d({'type': ihm.format_bcif._Float32}, b'\x00\x00(B')
        self.assertAlmostEqual(list(data)[0], 42.0, delta=0.1)

        # type 33 (64-bit float)
        data = d({'type': ihm.format_bcif._Float64},
                 b'\x00\x00\x00\x00\x00\x00E@')
        self.assertAlmostEqual(list(data)[0], 42.0, delta=0.1)

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_byte_array_decoder_full_file(self):
        """Test ByteArray decoder working on full BinaryCIF file"""
        class MyCategory(Category):
            def __init__(self, name, data, raw_data, data_type, enc=None):
                Category.__init__(self, name, data)
                self.raw_data, self.data_type = raw_data, data_type
                self.enc = enc

            def _encode_rows(self, rows):
                if self.enc is not None:
                    enc = self.enc
                else:
                    enc = [{'kind': 'ByteArray', 'type': self.data_type}]
                return {'data': self.raw_data,
                        'encoding': enc}, None

        def get_decoded(data_type, raw_data, enc=None):
            cat = MyCategory('_exptl', {'method': []}, raw_data, data_type,
                             enc=enc)
            h = GenericHandler()
            self._read_bcif([Block([cat])], {'_exptl': h})
            return [x['method'] for x in h.data]

        # type 3 (signed int)
        data = get_decoded(ihm.format_bcif._Int32, b'\x00\x01\x01\x05')
        self.assertEqual(data, ['83951872'])

        # Raw data not a multiple of type size
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Int16, b'\x00\x01\x01')
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Uint16, b'\x00\x01\x01')
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Int32, b'\x00\x01\x01')
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Uint32, b'\x00\x01\x01')
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Float64, b'\x00\x00\x00\x00')
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Float32, b'\x00\x00\x00')

        # ByteArray must take raw data, not the output of another decoder
        self.assertRaises(_format.FileFormatError, get_decoded,
                          ihm.format_bcif._Int32, b'\x00\x01\x01\x05',
                          enc=[{'kind': 'ByteArray',
                                'type': ihm.format_bcif._Int32}] * 2)

        # type 4 (unsigned char)
        data = get_decoded(ihm.format_bcif._Uint8, b'\x00\xFF')
        self.assertEqual(data, ['0', '255'])

        # type 33 (64-bit float)
        data = get_decoded(ihm.format_bcif._Float64,
                           b'\x00\x00\x00\x00\x00\x00E@')
        self.assertIsInstance(data[0], str)
        self.assertAlmostEqual(float(data[0]), 42.0, delta=0.1)

        # unsupported type
        self.assertRaises(_format.FileFormatError,
                          get_decoded, ihm.format_bcif._Float32, b'\x00\x00(B')

    def test_integer_packing_decoder_signed(self):
        """Test IntegerPacking decoder with signed data"""
        d = ihm.format_bcif._IntegerPackingDecoder()
        self.assertEqual(d._kind, 'IntegerPacking')

        # 1-byte data
        data = d({'isUnsigned': False, 'byteCount': 1},
                 [1, 2, -3, 127, 1, -128, -5])
        self.assertEqual(list(data), [1, 2, -3, 128, -133])

        # 2-byte data
        data = d({'isUnsigned': False, 'byteCount': 2},
                 [1, 2, -3, 32767, 1, -32768, -5])
        self.assertEqual(list(data), [1, 2, -3, 32768, -32773])

    def test_integer_packing_decoder_unsigned(self):
        """Test IntegerPacking decoder with unsigned data"""
        d = ihm.format_bcif._IntegerPackingDecoder()
        self.assertEqual(d._kind, 'IntegerPacking')

        # 1-byte data
        data = d({'isUnsigned': True, 'byteCount': 1},
                 [1, 2, 3, 127, 1, 255, 1])
        self.assertEqual(list(data), [1, 2, 3, 127, 1, 256])

        # 2-byte data
        data = d({'isUnsigned': True, 'byteCount': 2},
                 [1, 2, 3, 32767, 1, 65535, 5])
        self.assertEqual(list(data), [1, 2, 3, 32767, 1, 65540])

    def test_delta_decoder(self):
        """Test Delta decoder"""
        d = ihm.format_bcif._DeltaDecoder()
        self.assertEqual(d._kind, 'Delta')

        data = d({'origin': 1000}, [0, 3, 2, 1])
        self.assertEqual(list(data), [1000, 1003, 1005, 1006])

    def test_run_length_decoder(self):
        """Test RunLength decoder"""
        d = ihm.format_bcif._RunLengthDecoder()
        self.assertEqual(d._kind, 'RunLength')

        data = d({}, [1, 3, 2, 1, 3, 2])
        self.assertEqual(list(data), [1, 1, 1, 2, 3, 3])

    def test_fixed_point_decoder(self):
        """Test FixedPoint decoder"""
        d = ihm.format_bcif._FixedPointDecoder()
        self.assertEqual(d._kind, 'FixedPoint')

        data = list(d({'factor': 100}, [120, 123, 12]))
        self.assertEqual(len(data), 3)
        self.assertAlmostEqual(data[0], 1.20, delta=0.01)
        self.assertAlmostEqual(data[1], 1.23, delta=0.01)
        self.assertAlmostEqual(data[2], 0.12, delta=0.01)

    def test_decode(self):
        """Test _decode function"""
        data = b'\x01\x03\x02\x01\x03\x02'
        runlen = {'kind': 'RunLength'}
        bytearr = {'kind': 'ByteArray', 'type': ihm.format_bcif._Int8}
        data = ihm.format_bcif._decode(data, [runlen, bytearr])
        self.assertEqual(list(data), [1, 1, 1, 2, 3, 3])

    def _read_bcif(self, blocks, category_handlers,
                   unknown_category_handler=None,
                   unknown_keyword_handler=None):
        fh = _make_bcif_file(blocks)
        sys.modules['msgpack'] = MockMsgPack
        r = ihm.format_bcif.BinaryCifReader(fh, category_handlers,
                                            unknown_category_handler,
                                            unknown_keyword_handler)
        r.read_file()

    def test_category_case_insensitive(self):
        """Categories and keywords should be case insensitive"""
        cat1 = Category('_exptl', {'method': ['foo']})
        cat2 = Category('_Exptl', {'METHod': ['foo']})
        for cat in cat1, cat2:
            h = GenericHandler()
            self._read_bcif([Block([cat])], {'_exptl': h})
        self.assertEqual(h.data, [{'method': 'foo'}])

    def test_int_keys(self):
        """Check handling of integer keywords"""
        cat = Category('_foo', {'intkey1': [42]})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        self.assertEqual(h.data, [{'intkey1': 42}])

        # Float will be truncated to int
        cat = Category('_foo', {'intkey1': [42.34]})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        self.assertEqual(h.data, [{'intkey1': 42}])

        # Can coerce an intlike string to int
        cat = Category('_foo', {'intkey1': ["42"]})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        self.assertEqual(h.data, [{'intkey1': 42}])

        # Cannot coerce arbitrary string to int
        cat = Category('_foo', {'intkey1': ["some string"]})
        h = GenericHandler()
        self.assertRaises(ValueError, self._read_bcif,
                          [Block([cat])], {'_foo': h})

    def test_float_keys(self):
        """Check handling of floating-point keywords"""
        cat = Category('_foo', {'floatkey1': [42.340]})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        val = h.data[0]['floatkey1']
        self.assertIsInstance(val, float)
        self.assertAlmostEqual(val, 42.34, delta=0.01)

        # int will be coerced to float
        cat = Category('_foo', {'floatkey1': [42]})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        val = h.data[0]['floatkey1']
        self.assertIsInstance(val, float)
        self.assertAlmostEqual(val, 42.0, delta=0.01)

        # Can coerce a floatlike string to float
        cat = Category('_foo', {'floatkey1': ["42.340"]})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        val = h.data[0]['floatkey1']
        self.assertIsInstance(val, float)
        self.assertAlmostEqual(val, 42.34, delta=0.01)

        # Cannot coerce arbitary string to float
        cat = Category('_foo', {'floatkey1': ["some string"]})
        h = GenericHandler()
        self.assertRaises(ValueError, self._read_bcif,
                          [Block([cat])], {'_foo': h})

    def test_omitted_unknown(self):
        """Test handling of omitted/unknown data"""
        cat = Category('_foo',
                       {'var1': ['test1', '?', 'test2', None, 'test3']})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        self.assertEqual(h.data,
                         [{'var1': 'test1'}, {'var1': '?'},
                          {'var1': 'test2'}, {}, {'var1': 'test3'}])

    def _read_bcif_raw(self, d, category_handlers):
        fh = _python_to_msgpack(d)
        r = ihm.format_bcif.BinaryCifReader(fh, category_handlers)
        r.read_file()

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_read_header_c(self):
        """Test handling of various BinaryCIF headers"""
        # No header
        d = 42
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Header keys not strings
        d = {42: 50}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Data blocks not a list
        d = {'dataBlocks': 42}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Empty header
        d = {}
        self._read_bcif_raw(d, {})
        # Unknown keys should be ignored
        d = {'unknown-key': None}
        self._read_bcif_raw(d, {})
        # Skipped object of bad type
        d = {'unknown-keyword': BAD_MSGPACK_TYPE}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_bad_block(self):
        """Test handling of various bad BinaryCIF blocks"""
        # Block not a map
        d = {'dataBlocks': [42]}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Block keys not strings
        d = {'dataBlocks': [{42: 50}]}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Skipped object of bad type
        d = {'dataBlocks': [{'unknown-keyword': BAD_MSGPACK_TYPE}]}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_bad_categories(self):
        """Test handling of various bad BinaryCIF category lists"""
        # Categories not a list
        d = {'dataBlocks': [{'categories': 42}]}
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_bad_category(self):
        """Test handling of various bad BinaryCIF categories"""
        def make_bcif(c):
            return {'dataBlocks': [{'categories': [c]}]}

        # Category not a map
        d = make_bcif(42)
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Category keys not strings
        d = make_bcif({42: 50})
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Category name not a string
        d = make_bcif({'name': 42})
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})
        # Skipped object of bad type
        d = make_bcif({'name': 'bar', 'unknown-key': BAD_MSGPACK_TYPE})
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw, d, {})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_bad_columns(self):
        """Test handling of various bad BinaryCIF column lists"""
        def make_bcif(c):
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': c}]}]}

        # Columns are skipped
        d = make_bcif([])
        self._read_bcif_raw(d, {})

        # Columns not an array
        d = make_bcif(42)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_read_column(self):
        """Test handling of various BinaryCIF columns"""
        def make_bcif(c):
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}

        # Column not a map
        d = make_bcif(42)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})
        # Column keys not strings
        d = make_bcif({42: 50})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})
        # Column name not string
        d = make_bcif({'name': 50})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Skipped unknown key contains invalid data
        d = make_bcif({'name': 'bar',
                       'data': {'data': b'\x00\x00\x00\x00',
                                'encoding':
                                [{'kind': 'ByteArray',
                                  'type': ihm.format_bcif._Int32}]},
                       'unknown-key': BAD_MSGPACK_TYPE})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Normal usage; unknown keys should be skipped
        d = make_bcif({'name': 'bar',
                       'data': {'data': b'\x00\x00\x00\x00',
                                'encoding':
                                [{'kind': 'ByteArray',
                                  'type': ihm.format_bcif._Int32}]},
                       'unknown-key': None})
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_read_mask(self):
        """Test handling of BinaryCIF mask"""
        def make_bcif(mask):
            c = {'name': 'bar',
                 'data': {'data': b'\x00\x00\x00\x00',
                          'encoding':
                          [{'kind': 'ByteArray',
                            'type': ihm.format_bcif._Int32}]},
                 'mask': mask}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}
        # Bad mask type
        d = make_bcif('foo')
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Null mask
        d = make_bcif(None)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})

        # Mask of bad type
        d = make_bcif("foo")
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Map keys not strings
        d = make_bcif({42: 50})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Encodings not an array
        d = make_bcif({'encoding': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad data
        d = make_bcif({'data': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Skipped unknown key contains invalid data
        d = make_bcif({'unknown-key': BAD_MSGPACK_TYPE})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Unknown map keys should be ignored
        d = make_bcif({'unknown-key': None})
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})

        # Bad data type
        d = make_bcif({'data': b'\x00\x00\x01\x00\x02\x00',
                       'encoding': [{'kind': 'ByteArray',
                                     'type': ihm.format_bcif._Uint16}]})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Normal usage
        d = make_bcif({'data': b'\x00\x01\x02',
                       'encoding': [{'kind': 'ByteArray',
                                     'type': ihm.format_bcif._Uint8}]})
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_read_data(self):
        """Test handling of BinaryCIF data"""
        def make_bcif(data):
            c = {'name': 'bar',
                 'data': data}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}
        # Data not a map
        d = make_bcif('foo')
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Map keys not strings
        d = make_bcif({42: 50})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad data
        d = make_bcif({'data': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Skipped unknown key contains invalid data
        d = make_bcif({'unknown-key': BAD_MSGPACK_TYPE})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Unknown map keys should be ignored
        d = make_bcif({'unknown-key': None,
                       'data': b'\x00\x00\x00\x00',
                       'encoding': [{'kind': 'ByteArray',
                                     'type': ihm.format_bcif._Int32}]})
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_read_encoding(self):
        """Test handling of BinaryCIF encoding information"""
        def make_bcif(encoding):
            c = {'name': 'bar',
                 'data': {'data': b'\x00\x00\x00\x00',
                          'encoding': [encoding]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}
        # Encoding not a map
        d = make_bcif('foo')
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Map keys not strings
        d = make_bcif({42: 50})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad kind
        d = make_bcif({'kind': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad dataEncoding
        d = make_bcif({'dataEncoding': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad offsetEncoding
        d = make_bcif({'offsetEncoding': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad stringData
        d = make_bcif({'stringData': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad offsets
        d = make_bcif({'offsets': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad type
        d = make_bcif({'type': None})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Skipped unknown key contains invalid data
        d = make_bcif({'unknown-key': BAD_MSGPACK_TYPE})
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Normal usage; unknown keys should be ignored
        d = make_bcif({'kind': 'ByteArray',
                       'type': ihm.format_bcif._Int32,
                       'unknown-key': None})
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_string_array_encoding_c(self):
        """Test handling of various BinaryCIF StringArray encodings"""
        def make_bcif(data, data_type, offsets, offsets_type,
                      data_encoding='ByteArray'):
            c = {'name': 'bar',
                 'data': {'data': data,
                          'encoding':
                          [{'kind': 'StringArray', 'stringData': 'aAB',
                            'dataEncoding': [{'kind': data_encoding,
                                              'type': data_type}],
                            'offsetEncoding': [{'kind': 'ByteArray',
                                                'type': offsets_type}],
                            'offsets': offsets}]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}

        # Test normal usage
        for (data, data_type) in (
                (b'\x00\x01', ihm.format_bcif._Uint8),
                (b'\x00\x01', ihm.format_bcif._Int8),
                (b'\x00\x00\x01\x00', ihm.format_bcif._Uint16),
                (b'\x00\x00\x01\x00', ihm.format_bcif._Int16),
                (b'\x00\x00\x00\x00\x01\x00\x00\x00', ihm.format_bcif._Int32)):
            d = make_bcif(data=data, data_type=data_type,
                          offsets=b'\x00\x01\x03',
                          offsets_type=ihm.format_bcif._Uint8)
            h = GenericHandler()
            self._read_bcif_raw(d, {'_foo': h})
            self.assertEqual(h.data, [{'bar': 'a'}, {'bar': 'AB'}])

        # StringArray decoding can't be used for offset or data
        d = make_bcif(data=b'\x00\x01', data_type=ihm.format_bcif._Uint8,
                      offsets=b'\x00\x01\x03',
                      offsets_type=ihm.format_bcif._Uint8,
                      data_encoding='StringArray')
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Indices must be int, not float
        d = make_bcif(data=b'\x00\x00(B', data_type=ihm.format_bcif._Float32,
                      offsets=b'\x00\x01\x03',
                      offsets_type=ihm.format_bcif._Uint8)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Offsets must be int, not float
        d = make_bcif(data=b'\x00\x01', data_type=ihm.format_bcif._Uint8,
                      offsets=b'\x00\x00(B',
                      offsets_type=ihm.format_bcif._Float32)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Offsets must be in range of string data
        d = make_bcif(data=b'\x00\x01', data_type=ihm.format_bcif._Uint8,
                      offsets=b'\x00\x01\xcc',
                      offsets_type=ihm.format_bcif._Uint8)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Indices must be in range
        for data in (struct.pack('2b', 0, 40), struct.pack('b', -32)):
            d = make_bcif(data=data, data_type=ihm.format_bcif._Int8,
                          offsets=b'\x00\x01\x03',
                          offsets_type=ihm.format_bcif._Uint8)
            h = GenericHandler()
            self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                              d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_fixed_point_encoding_c(self):
        """Test handling of various BinaryCIF FixedPoint encodings"""
        def make_bcif(data, data_type, factor):
            c = {'name': 'bar',
                 'data': {'data': data,
                          'encoding':
                          [{'kind': 'FixedPoint', 'factor': factor},
                           {'kind': 'ByteArray', 'type': data_type}]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}

        # Test normal usage
        d = make_bcif(data=b'\xcc\x00\x00\x00',
                      data_type=ihm.format_bcif._Int32,
                      factor=100)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        bar = h.data[0]['bar']
        self.assertIsInstance(bar, str)
        self.assertAlmostEqual(float(bar), 2.04, delta=0.01)

        # Bad factor type
        d = make_bcif(data=b'\xcc\x00\x00\x00',
                      data_type=ihm.format_bcif._Int32,
                      factor='bad factor')
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad input type
        d = make_bcif(data=b'\xcc\x00',
                      data_type=ihm.format_bcif._Int16,
                      factor=100)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_run_length_encoding_c(self):
        """Test handling of various BinaryCIF RunLength encodings"""
        def make_bcif(data, data_type):
            c = {'name': 'bar',
                 'data': {'data': data,
                          'encoding':
                          [{'kind': 'RunLength'},
                           {'kind': 'ByteArray', 'type': data_type}]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}

        # Test normal usage
        d = make_bcif(data=b'\x05\x00\x00\x00\x03\x00\x00\x00',
                      data_type=ihm.format_bcif._Int32)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data, [{'bar': '5'}] * 3)

        # Bad input type
        d = make_bcif(data=b'\x05\x03', data_type=ihm.format_bcif._Int8)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_delta_encoding_c(self):
        """Test handling of various BinaryCIF Delta encodings"""
        def make_bcif(data, data_type, origin):
            c = {'name': 'bar',
                 'data': {'data': data,
                          'encoding':
                          [{'kind': 'Delta', 'origin': origin},
                           {'kind': 'ByteArray', 'type': data_type}]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}

        # Test normal usage
        d = make_bcif(data=b'\x05\x00\x00\x00\x03\x00\x00\x00',
                      data_type=ihm.format_bcif._Int32, origin=50)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data, [{'bar': '55'}, {'bar': '58'}])

        # Bad input type
        d = make_bcif(data=b'\x05\x03', data_type=ihm.format_bcif._Int8,
                      origin=50)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

        # Bad origin type
        d = make_bcif(data=b'\x05\x00\x00\x00\x03\x00\x00\x00',
                      data_type=ihm.format_bcif._Int32, origin='foo')
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_integer_packing_encoding_c(self):
        """Test handling of various BinaryCIF IntegerPacking encodings"""
        def make_bcif(data, data_type):
            c = {'name': 'bar',
                 'data': {'data': data,
                          'encoding':
                          [{'kind': 'IntegerPacking'},
                           {'kind': 'ByteArray', 'type': data_type}]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c]}]}]}

        # Test signed 8-bit input
        d = make_bcif(data=struct.pack('6b', 5, 127, 8, -30, -128, -10),
                      data_type=ihm.format_bcif._Int8)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data, [{'bar': '5'}, {'bar': '135'},
                                  {'bar': '-30'}, {'bar': '-138'}])

        # Test unsigned 8-bit input
        d = make_bcif(data=struct.pack('3B', 5, 255, 8),
                      data_type=ihm.format_bcif._Uint8)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data, [{'bar': '5'}, {'bar': '263'}])

        # Test signed 16-bit input
        d = make_bcif(data=struct.pack('<6h', 5, 32767, 8, -30, -32768, -10),
                      data_type=ihm.format_bcif._Int16)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data, [{'bar': '5'}, {'bar': '32775'},
                                  {'bar': '-30'}, {'bar': '-32778'}])

        # Test unsigned 16-bit input
        d = make_bcif(data=struct.pack('<3H', 5, 65535, 8),
                      data_type=ihm.format_bcif._Uint16)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data, [{'bar': '5'}, {'bar': '65543'}])

        # Bad input type
        d = make_bcif(data=struct.pack('<f', 42.0),
                      data_type=ihm.format_bcif._Float32)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_process_bcif_category_c(self):
        """Test processing of BinaryCIF category"""
        def make_bcif(data1, data2, data_type):
            c1 = {'name': 'bar',
                  'data': {'data': data1,
                           'encoding':
                           [{'kind': 'IntegerPacking'},
                            {'kind': 'ByteArray', 'type': data_type}]}}
            c2 = {'name': 'baz',
                  'data': {'data': data2,
                           'encoding':
                           [{'kind': 'IntegerPacking'},
                            {'kind': 'ByteArray', 'type': data_type}]}}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c1, c2]}]}]}

        class _ThrowHandler(GenericHandler):
            def __call__(self, *args):
                raise ValueError("some error")

        # Normal operation
        d = make_bcif(data1=struct.pack('2b', 1, 42),
                      data2=struct.pack('2b', 8, 4),
                      data_type=ihm.format_bcif._Int8)
        h = GenericHandler()
        self._read_bcif_raw(d, {'_foo': h})
        self.assertEqual(h.data,
                         [{'bar': '1', 'baz': '8'}, {'bar': '42', 'baz': '4'}])

        # Handler errors should be propagated
        h = _ThrowHandler()
        self.assertRaises(ValueError, self._read_bcif_raw, d, {'_foo': h})

        # Mismatched column size
        d = make_bcif(data1=struct.pack('3b', 1, 42, 9),
                      data2=struct.pack('2b', 8, 4),
                      data_type=ihm.format_bcif._Int8)
        h = GenericHandler()
        self.assertRaises(_format.FileFormatError, self._read_bcif_raw,
                          d, {'_foo': h})

    @unittest.skipIf(_format is None, "No C tokenizer")
    def test_read_error(self):
        """Test handling of errors from filelike read()"""
        def make_bcif():
            c1 = {'name': 'bar',
                  'data': {'data': struct.pack('2b', 1, 42),
                           'encoding':
                           [{'kind': 'IntegerPacking'},
                            {'kind': 'ByteArray',
                             'type': ihm.format_bcif._Int8}]},
                  'mask': None}
            return {'dataBlocks': [{'categories': [{'name': '_foo',
                                                    'columns': [c1]}]}]}

        class ReadError:
            """Filelike object that returns defined-size blocks from read,
               or errors out if empty"""
            def __init__(self, read_sz):
                self.read_sz = read_sz
                self.data = b''

            def write(self, b):
                self.data += b

            def read(self, sz):
                if not self.read_sz:
                    raise IndexError("foo")
                rsz = self.read_sz.pop(0)
                assert sz >= rsz
                ret = self.data[:rsz]
                self.data = self.data[rsz:]
                return ret

        def run_test(read_sz, ind=-1, cat='_foo'):
            d = make_bcif()
            h = GenericHandler()
            fh = ReadError(read_sz)
            _add_msgpack(d, fh)
            # If ind is given, just read up to the first instance of that
            # msgpack type
            if ind > 0:
                fh.read_sz = [fh.data.index(ind)]
            r = ihm.format_bcif.BinaryCifReader(fh, {cat: h})
            r.read_file()

        # Less data read than requested
        with self.assertRaises(OSError) as cm:
            run_test([0])
        self.assertIn('Less data read than requested', str(cm.exception))

        # Exception in read_map_or_nil (None=0xc0 in msgpack)
        self.assertRaises(IndexError, run_test, [], ind=0xc0)

        # Exception in read_array (array=0xdd)
        self.assertRaises(IndexError, run_test, [], ind=0xdd)

        # Exception in read_int (int=0xce)
        self.assertRaises(IndexError, run_test, [], ind=0xce)

        # Exception in read_bcif_exact_string (str=0xdb)
        self.assertRaises(IndexError, run_test, [], ind=0xdb)

        # Exception in read_bcif_string_dup (107=index of column name's
        # string size)
        self.assertRaises(IndexError, run_test, [107])

        # Exception in read_bcif_string (168=index of encoding name's
        # string size)
        self.assertRaises(IndexError, run_test, [168])

        # Exception in read_map (25=index of second map)
        self.assertRaises(IndexError, run_test, [25])

        # Exception in read_binary (137=index of data's binary size)
        self.assertRaises(IndexError, run_test, [137])

        # Exception in skip_no_limit
        self.assertRaises(IndexError, run_test, [147], cat='_bar')

    def test_omitted_unknown_not_in_file_explicit(self):
        """Test explicit handling of omitted/unknown/not in file data"""
        cat = Category('_foo',
                       {'var1': ['test1', '?', 'test2', None, 'test3']})
        h = GenericHandler()
        h.omitted = 'OMIT'
        h.unknown = 'UNK'
        h.not_in_file = 'NOT'
        h._keys = ('var1', 'var2')
        h._int_keys = frozenset()
        h._float_keys = frozenset()
        self._read_bcif([Block([cat])], {'_foo': h})
        self.assertEqual(h.data,
                         [{'var1': 'test1', 'var2': 'NOT'},
                          {'var1': 'UNK', 'var2': 'NOT'},
                          {'var1': 'test2', 'var2': 'NOT'},
                          {'var1': 'OMIT', 'var2': 'NOT'},
                          {'var1': 'test3', 'var2': 'NOT'}])

    def test_unknown_categories_ignored(self):
        """Check that unknown categories are just ignored"""
        cat1 = Category('_foo', {'var1': ['test1']})
        cat2 = Category('_bar', {'var2': ['test2']})
        h = GenericHandler()
        self._read_bcif([Block([cat1, cat2])], {'_foo': h})
        self.assertEqual(h.data, [{'var1': 'test1'}])

    def test_unknown_categories_handled(self):
        """Check that unknown categories are handled if requested"""
        class CatHandler:
            def __init__(self):
                self.warns = []

            def __call__(self, cat, line):
                self.warns.append((cat, line))

        ch = CatHandler()
        cat1 = Category('_foo', {'var1': ['test1']})
        cat2 = Category('_bar', {'var2': ['test2']})
        h = GenericHandler()
        self._read_bcif([Block([cat1, cat2])], {'_foo': h},
                        unknown_category_handler=ch)
        self.assertEqual(h.data, [{'var1': 'test1'}])
        self.assertEqual(ch.warns, [('_bar', 0)])

    def test_unknown_keywords_ignored(self):
        """Check that unknown keywords are ignored"""
        cat = Category('_foo', {'var1': ['test1'], 'othervar': ['test2']})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h})
        self.assertEqual(h.data, [{'var1': 'test1'}])

    def test_unknown_keywords_handled(self):
        """Check that unknown keywords are handled if requested"""
        class KeyHandler:
            def __init__(self):
                self.warns = []

            def __call__(self, cat, key, line):
                self.warns.append((cat, key, line))

        kh = KeyHandler()
        cat = Category('_foo', {'var1': ['test1'], 'othervar': ['test2']})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo': h},
                        unknown_keyword_handler=kh)
        self.assertEqual(h.data, [{'var1': 'test1'}])
        self.assertEqual(kh.warns, [('_foo', 'othervar', 0)])

    def test_multiple_data_blocks(self):
        """Test handling of multiple data blocks"""
        block1 = Block([Category('_foo',
                                 {'var1': ['test1'], 'var2': ['test2']})])
        block2 = Block([Category('_foo', {'var3': ['test3']})])
        fh = _make_bcif_file([block1, block2])

        h = GenericHandler()
        r = ihm.format_bcif.BinaryCifReader(fh, {'_foo': h})
        sys.modules['msgpack'] = MockMsgPack
        # Read first data block
        self.assertTrue(r.read_file())
        self.assertEqual(h.data, [{'var1': 'test1', 'var2': 'test2'}])

        # Read second data block
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [{'var3': 'test3'}])

        # No more data blocks
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [])

    def test_encoder(self):
        """Test _Encoder base class"""
        e = ihm.format_bcif._Encoder()
        e(None)  # noop

    def test_byte_array_encoder(self):
        """Test ByteArray encoder"""
        d = ihm.format_bcif._ByteArrayEncoder()

        # type 1 (signed char)
        data, encd = d([0, 1, -1])
        self.assertEqual(data, b'\x00\x01\xFF')
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Int8})

        # type 2 (signed short)
        data, encd = d([256, -21503])
        self.assertEqual(data, b'\x00\x01\x01\xAC')
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Int16})

        # type 3 (signed int)
        data, encd = d([-83951872])
        self.assertEqual(data, b'\x00\xff\xfe\xfa')
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Int32})

        # type 4 (unsigned char)
        data, encd = d([0, 255])
        self.assertEqual(data, b'\x00\xFF')
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Uint8})

        # type 5 (unsigned short)
        data, encd = d([256, 44033])
        self.assertEqual(data, b'\x00\x01\x01\xAC')
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Uint16})

        # type 6 (unsigned int)
        data, encd = d([4278255872])
        self.assertEqual(data, b'\x00\x01\x01\xFF')
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Uint32})

        # type 32 (32-bit float)
        data, encd = d([42.0])
        self.assertEqual(len(data), 4)
        self.assertEqual(encd, {'kind': 'ByteArray',
                                'type': ihm.format_bcif._Float32})

        # Too-large ints should cause an error
        self.assertRaises(TypeError, d, [2**34])
        self.assertRaises(TypeError, d, [-2**34])

    def test_delta_encoder(self):
        """Test Delta encoder"""
        d = ihm.format_bcif._DeltaEncoder()

        # too-small data is returned unchanged
        data = [0, 1, -1]
        encdata, encdict = d(data)
        self.assertEqual(data, encdata)
        self.assertIsNone(encdict)

        # large data is encoded
        data = [0, 1, -1] + [-1] * 40
        encdata, encdict = d(data)
        self.assertEqual(encdata, [0, 1, -2] + [0] * 40)
        self.assertEqual(encdict, {'origin': 0, 'kind': 'Delta',
                                   'srcType': ihm.format_bcif._Int8})

    def test_run_length_encoder(self):
        """Test RunLength encoder"""
        d = ihm.format_bcif._RunLengthEncoder()

        # too-small data is returned unchanged
        data = [0, 1, -1]
        encdata, encdict = d(data)
        self.assertEqual(data, encdata)
        self.assertIsNone(encdict)

        # large data that can't be compressed is returned unchanged
        data = list(range(50))
        encdata, encdict = d(data)
        self.assertEqual(data, encdata)
        self.assertIsNone(encdict)

        # large data that can be compressed
        data = [0] * 30 + [1] * 40
        encdata, encdict = d(data)
        self.assertEqual(encdata, [0, 30, 1, 40])
        self.assertEqual(encdict, {'kind': 'RunLength', 'srcSize': 70,
                                   'srcType': ihm.format_bcif._Uint8})

    def test_encode(self):
        """Test _encode function"""
        data = [1, 1, 1, 2, 3, 3]
        encoders = [ihm.format_bcif._ByteArrayEncoder()]
        encdata, encds = ihm.format_bcif._encode(data, encoders)
        self.assertEqual(encdata, b'\x01\x01\x01\x02\x03\x03')
        self.assertEqual(encds, [{'kind': 'ByteArray',
                                  'type': ihm.format_bcif._Uint8}])

        # DeltaEncoder will be a noop here since data is small
        encoders = [ihm.format_bcif._DeltaEncoder(),
                    ihm.format_bcif._ByteArrayEncoder()]
        encdata, encds = ihm.format_bcif._encode(data, encoders)
        self.assertEqual(encdata, b'\x01\x01\x01\x02\x03\x03')
        self.assertEqual(encds, [{'kind': 'ByteArray',
                                  'type': ihm.format_bcif._Uint8}])

    def test_mask_type_no_mask(self):
        """Test get_mask_and_type with no mask"""
        data = [1, 2, 3, 4]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertIsNone(mask)
        self.assertEqual(typ, int)

    def test_mask_type_masked_int(self):
        """Test get_mask_and_type with masked int data"""
        data = [1, 2, 3, None, ihm.unknown, 4]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0, 0, 0, 1, 2, 0])
        self.assertEqual(typ, int)

    def test_mask_type_masked_float(self):
        """Test get_mask_and_type with masked float data"""
        data = [1.0, 2.0, 3.0, None, ihm.unknown, 4.0]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0, 0, 0, 1, 2, 0])
        self.assertEqual(typ, float)

    def test_mask_type_masked_numpy_float(self):
        """Test get_mask_and_type with masked numpy float data"""
        try:
            import numpy
        except ImportError:
            self.skipTest("this test requires numpy")
        data = [numpy.float64(4.2), None, ihm.unknown]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0, 1, 2])
        self.assertEqual(typ, float)

    def test_mask_type_masked_str(self):
        """Test get_mask_and_type with masked str data"""
        # Literal . and ? should not be masked
        data = ['a', 'b', None, ihm.unknown, 'c', '.', '?']
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0, 0, 1, 2, 0, 0, 0])
        self.assertEqual(typ, str)

    def test_mask_type_mix_int_float(self):
        """Test get_mask_and_type with a mix of int and float data"""
        data = [1, 2, 3, 4.0]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertIsNone(mask)
        self.assertEqual(typ, float)  # int/float is coerced to float

    def test_mask_type_mix_int_float_str(self):
        """Test get_mask_and_type with a mix of int/float/str data"""
        data = [1, 2, 3, 4.0, 'foo']
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertIsNone(mask)
        self.assertEqual(typ, str)  # int/float/str is coerced to str

    def test_mask_type_bad_type(self):
        """Test get_mask_and_type with unknown type data"""
        class MockObject:
            pass
        data = [MockObject()]
        self.assertRaises(ValueError, ihm.format_bcif._get_mask_and_type, data)

    def test_masked_encoder(self):
        """Test MaskedEncoder base class"""
        e = ihm.format_bcif._MaskedEncoder()
        e(None, None)  # noop

    def test_string_array_encoder_no_mask(self):
        """Test StringArray encoder with no mask"""
        d = ihm.format_bcif._StringArrayMaskedEncoder()
        indices, encs = d(['a', 'AB', 'a'], None)
        self.assertEqual(indices, b'\x00\x01\x00')
        enc, = encs
        self.assertEqual(enc['dataEncoding'],
                         [{'kind': 'ByteArray',
                           'type': ihm.format_bcif._Uint8}])
        self.assertEqual(enc['offsetEncoding'],
                         [{'kind': 'ByteArray',
                           'type': ihm.format_bcif._Uint8}])
        self.assertEqual(enc['offsets'], b'\x00\x01\x03')
        self.assertEqual(enc['stringData'], 'aAB')

    def test_string_array_encoder_mask(self):
        """Test StringArray encoder with mask"""
        d = ihm.format_bcif._StringArrayMaskedEncoder()
        # True should be mapped to 'YES'; int 3 to str '3'
        # Unmasked literal . and ? should be kept as-is
        indices, encs = d(['a', 'AB', True, ihm.unknown, None, 'a', 3,
                           '.', '?'],
                          [0, 0, 0, 2, 1, 0, 0, 0, 0])
        # \xff is -1 (masked value) as a signed char (Int8)
        self.assertEqual(indices, b'\x00\x01\x02\xff\xff\x00\x03\x04\x05')
        enc, = encs
        self.assertEqual(enc['dataEncoding'],
                         [{'kind': 'ByteArray',
                           'type': ihm.format_bcif._Int8}])
        self.assertEqual(enc['offsetEncoding'],
                         [{'kind': 'ByteArray',
                           'type': ihm.format_bcif._Uint8}])
        self.assertEqual(enc['offsets'], b'\x00\x01\x03\x06\x07\x08\t')
        self.assertEqual(enc['stringData'], 'aABYES3.?')

    def test_int_array_encoder_no_mask(self):
        """Test IntArray encoder with no mask"""
        d = ihm.format_bcif._IntArrayMaskedEncoder()
        data, encs = d([5, 7, 8], None)
        self.assertEqual(data, b'\x05\x07\x08')
        self.assertEqual(encs, [{'kind': 'ByteArray',
                                 'type': ihm.format_bcif._Uint8}])

    def test_int_array_encoder_mask(self):
        """Test IntArray encoder with mask"""
        d = ihm.format_bcif._IntArrayMaskedEncoder()
        data, encs = d([5, 7, '?', 8, None], [0, 0, 2, 0, 1])
        # \xff is -1 (masked value) as a signed char (Int8)
        self.assertEqual(data, b'\x05\x07\xff\x08\xff')
        self.assertEqual(encs, [{'kind': 'ByteArray',
                                 'type': ihm.format_bcif._Int8}])

    def test_float_array_encoder_no_mask(self):
        """Test FloatArray encoder with no mask"""
        d = ihm.format_bcif._FloatArrayMaskedEncoder()
        # int data should be coerced to float
        data, encs = d([5.0, 7.0, 8.0, 4], None)
        self.assertEqual(len(data), 4 * 4)
        self.assertEqual(encs, [{'kind': 'ByteArray',
                                 'type': ihm.format_bcif._Float32}])

    def test_float_array_encoder_mask(self):
        """Test FloatArray encoder with mask"""
        d = ihm.format_bcif._FloatArrayMaskedEncoder()
        data, encs = d([5., 7., '?', 8., None], [0, 0, 2, 0, 1])
        self.assertEqual(len(data), 5 * 4)
        self.assertEqual(encs, [{'kind': 'ByteArray',
                                 'type': ihm.format_bcif._Float32}])

    def test_category(self):
        """Test CategoryWriter class"""
        fh = MockFh()
        sys.modules['msgpack'] = MockMsgPack
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.category('foo') as loc:
            loc.write(bar='baz')
        writer.flush()
        block, = fh.data['dataBlocks']
        category, = block['categories']
        column, = category['columns']
        self.assertEqual(block['header'], 'ihm')
        self.assertEqual(category['name'], 'foo')
        self.assertEqual(category['rowCount'], 1)
        self.assertEqual(column['name'], 'bar')
        self.assertEqual(column['data']['encoding'][0]['stringData'],
                         'baz')

    def test_empty_loop(self):
        """Test LoopWriter class with no values"""
        fh = MockFh()
        sys.modules['msgpack'] = MockMsgPack
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.loop('foo', ["bar", "baz"]):
            pass
        writer.flush()
        self.assertEqual(fh.data['dataBlocks'][0]['categories'], [])

    def test_loop(self):
        """Test LoopWriter class"""
        fh = MockFh()
        sys.modules['msgpack'] = MockMsgPack
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.loop('foo', ["bar", "baz"]) as lp:
            lp.write(bar='x')
            lp.write(bar=None, baz='z')
            lp.write(bar=ihm.unknown, baz='z')
            lp.write(bar='.', baz='z')
            lp.write(bar='?', baz='z')
            lp.write(baz='y')
        writer.flush()
        block, = fh.data['dataBlocks']
        category, = block['categories']
        self.assertEqual(category['name'], 'foo')
        self.assertEqual(category['rowCount'], 6)
        cols = sorted(category['columns'], key=lambda x: x['name'])
        self.assertEqual(len(cols), 2)
        # Check mask for bar column; literal . and ? should not be masked (=0)
        self.assertEqual(cols[0]['mask']['data'],
                         b'\x00\x01\x02\x00\x00\x01')


if __name__ == '__main__':
    unittest.main()
