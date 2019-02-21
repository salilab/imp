import utils
import os
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.reader
import ihm.dumper

class Tests(unittest.TestCase):
    def test_entity(self):
        """Test Entity read followed by write"""
        sin = StringIO("""
loop_
_entity.id
_entity.type
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.formula_weight
_entity.details
1 polymer Nup84 2 100.0 .
""")
        s, = ihm.reader.read(sin)
        sout = StringIO()
        ihm.dumper.write(sout, [s])


if __name__ == '__main__':
    unittest.main()
