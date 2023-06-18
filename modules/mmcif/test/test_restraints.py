from __future__ import print_function
import IMP.test
import IMP.mmcif.restraint
import ihm.dataset


def make_model(system, chains=None):
    if chains is None:
        chains = (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                  ('baz', 'ACC', 'C'))
    s = IMP.mmcif.State(system)
    m = s.model
    top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
    for name, seq, cid in chains:
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        mol = IMP.atom.Molecule.setup_particle(h)
        mol.set_name(name)
        top.add_child(mol)

        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        chain = IMP.atom.Chain.setup_particle(h, cid)
        chain.set_sequence(seq)
        mol.add_child(chain)
    return top, s


class MockGaussianEMRestraint(IMP.Restraint):

    def __init__(self, m, em_filename):
        self.em_filename = em_filename
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.isd.GaussianEMRestraint")
        i.add_filename("filename", self.em_filename)
        return i

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        i.add_float("cross correlation", 0.4)
        return i


class MockCrossLinkRestraint(IMP.RestraintSet):

    def __init__(self, m, xlms_filename):
        self.xlms_filename = xlms_filename
        IMP.RestraintSet.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.pmi.CrossLinkingMassSpectrometryRestraint")
        i.add_filename("filename", self.xlms_filename)
        i.add_string("linker author name", "DSS")
        i.add_float("linker length", 21.0)
        return i


class MockSAXSRestraint(IMP.Restraint):

    def __init__(self, m, dat_filename):
        self.dat_filename = dat_filename
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.saxs.Restraint")
        i.add_string("form factor type", "residues")
        i.add_filename("filename", self.dat_filename)
        i.add_float("min q", 0.0)
        i.add_float("max q", 1.0)
        i.add_float("delta q", 0.01)
        return i


class MockZAxialRestraint(IMP.Restraint):
    def __init__(self, m):
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.npc.ZAxialPositionRestraint")
        i.add_float("lower bound", -10.0)
        i.add_float("upper bound", 20.0)
        i.add_float("sigma", 4.0)
        return i


class MockPCAFitRestraint(IMP.Restraint):

    def __init__(self, m, image_filename):
        self.image_filename = image_filename
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return []

    def get_static_info(self):
        i = IMP.RestraintInfo()
        i.add_string("type", "IMP.em2d.PCAFitRestraint")
        i.add_filenames("image files", [self.image_filename])
        i.add_float("pixel size", 2.2)
        i.add_float("resolution", 20.)
        i.add_int("projection number", 20)
        i.add_int("micrographs number", 0)
        return i

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        i.add_floats("cross correlation", [0.4])
        i.add_floats("rotation", [0., 0., 1., 0.])
        i.add_floats("translation", [241.8, 17.4, 0.0])
        return i


