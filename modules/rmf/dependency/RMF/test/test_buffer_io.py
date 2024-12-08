import unittest
import RMF


class Tests(unittest.TestCase):

    def test_data_types(self):
        """Test that import works"""
        # Make sure that interface is 8-bit clean
        buf = b"test string \x82\x00\xb8 with 8-bit data and embedded null"
        RMF.set_log_level("trace")
        bh = RMF.BufferConstHandle(buf)
        name = RMF._get_temporary_file_path("buffer")
        print(name)
        self.assertEqual(bh.get_buffer(), buf)
        RMF.write_buffer(bh, name)
        bbh = RMF.read_buffer(name)
        self.assertEqual(bbh.get_buffer(), bh.get_buffer())

if __name__ == '__main__':
    unittest.main()
