import unittest
import ihm.reader
import ihm.dumper
import urllib.request
import os


class Tests(unittest.TestCase):
    def _read_cif(self, pdb_id):
        url = 'https://pdb-ihm.org/cif/%s.cif' % pdb_id
        with urllib.request.urlopen(url) as fh:
            s, = ihm.reader.read(fh)
        return s

    def _write_cif(self, s, check=True):
        with open('test.cif', 'w') as fh:
            ihm.dumper.write(fh, [s], check=check)
        os.unlink('test.cif')

    def test_9a0e(self):
        """Test IMP structure without errors (9a0e)"""
        s = self._read_cif('9a0e')
        # This used to have an incorrect reference sequence, now fixed.
        self._write_cif(s)

    def test_8zzd(self):
        """Test docking structure without errors (8zzd)"""
        s = self._read_cif('8zzd')
        # This used to have an incorrect assembly, now fixed.
        self._write_cif(s)

    def test_9a82(self):
        """Test HADDOCK structure without errors (9a82)"""
        s = self._read_cif('9a82')
        self._write_cif(s)

    def test_9a13(self):
        """Test HADDOCK structure without errors (9a13)"""
        s = self._read_cif('9a13')
        # This used to fail because feature #71 was null, but this
        # is now fixed.
        self._write_cif(s)

    def test_9a0t(self):
        """Test IMP structure without errors (9a0t)"""
        s = self._read_cif('9a0t')
        self._write_cif(s)


if __name__ == '__main__':
    unittest.main()