class Tests(IMP.test.TestCase):
    def test_parse_restraint_info_empty(self):
        """Test _parse_restraint_info() with empty RestraintInfo"""
        d = IMP.mmcif.restraint._parse_restraint_info(None)
        self.assertEqual(d, {})

    def test_parse_restraint_info(self):
        """Test _parse_restraint_info()"""
        info = IMP.RestraintInfo()
        info.add_int("test int", 42)
        info.add_float("test float", 5.6)
        info.add_string("test string", "foo")
        info.add_filename("test filename", "/foo")
        info.add_floats("test floats", [1.2, 3.4])
        info.add_filenames("test filenames", ["/foo", "/bar"])
        d = IMP.mmcif.restraint._parse_restraint_info(info)
        self.assertEqual(sorted(d.keys()),
                         ['test filename', 'test filenames', 'test float',
                          'test floats', 'test int', 'test string'])
        self.assertEqual(d['test int'], 42)
        self.assertAlmostEqual(d['test float'], 5.6, delta=1e-4)
        self.assertEqual(d['test string'], "foo")
        self.assertEqual(d['test filename'], "/foo")
        self.assertAlmostEqual(d['test floats'][0], 1.2, delta=1e-4)
        self.assertAlmostEqual(d['test floats'][1], 3.4, delta=1e-4)
        self.assertEqual(d['test filenames'], ["/foo", "/bar"])

    def test_restraint_mapper_none(self):
        """Test _RestraintMapper with non-handled restraint"""
        m = IMP.Model()
        frame = None
        r = IMP._ConstRestraint(m, [], 1)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(None)
        self.assertEqual(rm.handle(r, frame, None), None)

    def test_restraint_mapper_gaussian_em(self):
        """Test _RestraintMapper with GaussianEM restraint"""
        s = IMP.mmcif.System()
        m = IMP.Model()
        em_filename = self.get_input_file_name('test.gmm.txt')
        r = MockGaussianEMRestraint(m, em_filename)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(s)
        frame = None
        assembly = None
        wr = rm.handle(r, frame, assembly)
        self.assertEqual(type(wr), IMP.mmcif.restraint._GaussianEMRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.EMDensityDataset)

    def test_restraint_mapper_cross_link(self):
        """Test _RestraintMapper with cross-link restraint"""
        system = IMP.mmcif.System()
        h, state = make_model(system, chains=[("Rpb1", "AMT", "X"),
                                              ("Rpb2", "ACC", "Z")])
        IMP.mmcif.Ensemble(state, "cluster 1").add_model([h], [], "model1")
        m = state.model

        xl_csv = self.get_input_file_name('test.xlms.csv')
        r = MockCrossLinkRestraint(m, xl_csv)
        r.set_was_used(True)
        xl = IMP.isd.CrossLinkMSRestraint(m, 21.0, 0.01)
        xl.set_source_protein1('Rpb1')
        xl.set_source_residue1(34)
        xl.set_source_protein2('Rpb1')
        xl.set_source_residue2(49)
        rpb1 = IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0]
        h1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        rpb1.add_child(h1)
        rpb1.add_child(h2)
        s1 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 10.0)
        s2 = IMP.isd.Scale.setup_particle(IMP.Particle(m), 10.0)
        psi = IMP.isd.Scale.setup_particle(IMP.Particle(m), 0.5)
        xl.add_contribution((h1, h2), (s1, s2), psi)
        r.restraints.append(xl)
        rm = IMP.mmcif.restraint._RestraintMapper(system)
        frame = None
        assembly = None
        wr = rm.handle(r, frame, assembly)
        self.assertEqual(type(wr), IMP.mmcif.restraint._CrossLinkRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.CXMSDataset)
        self.assertEqual(len(wr.experimental_cross_links), 1)
        self.assertEqual(len(wr.cross_links), 1)

    def test_restraint_mapper_saxs(self):
        """Test _RestraintMapper with SAXS restraint"""
        s = IMP.mmcif.System()
        m = IMP.Model()
        dat_filename = self.get_input_file_name('6lyz.pdb.dat')
        r = MockSAXSRestraint(m, dat_filename)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(s)
        frame = None
        assembly = None
        wr = rm.handle(r, frame, assembly)
        self.assertEqual(type(wr), IMP.mmcif.restraint._SAXSRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.SASDataset)

    def test_restraint_mapper_zaxial(self):
        """Test _RestraintMapper with ZAxialPositionRestraint"""
        s = IMP.mmcif.System()
        m = IMP.Model()
        r = MockZAxialRestraint(m)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(s)
        frame = None
        assembly = None
        wr = rm.handle(r, frame, assembly)
        self.assertEqual(type(wr), IMP.mmcif.restraint._ZAxialRestraint)
        self.assertIsNone(wr.dataset)

    def test_restraint_mapper_em2d_restraint(self):
        """Test _RestraintMapper with IMP.em2d.PCAFitRestraint"""
        s = IMP.mmcif.System()
        m = IMP.Model()
        image_filename = self.get_input_file_name('image_2.pgm')
        r = MockPCAFitRestraint(m, image_filename)
        r.set_was_used(True)
        rm = IMP.mmcif.restraint._RestraintMapper(s)
        frame = None
        assembly = None
        wr = rm.handle(r, frame, assembly)
        self.assertEqual(type(wr), IMP.mmcif.restraint._EM2DRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.EM2DClassDataset)


if __name__ == '__main__':
    IMP.test.main()
