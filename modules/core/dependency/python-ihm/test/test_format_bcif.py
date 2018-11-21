import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.format_bcif

# Provide dummy implementations of msgpack.unpack() and msgpack.pack() which
# just return the data unchanged. We can use these to test the BinaryCIF
# parser with Python objects rather than having to install msgpack and
# generate real binary files
class MockMsgPack(object):
    @staticmethod
    def unpack(fh):
        return fh
    @staticmethod
    def pack(data, fh):
        fh.data = data

class MockFh(object):
    pass

class GenericHandler(object):
    """Capture BinaryCIF data as a simple list of dicts"""
    not_in_file = None
    omitted = None
    unknown = "?"

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
        mask_data = ''.join(chr(i) for i in mask).encode('ascii')
        mask = {b'data': ''.join(chr(i) for i in mask).encode('ascii'),
                b'encoding': [{b'kind': b'ByteArray',
                               b'type': ihm.format_bcif._Uint8}]}
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
         b'kind': b'StringArray',
         b'dataEncoding': [{b'kind': b'ByteArray',
                            b'type': ihm.format_bcif._Uint8}],
         b'stringData': string_data.encode('ascii'),
         b'offsetEncoding': [{b'kind': b'ByteArray',
                              b'type': ihm.format_bcif._Uint8}],
         b'offsets': offsets }
    d = {b'data': indices,
         b'encoding': [string_array_encoding]}
    return d, mask

class Category(object):
    def __init__(self, name, data):
        self.name = name
        self.data = data

    def get_bcif(self):
        nrows = 0
        cols = []
        for name, rows in self.data.items():
            nrows = len(rows)
            data, mask = _encode(rows)
            cols.append({b'mask': mask, b'name': name.encode('ascii'),
                         b'data': data})
        return {b'name': self.name.encode('ascii'),
                b'columns': cols, b'rowCount': nrows}


class Block(list):
    pass


def _make_bcif_file(blocks):
    blocks = [{b'header':'ihm',
               b'categories':[c.get_bcif() for c in block]}
              for block in blocks]
    return {b'version':'0.1', b'encoder':'python-ihm test suite',
            b'dataBlocks':blocks}

