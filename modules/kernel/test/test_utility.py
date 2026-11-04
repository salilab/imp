import IMP.test


class Tests(IMP.test.TestCase):

    def test_unique_name(self):
        """Test get_unique_name()"""
        self.assertEqual(IMP.get_unique_name("test%1%"), "test0")
        self.assertEqual(IMP.get_unique_name("test%1%"), "test1")
        self.assertRaises(ValueError, IMP.get_unique_name, "test%2%")
        # No format directive
        self.assertEqual(IMP.get_unique_name("plain"), "plain")
        # Directive not at the end of the name; separate count for each name
        self.assertEqual(IMP.get_unique_name("a%1%b"), "a0b")
        self.assertEqual(IMP.get_unique_name("a%1%b"), "a1b")
        self.assertEqual(IMP.get_unique_name("%1%"), "0")
        # Multiple directives or other formats are handled by boost::format
        self.assertEqual(IMP.get_unique_name("c%1%d%1%"), "c0d0")
        self.assertEqual(IMP.get_unique_name("c%1%d%1%"), "c1d1")
        self.assertEqual(IMP.get_unique_name("e%%f%1%"), "e%f0")
        self.assertEqual(IMP.get_unique_name("g%1$03d"), "g000")
        self.assertRaises(ValueError, IMP.get_unique_name, "test%1")

    def test_copyright(self):
        """Test get_copyright()"""
        c = IMP.get_copyright()
        self.assertIn('Copyright', c)
        self.assertIn('IMP Inventors', c)


if __name__ == '__main__':
    IMP.test.main()
