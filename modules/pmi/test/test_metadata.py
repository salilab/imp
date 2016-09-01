from __future__ import print_function
import IMP.test
import IMP.pmi.metadata
import IMP.pmi.representation

class Tests(IMP.test.TestCase):

    def test_software(self):
        """Test metadata.Software"""
        s = IMP.pmi.metadata.Software(name='test', classification='test code',
                                      description='Some test program',
                                      url='http://salilab.org')
        self.assertEqual(s.name, 'test')

    def test_citation(self):
        """Test metadata.Citation"""
        s = IMP.pmi.metadata.Citation(title='Test paper', journal='J Mol Biol',
                                      volume=45, page_range=(1,20), year=2016,
                                      authors=['Smith, A.', 'Jones, B.'],
                                      doi='10.2345/S1384107697000225',
                                      pmid='1234')
        self.assertEqual(s.title, 'Test paper')

    def test_repository(self):
        """Test metadata.Repository"""
        s = IMP.pmi.metadata.Repository(doi='10.5281/zenodo.46266', root='..')
        self.assertEqual(s._root, '..')
        f = s.get_path('../foo')
        self.assertEqual(f.path, 'foo')

    def test_repr_add(self):
        """Test Representation.add_metadata()"""
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.add_metadata(IMP.pmi.metadata.Repository(
                                   doi='10.5281/zenodo.46266', root='..'))
        self.assertEqual(r._metadata[0]._root, '..')

if __name__ == '__main__':
    IMP.test.main()
