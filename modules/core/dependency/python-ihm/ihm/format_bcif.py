"""Utility classes to handle BinaryCIF format.

   See https://github.com/dsehnal/BinaryCIF for a description of the
   BinaryCIF file format.

   This module provides classes to read in and write out BinaryCIF files. It is
   only concerned with handling syntactically correct BinaryCIF -
   it does not know the set of tables or the mapping to ihm objects. For that,
   see :mod:`ihm.reader`.
"""

from __future__ import division
import struct
import sys
import inspect
import ihm.format
import ihm

# ByteArray types
_Int8 = 1
_Int16 = 2
_Int32 = 3
_Uint8 = 4
_Uint16 = 5
_Uint32 = 6
_Float32 = 32
_Float64 = 33

# msgpack data is UTF-8 strings; need to convert to/from Unicode in Python 2
# All mmCIF data is ASCII
if sys.version_info[0] >= 3:
    def _decode_bytes(bs):
        return bs

    def _encode_str(s):
        return s
else:
    def _decode_bytes(bs):
        if isinstance(bs, unicode):  # noqa: F821
            return bs.encode('ascii', errors='replace')
        else:
            return bs

    def _encode_str(s):
        return s.decode('ascii', errors='replace')


class _Decoder(object):
    """Base class for all decoders."""

    _kind = None  # Encoder kind (in BinaryCIF specification)

    def __call__(self, enc, data):
        """Given encoding information `enc` and raw data `data`, return
           decoded data. This can be a generator."""
        pass


class _StringArrayDecoder(_Decoder):
    """Decode an array of strings stored as a concatenation of all unique
       strings, an array of offsets describing substrings, and indices into
       the offset array."""
    _kind = 'StringArray'

    def __call__(self, enc, data):
        offsets = list(_decode(enc['offsets'], enc['offsetEncoding']))
        indices = _decode(data, enc['dataEncoding'])
        substr = []
        string_data = _decode_bytes(enc['stringData'])
        for i in range(0, len(offsets) - 1):
            substr.append(string_data[offsets[i]:offsets[i + 1]])
        # todo: return a listlike class instead?
        for i in indices:
            yield None if i < 0 else substr[i]


class _ByteArrayDecoder(_Decoder):
    """Decode an array of numbers of specified type stored as raw bytes"""

    _kind = 'ByteArray'

    # Map integer/float type to struct format string
    _struct_map = {
        _Int8: 'b',
        _Int16: 'h',
        _Int32: 'i',
        _Uint8: 'B',
        _Uint16: 'H',
        _Uint32: 'I',
        _Float32: 'f',
        _Float64: 'd',
    }

    def __call__(self, enc, data):
        fmt = self._struct_map[enc['type']]
        sz = len(data) // struct.calcsize(fmt)
        # All data is encoded little-endian in bcif
        return struct.unpack('<' + fmt * sz, data)


class _IntegerPackingDecoder(_Decoder):
    """Decode a (32-bit) integer array stored as 8- or 16-bit values."""
    _kind = 'IntegerPacking'

    def _unsigned_decode(self, enc, data):
        limit = 0xFF if enc['byteCount'] == 1 else 0xFFFF
        i = 0
        while i < len(data):
            value = 0
            t = data[i]
            while t == limit:
                value += t
                i += 1
                t = data[i]
            yield value + t
            i += 1

    def _signed_decode(self, enc, data):
        upper_limit = 0x7F if enc['byteCount'] == 1 else 0x7FFF
        lower_limit = -upper_limit - 1
        i = 0
        while i < len(data):
            value = 0
            t = data[i]
            while t == upper_limit or t == lower_limit:
                value += t
                i += 1
                t = data[i]
            yield value + t
            i += 1

    def __call__(self, enc, data):
        if enc['isUnsigned']:
            return self._unsigned_decode(enc, data)
        else:
            return self._signed_decode(enc, data)


class _DeltaDecoder(_Decoder):
    """Decode an integer array stored as an array of consecutive
       differences."""
    _kind = 'Delta'

    def __call__(self, enc, data):
        val = enc['origin']
        for d in data:
            val += d
            yield val


class _RunLengthDecoder(_Decoder):
    """Decode an integer array stored as pairs of (value, number of repeats)"""
    _kind = 'RunLength'

    def __call__(self, enc, data):
        data = list(data)
        for i in range(0, len(data), 2):
            for j in range(data[i + 1]):
                yield data[i]


class _FixedPointDecoder(_Decoder):
    """Decode a floating point array stored as integers multiplied by
       a given factor."""
    _kind = 'FixedPoint'

    def __call__(self, enc, data):
        factor = float(enc['factor'])
        for d in data:
            yield float(d) / factor


