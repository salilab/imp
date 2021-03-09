import unittest


class IHMInstallTests(unittest.TestCase):

    def test_import(self):
        """Check that ihm package was installed"""
        import ihm
        import ihm.dataset
        import ihm.restraint
        import ihm.model
        import ihm.format
        # We should also have the C-accelerated tokenizer
        self.assertTrue(ihm.format._format is not None)
        _ = ihm.System()


if __name__ == '__main__':
    unittest.main()
