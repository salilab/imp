import ihm
import ihm.dumper
import ihm.reader
import os
import unittest
try:
    import msgpack
except ImportError:
    msgpack = None


class Tests(unittest.TestCase):
    def test_basic(self):
        """Basic install test"""
        system = ihm.System(title='test system')

        entity_a = ihm.Entity('AAA', description='Subunit A')
        entity_b = ihm.Entity('AAAAAA', description='Subunit B')
        system.entities.extend((entity_a, entity_b))

        with open('output.cif', 'w') as fh:
            ihm.dumper.write(fh, [system])

        with open('output.cif') as fh:
            sys2, = ihm.reader.read(fh)

        self.assertEqual(sys2.title, 'test system')
        os.unlink('output.cif')

        # Also test with BinaryCIF
        if msgpack:
            with open('output.bcif', 'wb') as fh:
                ihm.dumper.write(fh, [system], format='BCIF')
            with open('output.bcif', 'rb') as fh:
                sys2, = ihm.reader.read(fh, format='BCIF')
            self.assertEqual(sys2.title, 'test system')
            os.unlink('output.bcif')


if __name__ == '__main__':
    unittest.main()
