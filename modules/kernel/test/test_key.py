import IMP
import IMP.test


class Tests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test key aliases"""
        k0 = IMP.IntKey("0")
        k1 = IMP.IntKey("1")
        ak1 = IMP.IntKey.add_alias(k0, "0p")
        k2 = IMP.IntKey("2")
        self.assertEqual(IMP.IntKey("0p"), k0)
        self.assertEqual(IMP.IntKey("2"), k2)
        self.assertNotEqual(k2, ak1)

    def test_key(self):
        """Test Key class methods"""
        self.assertFalse(IMP.FloatKey.get_key_exists("not-exist-key"))
        k = IMP.FloatKey.get_existing_key("not-exist-key")
        self.assertEqual(k, IMP.FloatKey())

        fk = IMP.FloatKey("exist-key")
        self.assertTrue(IMP.FloatKey.get_key_exists("exist-key"))
        k = IMP.FloatKey.get_existing_key("exist-key")
        self.assertEqual(k, fk)


if __name__ == '__main__':
    IMP.test.main()
