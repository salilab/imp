import IMP.test
import IMP.pmi
import pickle


def _parse_restraint_info(info):
    """Convert RestraintInfo object to Python dict"""
    d = {}
    if info is None:
        return d
    info.set_was_used(True)
    for typ in ('int', 'float', 'string', 'filename', 'floats', 'filenames',
                'particle_indexes'):
        for i in range(getattr(info, 'get_number_of_' + typ)()):
            key = getattr(info, 'get_%s_key' % typ)(i)
            value = getattr(info, 'get_%s_value' % typ)(i)
            d[key] = value
    return d


class Tests(IMP.test.TestCase):
    def test_metadata_no_linker(self):
        """Test metadata without linker info"""
        m = IMP.Model()
        r = IMP.pmi.CrossLinkRestraintSet(m, "foo")
        r.set_metadata('test_fn', 25.0, 0.1)
        info = _parse_restraint_info(r.get_static_info())
        self.assertEqual(len(info.keys()), 4)
        self.assertAlmostEqual(info['linker length'], 25.0, delta=1e-3)
        self.assertAlmostEqual(info['slope'], 0.1, delta=1e-3)
        self.assertEqual(info['filename'], 'test_fn')
        self.assertEqual(
            info['type'], 'IMP.pmi.CrossLinkingMassSpectrometryRestraint')

    def test_metadata_linker_smiles(self):
        """Test metadata with linker containing just SMILES info"""
        m = IMP.Model()
        r = IMP.pmi.CrossLinkRestraintSet(m, "foo")
        r.set_metadata('test_fn', 25.0, 0.1)
        r.set_linker_auth_name('DSS')
        r.set_linker_smiles('CC')
        info = _parse_restraint_info(r.get_static_info())
        self.assertEqual(
            sorted(info.keys()),
            ['filename', 'linker author name', 'linker length',
             'linker smiles', 'slope', 'type'])
        self.assertEqual(info['linker author name'], 'DSS')
        self.assertEqual(info['linker smiles'], 'CC')

    def test_metadata_linker_full(self):
        """Test metadata with linker containing full info"""
        m = IMP.Model()
        r = IMP.pmi.CrossLinkRestraintSet(m, "foo")
        r.set_metadata('test_fn', 25.0, 0.1)
        r.set_linker_auth_name('DSS')
        r.set_linker_chemical_name('chem')
        r.set_linker_smiles('CC')
        r.set_linker_smiles_canonical('CC2')
        r.set_linker_inchi('testinchi')
        r.set_linker_inchi_key('testinchikey')
        info = _parse_restraint_info(r.get_static_info())
        self.assertEqual(
            sorted(info.keys()),
            ['filename', 'linker author name', 'linker chemical name',
             'linker inchi', 'linker inchi key', 'linker length',
             'linker smiles', 'linker smiles canonical', 'slope', 'type'])
        self.assertEqual(info['linker author name'], 'DSS')
        self.assertEqual(info['linker chemical name'], 'chem')
        self.assertEqual(info['linker smiles'], 'CC')
        self.assertEqual(info['linker smiles canonical'], 'CC2')
        self.assertEqual(info['linker inchi'], 'testinchi')
        self.assertEqual(info['linker inchi key'], 'testinchikey')

    def test_pickle(self):
        """Test (un-)pickle of CrossLinkRestraintSet"""
        m = IMP.Model()
        r = IMP.pmi.CrossLinkRestraintSet(m, "foo")
        r.set_metadata('test_fn', 25.0, 0.1)
        dump = pickle.dumps(r)
        newrsr = pickle.loads(dump)
        self.assertEqual(newrsr.get_name(), "foo")
        info = _parse_restraint_info(newrsr.get_static_info())
        self.assertAlmostEqual(info['linker length'], 25.0, delta=1e-3)
        self.assertEqual(info['filename'], 'test_fn')

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of CrossLinkRestraintSet via polymorphic pointer"""
        m = IMP.Model()
        r = IMP.pmi.CrossLinkRestraintSet(m, "foo")
        r.set_metadata('test_fn', 25.0, 0.1)
        sf = IMP.core.RestraintsScoringFunction([r])
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newrsr, = newsf.restraints
        self.assertEqual(newrsr.get_name(), "foo")
        info = _parse_restraint_info(newrsr.get_static_info())
        self.assertAlmostEqual(info['linker length'], 25.0, delta=1e-3)
        self.assertEqual(info['filename'], 'test_fn')


if __name__ == '__main__':
    IMP.test.main()
