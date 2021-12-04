import utils
import os
import unittest
import sys
from test_format_bcif import MockMsgPack, MockFh

if sys.version_info[0] >= 3:
    from io import StringIO, BytesIO
else:
    from io import BytesIO
    StringIO = BytesIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.dictionary


def add_keyword(name, mandatory, category):
    k = ihm.dictionary.Keyword()
    k.name, k.mandatory = name, mandatory
    category.keywords[k.name] = k
    return k


def make_test_dictionary():
    d = ihm.dictionary.Dictionary()

    c = ihm.dictionary.Category()
    c.name = 'test_mandatory_category'
    c.mandatory = True
    add_keyword("foo", False, c)
    k = add_keyword("bar", True, c)
    k.item_type = ihm.dictionary.ItemType('int', 'numb', '[+-]?[0-9]+')
    d.categories[c.name] = c

    c = ihm.dictionary.Category()
    c.name = 'test_optional_category'
    c.mandatory = False
    k = add_keyword("foo", False, c)
    # For testing we only accept upper case values
    k.item_type = ihm.dictionary.ItemType('text', 'char', r'[ \n\t_()A-Z]+')
    k = add_keyword("bar", True, c)
    k.enumeration = set(('enum1', 'enum2'))
    add_keyword("baz", False, c)
    d.categories[c.name] = c

    d.linked_items = {'_test_optional_category.baz':
                      '_test_mandatory_category.foo',
                      '_test_optional_category.foo': '_entity.id'}
    return d


def make_other_test_dictionary():
    d = ihm.dictionary.Dictionary()

    c = ihm.dictionary.Category()
    c.name = 'ext_category'
    c.mandatory = False
    add_keyword("foo", False, c)
    d.categories[c.name] = c

    d.linked_items = {'_ext_category.foo': '_test_mandatory_category.foo'}
    return d


