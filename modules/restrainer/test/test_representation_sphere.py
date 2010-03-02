import sys
import os
import unittest
import IMP
import IMP.test
import IMP.restrainer

class RepresentationTest(IMP.test.TestCase):
    def test_representation_parser(self):
        """Check sphere representation"""

        RepParser = IMP.restrainer.XMLRepresentation(self.get_input_file_name('sphere_representation.xml'))
        repr = RepParser.run()

        M = repr.to_model()
        M.show()
        M.evaluate(False)
        self.assertEqual(len(M.get_particles()), 17)

if __name__ == '__main__':
    unittest.main()
