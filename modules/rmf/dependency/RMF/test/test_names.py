import RMF
import unittest

class Tests(unittest.TestCase):

    def test_get_as_node_name(self):
        """Test get_as_node_name() function"""
        self.assertEqual(RMF.get_as_node_name('foo"bar":=()[]{}\'\\baz'),
                         'foobar:=()[]{}\'\\baz')

    def test_is_valid_node_name(self):
        """Test get_is_valid_node_name() function"""
        self.assertFalse(RMF.get_is_valid_node_name(""))
        self.assertFalse(RMF.get_is_valid_node_name('foo"bar":=()[]{}\'\\baz'))
        self.assertTrue(RMF.get_is_valid_node_name('foo   bar:=()[]{}\'\\baz'))

    def test_is_valid_key_name(self):
        """Test get_is_valid_key_name() function"""
        self.assertFalse(RMF.get_is_valid_key_name(""))
        self.assertFalse(RMF.get_is_valid_key_name('foo"bar":=()[]{}\'\\baz'))
        self.assertFalse(RMF.get_is_valid_key_name('foobar:=()[]{}\'\\baz'))
        self.assertFalse(RMF.get_is_valid_key_name('foo  bar'))
        self.assertTrue(RMF.get_is_valid_key_name('foo bar'))
        self.assertTrue(RMF.get_is_valid_key_name('foobar'))

if __name__ == '__main__':
    unittest.main()