class Tests(unittest.TestCase):
    def test_decode_bytes(self):
        """Test decode_bytes function"""
        d = ihm.format_bcif._decode_bytes(b'foo')
        self.assertEqual(d, 'foo')

    def test_decoder_base(self):
        """Test Decoder base class"""
        d = ihm.format_bcif._Decoder()
        self.assertEqual(d._kind, None)
        d(enc=None, data=None) # noop

    def test_string_array_decoder(self):
        """Test StringArray decoder"""
        d = ihm.format_bcif._StringArrayDecoder()
        self.assertEqual(d._kind, b'StringArray')

        # Int8 is signed char (so FF is -1)
        enc = {b'stringData': b'aAB',
               b'dataEncoding': [{b'kind':b'ByteArray',
                                  b'type':ihm.format_bcif._Int8}],
               b'offsetEncoding': [{b'kind':b'ByteArray',
                                    b'type':ihm.format_bcif._Int8}],
               b'offsets':b'\x00\x01\x03'}
        data = b'\x00\x01\x00\xFF'

        data = d(enc, data)
        self.assertEqual(list(data), ['a', 'AB', 'a', None])

    def test_byte_array_decoder(self):
        """Test ByteArray decoder"""
        d = ihm.format_bcif._ByteArrayDecoder()
        self.assertEqual(d._kind, b'ByteArray')

        # type 1 (signed char)
        data = d({b'type':ihm.format_bcif._Int8}, b'\x00\x01\xFF')
        self.assertEqual(list(data), [0, 1, -1])

        # type 2 (signed short)
        data = d({b'type':ihm.format_bcif._Int16}, b'\x00\x01\x01\xAC')
        self.assertEqual(list(data), [256, -21503])

        # type 3 (signed int)
        data = d({b'type':ihm.format_bcif._Int32}, b'\x00\x01\x01\x05')
        self.assertEqual(list(data), [83951872])

        # type 4 (unsigned char)
        data = d({b'type':ihm.format_bcif._Uint8}, b'\x00\xFF')
        self.assertEqual(list(data), [0, 255])

        # type 5 (unsigned short)
        data = d({b'type':ihm.format_bcif._Uint16}, b'\x00\x01\x01\xAC')
        self.assertEqual(list(data), [256, 44033])

        # type 6 (unsigned int)
        data = d({b'type':ihm.format_bcif._Uint32}, b'\x00\x01\x01\xFF')
        self.assertEqual(list(data), [4278255872])

        # type 32 (32-bit float)
        data = d({b'type':ihm.format_bcif._Float32}, b'\x00\x00(B')
        self.assertAlmostEqual(list(data)[0], 42.0, places=1)

        # type 33 (64-bit float)
        data = d({b'type':ihm.format_bcif._Float64},
                  b'\x00\x00\x00\x00\x00\x00E@')
        self.assertAlmostEqual(list(data)[0], 42.0, places=1)

    def test_integer_packing_decoder_signed(self):
        """Test IntegerPacking decoder with signed data"""
        d = ihm.format_bcif._IntegerPackingDecoder()
        self.assertEqual(d._kind, b'IntegerPacking')

        # 1-byte data
        data = d({b'isUnsigned':False, b'byteCount':1},
                 [1, 2, -3, 127, 1, -128, -5])
        self.assertEqual(list(data), [1, 2, -3, 128, -133])

        # 2-byte data
        data = d({b'isUnsigned':False, b'byteCount':2},
                 [1, 2, -3, 32767, 1, -32768, -5])
        self.assertEqual(list(data), [1, 2, -3, 32768, -32773])

    def test_integer_packing_decoder_unsigned(self):
        """Test IntegerPacking decoder with unsigned data"""
        d = ihm.format_bcif._IntegerPackingDecoder()
        self.assertEqual(d._kind, b'IntegerPacking')

        # 1-byte data
        data = d({b'isUnsigned':True, b'byteCount':1},
                 [1, 2, 3, 127, 1, 255, 1])
        self.assertEqual(list(data), [1, 2, 3, 127, 1, 256])

        # 2-byte data
        data = d({b'isUnsigned':True, b'byteCount':2},
                 [1, 2, 3, 32767, 1, 65535, 5])
        self.assertEqual(list(data), [1, 2, 3, 32767, 1, 65540])

    def test_delta_decoder(self):
        """Test Delta decoder"""
        d = ihm.format_bcif._DeltaDecoder()
        self.assertEqual(d._kind, b'Delta')

        data = d({b'origin':1000}, [0, 3, 2, 1])
        self.assertEqual(list(data), [1000, 1003, 1005, 1006])

    def test_run_length_decoder(self):
        """Test RunLength decoder"""
        d = ihm.format_bcif._RunLengthDecoder()
        self.assertEqual(d._kind, b'RunLength')

        data = d({}, [1, 3, 2, 1, 3, 2])
        self.assertEqual(list(data), [1, 1, 1, 2, 3, 3])

    def test_fixed_point_decoder(self):
        """Test FixedPoint decoder"""
        d = ihm.format_bcif._FixedPointDecoder()
        self.assertEqual(d._kind, b'FixedPoint')

        data = list(d({b'factor':100}, [120, 123, 12]))
        self.assertEqual(len(data), 3)
        self.assertAlmostEqual(data[0], 1.20, places=2)
        self.assertAlmostEqual(data[1], 1.23, places=2)
        self.assertAlmostEqual(data[2], 0.12, places=2)

    def test_decode(self):
        """Test _decode function"""
        data = b'\x01\x03\x02\x01\x03\x02'
        runlen = {b'kind': b'RunLength'}
        bytearr = {b'kind': b'ByteArray', b'type':ihm.format_bcif._Int8}
        data = ihm.format_bcif._decode(data, [runlen, bytearr])
        self.assertEqual(list(data), [1, 1, 1, 2, 3, 3])

    def _read_bcif(self, blocks, category_handlers):
        fh = _make_bcif_file(blocks)
        sys.modules['msgpack'] = MockMsgPack
        r = ihm.format_bcif.BinaryCifReader(fh, category_handlers)
        r.read_file()

    def test_category_case_insensitive(self):
        """Categories and keywords should be case insensitive"""
        cat1 = Category('_exptl', {'method':['foo']})
        cat2 = Category('_Exptl', {'METHod':['foo']})
        for cat in cat1, cat2:
            h = GenericHandler()
            self._read_bcif([Block([cat])], {'_exptl':h})
        self.assertEqual(h.data, [{'method':'foo'}])

    def test_omitted_unknown(self):
        """Test handling of omitted/unknown data"""
        cat = Category('_foo', {'var1':['test1', '?', 'test2', None, 'test3']})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo':h})
        self.assertEqual(h.data,
                         [{'var1': 'test1'}, {'var1': '?'}, {'var1': 'test2'},
                          {}, {'var1': 'test3'}])

    def test_omitted_unknown_not_in_file_explicit(self):
        """Test explicit handling of omitted/unknown/not in file data"""
        cat = Category('_foo', {'var1':['test1', '?', 'test2', None, 'test3']})
        h = GenericHandler()
        h.omitted = 'OMIT'
        h.unknown = 'UNK'
        h.not_in_file = 'NOT'
        h._keys = ('var1', 'var2')
        self._read_bcif([Block([cat])], {'_foo':h})
        self.assertEqual(h.data,
                         [{'var1': 'test1', 'var2': 'NOT'},
                          {'var1': 'UNK', 'var2': 'NOT'},
                          {'var1': 'test2', 'var2': 'NOT'},
                          {'var1': 'OMIT', 'var2': 'NOT'},
                          {'var1': 'test3', 'var2': 'NOT'}])

    def test_extra_categories_ignored(self):
        """Check that extra categories in the file are ignored"""
        cat1 = Category('_foo', {'var1':['test1']})
        cat2 = Category('_bar', {'var2':['test2']})
        h = GenericHandler()
        self._read_bcif([Block([cat1, cat2])], {'_foo':h})
        self.assertEqual(h.data, [{'var1': 'test1'}])

    def test_extra_keywords_ignored(self):
        """Check that extra keywords in the file are ignored"""
        cat = Category('_foo', {'var1':['test1'], 'othervar':['test2']})
        h = GenericHandler()
        self._read_bcif([Block([cat])], {'_foo':h})
        self.assertEqual(h.data, [{'var1': 'test1'}])

    def test_multiple_data_blocks(self):
        """Test handling of multiple data blocks"""
        block1 = Block([Category('_foo', {'var1':['test1'], 'var2':['test2']})])
        block2 = Block([Category('_foo', {'var3':['test3']})])
        fh = _make_bcif_file([block1, block2])

        h = GenericHandler()
        r = ihm.format_bcif.BinaryCifReader(fh, {'_foo':h})
        sys.modules['msgpack'] = MockMsgPack
        # Read first data block
        self.assertTrue(r.read_file())
        self.assertEqual(h.data, [{'var1':'test1', 'var2':'test2'}])

        # Read second data block
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [{'var3':'test3'}])

        # No more data blocks
        h.data = []
        self.assertFalse(r.read_file())
        self.assertEqual(h.data, [])

    def test_encoder(self):
        """Test _Encoder base class"""
        e = ihm.format_bcif._Encoder()
        e(None) # noop

    def test_byte_array_encoder(self):
        """Test ByteArray encoder"""
        d = ihm.format_bcif._ByteArrayEncoder()

        # type 1 (signed char)
        data, encd = d([0, 1, -1])
        self.assertEqual(data, b'\x00\x01\xFF')
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Int8})

        # type 2 (signed short)
        data, encd = d([256, -21503])
        self.assertEqual(data, b'\x00\x01\x01\xAC')
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Int16})

        # type 3 (signed int)
        data, encd = d([-83951872])
        self.assertEqual(data, b'\x00\xff\xfe\xfa')
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Int32})

        # type 4 (unsigned char)
        data, encd = d([0, 255])
        self.assertEqual(data, b'\x00\xFF')
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Uint8})

        # type 5 (unsigned short)
        data, encd = d([256, 44033])
        self.assertEqual(data, b'\x00\x01\x01\xAC')
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Uint16})

        # type 6 (unsigned int)
        data, encd = d([4278255872])
        self.assertEqual(data, b'\x00\x01\x01\xFF')
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Uint32})

        # type 32 (32-bit float)
        data, encd = d([42.0])
        self.assertEqual(len(data), 4)
        self.assertEqual(encd, {b'kind': b'ByteArray',
                                b'type': ihm.format_bcif._Float32})

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
        self.assertEqual(encdict, None)

        # large data is encoded
        data = [0, 1, -1] + [-1] * 40
        encdata, encdict = d(data)
        self.assertEqual(encdata, [0, 1, -2] + [0] * 40)
        self.assertEqual(encdict, {b'origin': 0, b'kind': b'Delta',
                                   b'srcType': ihm.format_bcif._Int8})

    def test_run_length_encoder(self):
        """Test RunLength encoder"""
        d = ihm.format_bcif._RunLengthEncoder()

        # too-small data is returned unchanged
        data = [0, 1, -1]
        encdata, encdict = d(data)
        self.assertEqual(data, encdata)
        self.assertEqual(encdict, None)

        # large data that can't be compressed is returned unchanged
        data = list(range(50))
        encdata, encdict = d(data)
        self.assertEqual(data, encdata)
        self.assertEqual(encdict, None)

        # large data that can be compressed
        data = [0] * 30 + [1] * 40
        encdata, encdict = d(data)
        self.assertEqual(encdata, [0, 30, 1, 40])
        self.assertEqual(encdict, {b'kind': b'RunLength', b'srcSize': 70,
                                   b'srcType': ihm.format_bcif._Uint8})

    def test_encode(self):
        """Test _encode function"""
        data = [1, 1, 1, 2, 3, 3]
        encoders = [ihm.format_bcif._ByteArrayEncoder()]
        encdata, encds = ihm.format_bcif._encode(data, encoders)
        self.assertEqual(encdata, b'\x01\x01\x01\x02\x03\x03')
        self.assertEqual(encds, [{b'kind': b'ByteArray',
                                  b'type': ihm.format_bcif._Uint8}])

        # DeltaEncoder will be a noop here since data is small
        encoders = [ihm.format_bcif._DeltaEncoder(),
                    ihm.format_bcif._ByteArrayEncoder()]
        encdata, encds = ihm.format_bcif._encode(data, encoders)
        self.assertEqual(encdata, b'\x01\x01\x01\x02\x03\x03')
        self.assertEqual(encds, [{b'kind': b'ByteArray',
                                  b'type': ihm.format_bcif._Uint8}])

    def test_mask_type_no_mask(self):
        """Test get_mask_and_type with no mask"""
        data = [1,2,3,4]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, None)
        self.assertEqual(typ, int)

    def test_mask_type_masked_int(self):
        """Test get_mask_and_type with masked int data"""
        data = [1,2,3,None,'?',4]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0,0,0,1,2,0])
        self.assertEqual(typ, int)

    def test_mask_type_masked_long(self):
        """Test get_mask_and_type with masked long data"""
        if sys.version_info[0] < 3:
            # long type is only in Python 2
            # Use long(x) rather than xL since the latter will cause a syntax
            # error in Python 3
            data = [long(1),long(2),long(3),None,'?',long(4)]
            mask, typ = ihm.format_bcif._get_mask_and_type(data)
            self.assertEqual(mask, [0,0,0,1,2,0])
            self.assertEqual(typ, int)

    def test_mask_type_masked_float(self):
        """Test get_mask_and_type with masked float data"""
        data = [1.0,2.0,3.0,None,'?',4.0]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0,0,0,1,2,0])
        self.assertEqual(typ, float)

    def test_mask_type_masked_numpy_float(self):
        """Test get_mask_and_type with masked numpy float data"""
        try:
            import numpy
        except ImportError:
            self.skipTest("this test requires numpy")
        data = [numpy.float64(4.2),None,'?']
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0,1,2])
        self.assertEqual(typ, float)

    def test_mask_type_masked_str(self):
        """Test get_mask_and_type with masked str data"""
        data = ['a','b',None,'?','c']
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, [0,0,1,2,0])
        self.assertEqual(typ, str)

    def test_mask_type_mix_int_float(self):
        """Test get_mask_and_type with a mix of int and float data"""
        data = [1,2,3,4.0]
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, None)
        self.assertEqual(typ, float) # int/float is coerced to float

    def test_mask_type_mix_int_float_str(self):
        """Test get_mask_and_type with a mix of int/float/str data"""
        data = [1,2,3,4.0,'foo']
        mask, typ = ihm.format_bcif._get_mask_and_type(data)
        self.assertEqual(mask, None)
        self.assertEqual(typ, str) # int/float/str is coerced to str

    def test_mask_type_bad_type(self):
        """Test get_mask_and_type with unknown type data"""
        class MockObject(object):
            pass
        data = [MockObject()]
        self.assertRaises(ValueError, ihm.format_bcif._get_mask_and_type, data)

    def test_masked_encoder(self):
        """Test MaskedEncoder base class"""
        e = ihm.format_bcif._MaskedEncoder()
        e(None, None) # noop

    def test_string_array_encoder_no_mask(self):
        """Test StringArray encoder with no mask"""
        d = ihm.format_bcif._StringArrayMaskedEncoder()
        indices, encs = d(['a', 'AB', 'a'], None)
        self.assertEqual(indices, b'\x00\x01\x00')
        enc, = encs
        self.assertEqual(enc[b'dataEncoding'],
                             [{b'kind':b'ByteArray',
                               b'type':ihm.format_bcif._Uint8}])
        self.assertEqual(enc[b'offsetEncoding'],
                             [{b'kind':b'ByteArray',
                               b'type':ihm.format_bcif._Uint8}])
        self.assertEqual(enc[b'offsets'], b'\x00\x01\x03')
        self.assertEqual(enc[b'stringData'], b'aAB')

    def test_string_array_encoder_mask(self):
        """Test StringArray encoder with mask"""
        d = ihm.format_bcif._StringArrayMaskedEncoder()
        # True should be mapped to 'YES'; int 3 to str '3'
        indices, encs = d(['a', 'AB', True, '?', None, 'a', 3],
                          [0, 0, 0, 2, 1, 0, 0])
        # \xff is -1 (masked value) as a signed char (Int8)
        self.assertEqual(indices, b'\x00\x01\x02\xff\xff\x00\x03')
        enc, = encs
        self.assertEqual(enc[b'dataEncoding'],
                             [{b'kind':b'ByteArray',
                               b'type':ihm.format_bcif._Int8}])
        self.assertEqual(enc[b'offsetEncoding'],
                             [{b'kind':b'ByteArray',
                               b'type':ihm.format_bcif._Uint8}])
        self.assertEqual(enc[b'offsets'], b'\x00\x01\x03\x06\x07')
        self.assertEqual(enc[b'stringData'], b'aABYES3')

    def test_int_array_encoder_no_mask(self):
        """Test IntArray encoder with no mask"""
        d = ihm.format_bcif._IntArrayMaskedEncoder()
        data, encs = d([5, 7, 8], None)
        self.assertEqual(data, b'\x05\x07\x08')
        self.assertEqual(encs, [{b'kind': b'ByteArray',
                                 b'type': ihm.format_bcif._Uint8}])

    def test_int_array_encoder_mask(self):
        """Test IntArray encoder with mask"""
        d = ihm.format_bcif._IntArrayMaskedEncoder()
        data, encs = d([5, 7, '?', 8, None], [0, 0, 2, 0, 1])
        # \xff is -1 (masked value) as a signed char (Int8)
        self.assertEqual(data, b'\x05\x07\xff\x08\xff')
        self.assertEqual(encs, [{b'kind': b'ByteArray',
                                 b'type': ihm.format_bcif._Int8}])

    def test_float_array_encoder_no_mask(self):
        """Test FloatArray encoder with no mask"""
        d = ihm.format_bcif._FloatArrayMaskedEncoder()
        # int data should be coerced to float
        data, encs = d([5.0, 7.0, 8.0, 4], None)
        self.assertEqual(len(data), 4 * 4)
        self.assertEqual(encs, [{b'kind': b'ByteArray',
                                 b'type': ihm.format_bcif._Float32}])

    def test_float_array_encoder_mask(self):
        """Test FloatArray encoder with mask"""
        d = ihm.format_bcif._FloatArrayMaskedEncoder()
        data, encs = d([5., 7., '?', 8., None], [0, 0, 2, 0, 1])
        self.assertEqual(len(data), 5 * 4)
        self.assertEqual(encs, [{b'kind': b'ByteArray',
                                 b'type': ihm.format_bcif._Float32}])

    def test_category(self):
        """Test CategoryWriter class"""
        fh = MockFh()
        sys.modules['msgpack'] = MockMsgPack
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.category('foo') as l:
            l.write(bar='baz')
        writer.flush()
        block, = fh.data[b'dataBlocks']
        category, = block[b'categories']
        column, = category[b'columns']
        self.assertEqual(block[b'header'], b'ihm')
        self.assertEqual(category[b'name'], b'foo')
        self.assertEqual(category[b'rowCount'], 1)
        self.assertEqual(column[b'name'], b'bar')
        self.assertEqual(column[b'data'][b'encoding'][0][b'stringData'], b'baz')

    def test_empty_loop(self):
        """Test LoopWriter class with no values"""
        fh = MockFh()
        sys.modules['msgpack'] = MockMsgPack
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.loop('foo', ["bar", "baz"]) as l:
            pass
        writer.flush()
        self.assertEqual(fh.data[b'dataBlocks'][0][b'categories'], [])

    def test_loop(self):
        """Test LoopWriter class"""
        fh = MockFh()
        sys.modules['msgpack'] = MockMsgPack
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.loop('foo', ["bar", "baz"]) as l:
            l.write(bar='x')
            l.write(bar=None, baz='z')
            l.write(baz='y')
        writer.flush()
        block, = fh.data[b'dataBlocks']
        category, = block[b'categories']
        self.assertEqual(category[b'name'], b'foo')
        self.assertEqual(category[b'rowCount'], 3)
        col1, col2 = category[b'columns']


if __name__ == '__main__':
    unittest.main()