def _get_decoder_map():
    m = {}
    for d in [x[1] for x in inspect.getmembers(sys.modules[__name__],
                                               inspect.isclass)
              if issubclass(x[1], _Decoder)]:
        m[d._kind] = d()
    return m


# Mapping from BinaryCIF encoding names to _Decoder objects
_decoder_map = _get_decoder_map()


def _decode(data, encoding):
    """Decode the data using the list of encodings, and return it."""
    for enc in reversed(encoding):
        data = _decoder_map[enc['kind']](enc, data)
    return data


class BinaryCifReader(ihm.format._Reader):
    """Class to read a BinaryCIF file and extract some or all of its data.

       Use :meth:`read_file` to actually read the file.
       See :class:`ihm.format.CifReader` for a description of the parameters.
    """
    def __init__(self, fh, category_handler, unknown_category_handler=None,
                 unknown_keyword_handler=None):
        self.category_handler = category_handler
        self.unknown_category_handler = unknown_category_handler
        self.unknown_keyword_handler = unknown_keyword_handler
        self.fh = fh
        self._file_blocks = None

    def read_file(self):
        """Read the file and extract data.

           :return: True iff more data blocks are available to be read.
        """
        self._add_category_keys()
        if self._file_blocks is None:
            self._file_blocks = self._read_msgpack()
        if len(self._file_blocks) > 0:
            for category in self._file_blocks[0]['categories']:
                cat_name = _decode_bytes(category['name']).lower()
                handler = self.category_handler.get(cat_name, None)
                if handler:
                    self._handle_category(handler, category, cat_name)
                elif self.unknown_category_handler is not None:
                    self.unknown_category_handler(cat_name, None)
            del self._file_blocks[0]
        return len(self._file_blocks) > 0

    def _handle_category(self, handler, category, cat_name):
        """Extract data for the given category"""
        num_cols = len(handler._keys)
        # Read all data for the category;
        # category_data[col][row]
        category_data = [None] * num_cols
        num_rows = 0
        # Only read columns that match a handler key (case insensitive)
        key_index = {}
        for i, key in enumerate(handler._keys):
            key_index[key] = i
        column_indices = []
        for c in category['columns']:
            key_name = _decode_bytes(c['name']).lower()
            ki = key_index.get(key_name, None)
            if ki is not None:
                column_indices.append(ki)
                r = self._read_column(c, handler)
                num_rows = len(r)
                category_data[ki] = r
            elif self.unknown_keyword_handler is not None:
                self.unknown_keyword_handler(cat_name, key_name, None)
        row_data = [handler.not_in_file] * num_cols
        for row in range(num_rows):
            # Only update data for columns that we read (others will
            # remain None)
            for i in column_indices:
                row_data[i] = category_data[i][row]
            handler(*row_data)

    def _read_column(self, column, handler):
        """Read a single category column data"""
        data = _decode(column['data']['data'], column['data']['encoding'])
        # Handle 'unknown' values (mask==2) or 'omitted' (mask==1)
        if column['mask'] is not None:
            mask = _decode(column['mask']['data'],
                           column['mask']['encoding'])
            data = [handler.unknown if m == 2 else handler.omitted if m == 1
                    else d for d, m in zip(data, mask)]
        return list(data)

    def _read_msgpack(self):
        """Read the msgpack data from the file and return data blocks"""
        import msgpack
        d = msgpack.unpack(self.fh, raw=False)
        return d['dataBlocks']


class _CategoryWriter(object):
    def __init__(self, writer, category):
        self.writer = writer
        self.category = category
        self._data = {}

    def write(self, **kwargs):
        self._data.update(kwargs)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        for k in self._data:
            self._data[k] = [self._data[k]]
        self.writer._add_category(self.category, self._data)


class _LoopWriter(object):
    def __init__(self, writer, category, keys):
        self.writer = writer
        self.category = category
        self.keys = keys
        # Remove characters that we can't use in Python identifiers
        self.python_keys = [k.replace('[', '').replace(']', '') for k in keys]
        self._values = []
        for i in range(len(keys)):
            self._values.append([])

    def write(self, **kwargs):
        for i, k in enumerate(self.python_keys):
            val = kwargs.get(k, None)
            self._values[i].append(val)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        data = {}
        for key, value in zip(self.keys, self._values):
            data[key] = value
        self.writer._add_category(self.category, data)


class EncodeError(Exception):
    """Exception raised if input data cannot be encoded"""
    pass


class _Encoder(object):
    """Base class for all encoders"""
    _kind = None  # Encoder kind (in BinaryCIF specification)

    def __call__(self, data):
        """Given raw data `data`, return encoded data and a BinaryCIF
           encoder information dict."""
        pass


