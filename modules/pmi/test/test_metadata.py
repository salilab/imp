from __future__ import print_function
import IMP.test
import IMP.pmi.representation
import ihm.location
import os

class Tests(IMP.test.TestCase):

    def test_repr_add(self):
        """Test Representation.add_metadata()"""
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.add_metadata(ihm.location.Repository(
                                   doi='10.5281/zenodo.46266', root='..'))
        self.assertEqual(r._metadata[0]._root, os.path.abspath('..'))

if __name__ == '__main__':
    IMP.test.main()
