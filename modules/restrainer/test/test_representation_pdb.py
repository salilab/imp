import sys
import os
import unittest
import IMP
import IMP.test
import IMP.restrainer

class RepresentationTest(IMP.test.TestCase):
    def test_representation_parser(self):
        """Check pdb representation"""

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('pdb_representation.xml'))
        repr = RepParser.run()

        M = repr.to_model()
        M.show()
        M.evaluate(False)
        self.assertEqual(len(M.get_particles()), 356)

if __name__ == '__main__':
    unittest.main()