def _get_int_float_type(data):
    """Determine the int/float type of the given data"""
    # If anything is float, treat everything as single-precision float
    for d in data:
        if isinstance(d, float):
            return _Float32
    # Otherwise, figure out the most appropriate int type
    min_val = min(data)
    max_val = max(data)
    if min_val >= 0:
        # Unsigned types
        for typ, limit in [(_Uint8, 0xFF), (_Uint16, 0xFFFF),
                           (_Uint32, 0xFFFFFFFF)]:
            if max_val <= limit:
                return typ
    else:
        # Signed types
        for typ, up_limit in [(_Int8, 0x7F), (_Int16, 0x7FFF),
                              (_Int32, 0x7FFFFFFF)]:
            low_limit = -up_limit - 1
            if min_val >= low_limit and max_val <= up_limit:
                return typ
    raise TypeError("Cannot represent data as BinaryCIF")


class _ByteArrayEncoder(_Encoder):

    # Map integer/float type to struct format string
    _struct_map = _ByteArrayDecoder._struct_map

    def __call__(self, data):
        ba_type = _get_int_float_type(data)
        encdict = {u'kind': u'ByteArray', u'type': ba_type}
        fmt = self._struct_map[ba_type]
        # All data is encoded little-endian in bcif
        return struct.pack('<' + fmt * len(data), *data), encdict


class _DeltaEncoder(_Encoder):
    """Encode an integer array as an array of consecutive differences."""
    def __call__(self, data):
        # Don't try to compress small arrays; the overhead of the compression
        # probably will exceed the space savings
        if len(data) <= 40:
            return data, None
        data_type = _get_int_float_type(data)
        encdict = {u'kind': u'Delta', u'origin': data[0],
                   u'srcType': data_type}
        encdata = [0] + [data[i] - data[i - 1] for i in range(1, len(data))]
        return encdata, encdict


class _RunLengthEncoder(_Encoder):
    """Encode an integer array as pairs of (value, number of repeats)"""
    def __call__(self, data):
        # Don't try to compress small arrays; the overhead of the compression
        # probably will exceed the space savings
        if len(data) <= 40:
            return data, None
        data_type = _get_int_float_type(data)
        encdict = {u'kind': u'RunLength',
                   u'srcType': data_type, u'srcSize': len(data)}
        encdata = []
        val = None
        for d in data:
            if d != val:
                if val is not None:
                    encdata.extend((val, repeat))   # noqa: F821
                val = d
                repeat = 1
            else:
                repeat += 1
        encdata.extend((val, repeat))
        # If we didn't save any space, return the original unchanged
        if len(encdata) > len(data):
            return data, None
        else:
            return encdata, encdict


def _encode(data, encoders):
    """Encode data using the given encoder objects. Return the encoded data
       and a list of BinaryCIF encoding dicts."""
    encdicts = []
    for enc in encoders:
        data, encdict = enc(data)
        if encdict is not None:
            encdicts.append(encdict)
    return data, encdicts


class _MaskedEncoder(object):
    """Base class for all encoders that handle potentially masked data"""

    def __call__(self, data, mask):
        """Given raw data `data`, and `mask`, return encoded data"""
        pass


class _StringArrayMaskedEncoder(_MaskedEncoder):
    _int_encoders = [_DeltaEncoder(), _RunLengthEncoder(),
                     _ByteArrayEncoder()]

    def __call__(self, data, mask):
        seen_substrs = {}  # keys are substrings, values indices
        sorted_substrs = []
        indices = []
        for i, reals in enumerate(data):
            if mask is not None and mask[i]:
                indices.append(-1)
            else:
                s = reals
                # Map bool to YES/NO strings
                if isinstance(s, bool):
                    s = ihm.format._Writer._boolmap[s]
                else:
                    s = str(s)  # coerce non-str data to str
                if s not in seen_substrs:
                    seen_substrs[s] = len(seen_substrs)
                    sorted_substrs.append(s)
                indices.append(seen_substrs[s])
        offsets = [0]
        total_len = 0
        for s in sorted_substrs:
            total_len += len(s)
            offsets.append(total_len)

        data_offsets, enc_offsets = _encode(offsets, self._int_encoders)
        data_indices, enc_indices = _encode(indices, self._int_encoders)

        enc_dict = {u'kind': u'StringArray',
                    u'dataEncoding': enc_indices,
                    u'stringData': _encode_str(''.join(sorted_substrs)),
                    u'offsetEncoding': enc_offsets,
                    u'offsets': data_offsets}
        return data_indices, [enc_dict]