class Tests(unittest.TestCase):
    def test_keyword_enum_case_insen(self):
        """Test KeywordEnumeration (case insensitive)"""
        x = ihm.dictionary._KeywordEnumeration()
        x.case_sensitive = False
        self.assertNotIn('foo', x)
        x.add('foo')
        self.assertNotIn('bar', x)
        self.assertIn('foo', x)
        self.assertIn('FOO', x)
        x.add('bar')
        self.assertIn('BAR', x)

    def test_keyword_enum_case_sen(self):
        """Test KeywordEnumeration (case sensitive)"""
        x = ihm.dictionary._KeywordEnumeration()
        self.assertNotIn('foo', x)
        x.add('foo')
        self.assertNotIn('bar', x)
        self.assertIn('foo', x)
        self.assertNotIn('FOO', x)
        x.add('bar')
        self.assertNotIn('BAR', x)

    def test_read(self):
        """Test read() function"""
        # Note that _item.category_id is intentionally missing from
        # save_unknown_code
        cif = r"""
loop_
_item_type_list.code
_item_type_list.primitive_code
_item_type_list.construct
code char '[][_,.;:"&<>()/\{}'`~!@#$%A-Za-z0-9*|+-]*'
ucode uchar '[][_,.;:"&<>()/\{}'`~!@#$%A-Za-z0-9*|+-]*'

save_foo
  _category.id               test_category1
  _category.mandatory_code   yes
save_

save_bar
  loop_
  _item.name
  _item.category_id
  _item.mandatory_code
        '_test_category1.bar'     test_category1     no
        '_test_category3.bar'     test_category3     yes

  _item_type.code            code
save_

save_unknown_code
  _item.name                 '_test_category1.unknown_code'
  _item.mandatory_code       no
  _item_type.code            atcode
save_

save_missing_code
  _item.name                 '_test_category1.missing_code'
  _item.category_id          test_category1
  _item.mandatory_code       no
save_

save_insensitive_code
  _item.name                 '_test_category1.insensitive_code'
  _item.category_id          test_category1
  _item.mandatory_code       no
  _item_type.code            ucode
save_

save_baz
  _item.name                 '_test_category2.baz'
  _item.category_id          test_category2
  _item.mandatory_code       no
  _item_type.code            ucode
  _item_linked.child_name    '_test_category2.baz'
  _item_linked.parent_name   '_test_category1.bar'
  loop_
    _item_enumeration.value
    "enum 1"
    "enum 2"
save_
"""
        d = ihm.dictionary.read(StringIO(cif))
        self.assertEqual(sorted(d.categories.keys()),
                         ['test_category1', 'test_category2',
                          'test_category3'])
        c1 = d.categories['test_category1']
        self.assertTrue(c1.mandatory)
        self.assertEqual(
            sorted(c1.keywords.keys()),
            ['bar', 'insensitive_code', 'missing_code', 'unknown_code'])
        self.assertFalse(c1.keywords['bar'].mandatory)
        self.assertIsNone(c1.keywords['bar'].enumeration)
        self.assertEqual(c1.keywords['bar'].item_type.name, "code")
        self.assertTrue(c1.keywords['bar'].item_type.case_sensitive)
        self.assertIsNone(c1.keywords['missing_code'].item_type)
        self.assertIsNone(c1.keywords['unknown_code'].item_type)
        self.assertFalse(
            c1.keywords['insensitive_code'].item_type.case_sensitive)

        c2 = d.categories['test_category2']
        self.assertIsNone(c2.mandatory)
        self.assertEqual(sorted(c2.keywords.keys()), ["baz"])
        self.assertFalse(c2.keywords['baz'].mandatory)
        self.assertEqual(c2.keywords['baz'].enumeration,
                         set(('enum 1', 'enum 2')))
        self.assertFalse(c2.keywords['baz'].enumeration.case_sensitive)
        self.assertFalse(c2.keywords['baz'].item_type.case_sensitive)

        c3 = d.categories['test_category3']
        self.assertIsNone(c3.mandatory)
        self.assertEqual(sorted(c3.keywords.keys()), ["bar"])
        self.assertTrue(c3.keywords['bar'].mandatory)

        self.assertEqual(d.linked_items,
                         {'_test_category2.baz': '_test_category1.bar'})

        if sys.version_info[0] >= 3:
            # Make sure that files can be read in binary mode
            d = ihm.dictionary.read(BytesIO(cif.encode('latin-1')))
            self.assertEqual(sorted(d.categories.keys()),
                             ['test_category1', 'test_category2',
                              'test_category3'])

    def test_add(self):
        """Test adding two Dictionaries"""
        d1 = make_test_dictionary()
        d2 = make_other_test_dictionary()
        d = d1 + d2
        self._check_test_dictionary(d1)
        self._check_other_test_dictionary(d2)
        self._check_summed_dictionary(d)

    def test_add_update(self):
        """Test add Dictionaries that both contain same Category"""
        d1 = make_test_dictionary()
        d2 = ihm.dictionary.Dictionary()

        c = ihm.dictionary.Category()
        c.name = 'test_mandatory_category'
        c.mandatory = True
        add_keyword("baz", False, c)
        d2.categories[c.name] = c
        d = d1 + d2
        self.assertEqual(sorted(d.categories.keys()),
                         ['test_mandatory_category', 'test_optional_category'])
        ks = sorted(d.categories['test_mandatory_category'].keywords.keys())
        # Category should now contain keywords from from dictionaries
        self.assertEqual(ks, ['bar', 'baz', 'foo'])

    def test_category_update(self):
        """Test Category._update()"""
        cman = ihm.dictionary.Category()
        cman.name = 'test_mandatory_category'
        cman.description = 'my description'
        cman.mandatory = True
        add_keyword("foo", False, cman)

        coth = ihm.dictionary.Category()
        coth.name = 'test_mandatory_category'
        coth.description = 'desc2'
        coth.mandatory = False
        add_keyword("bar", False, coth)

        cman._update(coth)
        self.assertIs(cman.mandatory, True)
        self.assertEqual(cman.description, 'my description')
        self.assertEqual(sorted(cman.keywords.keys()), ['bar', 'foo'])

        cnone = ihm.dictionary.Category()
        cnone.name = 'test_mandatory_category'
        cnone._update(coth)
        self.assertIs(cnone.mandatory, False)
        self.assertEqual(cnone.description, 'desc2')
        self.assertEqual(sorted(cnone.keywords.keys()), ['bar'])

    def test_add_inplace(self):
        """Test adding two Dictionaries in place"""
        d1 = make_test_dictionary()
        d2 = make_other_test_dictionary()
        d1 += d2
        self._check_other_test_dictionary(d2)
        self._check_summed_dictionary(d1)

    def _check_test_dictionary(self, d):
        self.assertEqual(sorted(d.categories.keys()),
                         ['test_mandatory_category', 'test_optional_category'])
        self.assertEqual(d.linked_items,
                         {'_test_optional_category.baz':
                             '_test_mandatory_category.foo',
                          '_test_optional_category.foo': '_entity.id'})

    def _check_other_test_dictionary(self, d):
        self.assertEqual(sorted(d.categories.keys()),
                         ['ext_category'])
        self.assertEqual(d.linked_items,
                         {'_ext_category.foo': '_test_mandatory_category.foo'})

    def _check_summed_dictionary(self, d):
        self.assertEqual(sorted(d.categories.keys()),
                         ['ext_category', 'test_mandatory_category',
                          'test_optional_category'])
        self.assertEqual(d.linked_items,
                         {'_test_optional_category.baz':
                             '_test_mandatory_category.foo',
                          '_test_optional_category.foo': '_entity.id',
                          '_ext_category.foo': '_test_mandatory_category.foo'})

    def test_validate_ok(self):
        """Test successful validation"""
        d = make_test_dictionary()
        d.validate(StringIO("_test_mandatory_category.bar 1"))

    def test_validate_ok_binary_cif(self):
        """Test successful validation of BinaryCIF input"""
        sys.modules['msgpack'] = MockMsgPack
        d = make_test_dictionary()
        fh = MockFh()
        writer = ihm.format_bcif.BinaryCifWriter(fh)
        writer.start_block('ihm')
        with writer.category('_test_mandatory_category') as loc:
            loc.write(bar=1)
        with writer.category('_test_optional_category') as loc:
            loc.write(bar='enum1')
        writer.flush()
        d.validate(fh.data, format='BCIF')

    def test_validate_multi_data_ok(self):
        """Test successful validation of multiple data blocks"""
        d = make_test_dictionary()
        d.validate(StringIO("""
data_block1
_test_mandatory_category.bar 1

data_block2
_test_mandatory_category.bar 2
"""))

    def test_validate_missing_mandatory_category(self):
        """Test validation failure with missing mandatory category"""
        d = make_test_dictionary()
        self.assertRaises(ihm.dictionary.ValidatorError,
                          d.validate, StringIO("_struct.entry_id id1"))

    def test_validate_missing_mandatory_keyword(self):
        """Test validation failure with missing mandatory keyword"""
        d = make_test_dictionary()
        # mandatory 'bar' is marked unknown
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO("_test_mandatory_category.bar ?"))
        # mandatory 'bar' is missing entirely
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO("_test_mandatory_category.foo xy"))

    def test_validate_enumeration(self):
        """Test validation of enumerated values"""
        prefix = """_test_mandatory_category.bar 1
                    _test_optional_category.bar """
        d = make_test_dictionary()
        # Value in the enumeration is OK
        d.validate(StringIO(prefix + 'enum1'))
        # Omitted value is OK
        d.validate(StringIO(prefix + '.'))
        # Value not in the enumeration is not OK
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + 'bad'))

    def test_validate_item_type_int(self):
        """Test validation of int item type"""
        prefix = "_test_mandatory_category.bar "
        d = make_test_dictionary()
        # Int value is OK
        d.validate(StringIO(prefix + '+45'))
        d.validate(StringIO(prefix + '-4'))
        d.validate(StringIO(prefix + '5'))
        # Omitted value is OK
        d.validate(StringIO(prefix + '.'))
        # Non-int value is not OK
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + '45A'))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + 'foo'))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + '++44'))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + '44+'))

    def test_validate_item_type_multiline(self):
        """Test validation of multiline item type"""
        # This regex '[ \n\t_()A-Z]+' includes \n and \t special characters,
        # which should match newline and tab, not literal \n and \t
        prefix = """_test_mandatory_category.bar 1
                    _test_optional_category.bar enum1
                    _test_optional_category.foo """
        d = make_test_dictionary()
        # OK strings
        d.validate(StringIO(prefix + '"FOO BAR"'))
        d.validate(StringIO(prefix + '"FOO_BAR"'))
        d.validate(StringIO(prefix + '"FOO\tBAR"'))
        d.validate(StringIO(prefix + '\n;FOO\nBAR\n;'))
        # Bad strings
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + '"foo BAR"'))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + '"FOO\\BAR"'))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + 'n'))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix + 't'))

    def test_item_type_bad_regex(self):
        """Make sure that ItemType handles invalid regex"""
        # "+" is not a valid Python regex; it should be skipped and will
        # match any value
        it = ihm.dictionary.ItemType("test", "text", "+")
        self.assertTrue(it.regex.match("something"))
        self.assertTrue(it.regex.match(None))

    def test_validate_linked_items(self):
        """Test validation of linked items"""
        prefix = "_test_mandatory_category.bar 1\n"

        d = make_test_dictionary()

        c = ihm.dictionary.Category()
        c.name = 'chem_comp_atom'
        add_keyword("foo", False, c)
        d.categories[c.name] = c
        d.linked_items['_test_optional_category.bar'] \
            = '_chem_comp_atom.atom_id'

        # OK: same key in child and parent
        d.validate(StringIO(prefix +
                            "_test_optional_category.bar .\n"
                            "_test_optional_category.baz 42\n"
                            "_test_mandatory_category.foo 42"))
        # OK: missing parent key but in category not in the dictionary
        d.validate(StringIO(prefix +
                            "_test_optional_category.bar .\n"
                            "_test_optional_category.foo AB"))
        # OK: missing parent key but chem_comp_* is explicitly excluded
        # from validation
        d.validate(StringIO(prefix +
                            "_test_optional_category.bar enum1"))
        # Not OK: parent is missing or does not include the child key
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix +
                                   "_test_optional_category.bar .\n"
                                   "_test_optional_category.baz 42\n"
                                   "_test_mandatory_category.foo 24"))
        self.assertRaises(ihm.dictionary.ValidatorError, d.validate,
                          StringIO(prefix +
                                   "_test_optional_category.bar .\n"
                                   "_test_optional_category.baz 42"))

    def test_unknown_category(self):
        """Test validator failure for unknown categories"""
        d = make_test_dictionary()
        self.assertRaises(
            ihm.dictionary.ValidatorError, d.validate,
            StringIO("_test_mandatory_category.bar 1\n_foo.bar baz"))

    def test_unknown_keyword(self):
        """Test validator failure for unknown keywords"""
        d = make_test_dictionary()
        self.assertRaises(
            ihm.dictionary.ValidatorError, d.validate,
            StringIO("_test_mandatory_category.bar 1\n"
                     "_test_mandatory_category.unk 42"))


if __name__ == '__main__':
    unittest.main()