class _IntArrayMaskedEncoder(_MaskedEncoder):
    _encoders = [_DeltaEncoder(), _RunLengthEncoder(), _ByteArrayEncoder()]

    def __call__(self, data, mask):
        if mask:
            masked_data = [-1 if m else d for m, d in zip(mask, data)]
        else:
            masked_data = data
        encdata, encoders = _encode(masked_data, self._encoders)
        return encdata, encoders


class _FloatArrayMaskedEncoder(_MaskedEncoder):
    _encoders = [_ByteArrayEncoder()]

    def __call__(self, data, mask):
        if mask:
            masked_data = [0. if m else d for m, d in zip(mask, data)]
        else:
            masked_data = data
        encdata, encoders = _encode(masked_data, self._encoders)
        return encdata, encoders


def _get_mask_and_type(data):
    """Detect missing/omitted values in `data` and determine the type of
       the remaining values (str, int, float)"""
    mask = None
    seen_types = set()
    for i, val in enumerate(data):
        if val is None or val == ihm.unknown:
            if mask is None:
                mask = [0] * len(data)
            mask[i] = 1 if val is None else 2
        else:
            seen_types.add(type(val))
    # If a mix of types, coerce to that of the highest precendence
    # (mixed int/float can be represented as float; mix int/float/str can
    # be represented as str; bool is represented as str)
    if not seen_types or bool in seen_types or str in seen_types:
        return mask, str
    elif float in seen_types:
        return mask, float
    elif int in seen_types:
        return mask, int
    elif sys.version_info[0] < 3 and long in seen_types:   # noqa: F821
        # Handle long like int (we don't have a 64-bit int type in BCIF anyway,
        # so hopefully the data can be represented in an int)
        return mask, int
    for t in seen_types:
        # Handle numpy float types like Python float
        # todo: this is a hack
        if 'numpy.float' in str(t):
            return mask, float
    raise ValueError("Cannot determine type of data %s" % data)


class BinaryCifWriter(ihm.format._Writer):
    """Write information to a BinaryCIF file. See :class:`ihm.format.CifWriter`
       for more information. The constructor takes a single argument - a Python
       filelike object, open for writing in binary mode."""

    _mask_encoders = [_DeltaEncoder(), _RunLengthEncoder(),
                      _ByteArrayEncoder()]

    def __init__(self, fh):
        super(BinaryCifWriter, self).__init__(fh)
        self._blocks = []
        self._masked_encoder = {str: _StringArrayMaskedEncoder(),
                                int: _IntArrayMaskedEncoder(),
                                float: _FloatArrayMaskedEncoder()}

    def category(self, category):
        """See :meth:`ihm.format.CifWriter.category`."""
        return _CategoryWriter(self, category)

    def loop(self, category, keys):
        """See :meth:`ihm.format.CifWriter.loop`."""
        return _LoopWriter(self, category, keys)

    def write_comment(self, comment):
        """See :meth:`ihm.format.CifWriter.write_comment`.

           .. note::

              BinaryCIF does not support comments, so this is a noop.
        """
        pass

    def _encode_data(self, data):
        mask, typ = _get_mask_and_type(data)
        enc = self._masked_encoder[typ]
        encdata, encs = enc(data, mask)
        if mask:
            data_mask, enc_mask = _encode(mask, self._mask_encoders)
            mask = {u'data': data_mask, u'encoding': enc_mask}
        return mask, encdata, encs

    def _encode_column(self, name, data):
        mask, encdata, encs = self._encode_data(data)
        return {u'name': _encode_str(name), u'mask': mask,
                u'data': {u'data': encdata, u'encoding': encs}}

    def start_block(self, name):
        """See :meth:`ihm.format.CifWriter.start_block`."""
        block = {u'header': _encode_str(name), u'categories': []}
        self._categories = block[u'categories']
        self._blocks.append(block)

    def end_block(self):
        # noop - end-of-block is handled by start_block() and flush()
        pass

    def _add_category(self, category, data):
        row_count = 0
        cols = []
        for k, v in data.items():
            row_count = len(v)
            # Do nothing if the category has no data
            if row_count == 0:
                return
            cols.append(self._encode_column(k, v))
        self._categories.append({u'name': _encode_str(category),
                                 u'columns': cols, u'rowCount': row_count})

    def flush(self):
        data = {u'version': _encode_str(ihm.__version__),
                u'encoder': u'python-ihm library',
                u'dataBlocks': self._blocks}
        self._write_msgpack(data)

    def _write_msgpack(self, data):
        """Read the msgpack data from the file and return data blocks"""
        import msgpack
        msgpack.pack(data, self.fh, use_bin_type=True)
