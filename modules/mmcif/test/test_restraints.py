import IMP.test
import IMP.mmcif.restraint
import ihm.dataset


def make_model(m, chains=None):
    if chains is None:
        chains = (('foo', 'ACGT', 'A'), ('bar', 'ACGT', 'B'),
                  ('baz', 'ACC', 'C'))
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
    return top


class MockGaussianEMRestraint(IMP.Restraint):

    def __init__(self, m, em_filename, inputs=[]):
        self.em_filename = em_filename
        self.inputs = inputs
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")
    def do_show(self, fh):
        fh.write('MockRestraint')
    def do_get_inputs(self):
        return self.inputs

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

    def __init__(self, m, image_filenames):
        self.image_filenames = image_filenames
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
        i.add_filenames("image files", self.image_filenames)
        i.add_float("pixel size", 2.2)
        i.add_float("resolution", 20.)
        i.add_int("projection number", 20)
        i.add_int("micrographs number", 0)
        return i

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        i.add_floats("cross correlation", [0.4, 0.3])
        i.add_floats("rotation", [0., 0., 1., 0.,
                                  0., 0., 1., 0.])
        i.add_floats("translation", [241.8, 17.4, 0.0,
                                     90.0, 80.0, 70.0])
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

    def test_all_restraints_none(self):
        """Test _AllRestraints with non-handled restraint"""
        s = ihm.System()
        comps = IMP.mmcif.data._ComponentMapper(s)
        m = IMP.Model()
        r = IMP._ConstRestraint(m, [], 1)
        rm = IMP.mmcif.restraint._AllRestraints(s, comps)
        rs = list(rm.handle(r, ["model0", "model1"]))
        self.assertEqual(len(rs), 0)

    def test_all_restraints_gaussian_em(self):
        """Test _AllRestraints with GaussianEM restraint"""
        s = ihm.System()
        comps = IMP.mmcif.data._ComponentMapper(s)
        m = IMP.Model()
        em_filename = self.get_input_file_name('test.gmm.txt')
        r = MockGaussianEMRestraint(m, em_filename)
        rm = IMP.mmcif.restraint._AllRestraints(s, comps)
        wr, = list(rm.handle(r, ["model0", "model1"]))
        self.assertEqual(type(wr), IMP.mmcif.restraint._EM3DRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.EMDensityDataset)
        self.assertAlmostEqual(
            wr.fits["model0"].cross_correlation_coefficient, 0.4, delta=1e-3)
        self.assertAlmostEqual(
            wr.fits["model1"].cross_correlation_coefficient, 0.4, delta=1e-3)

    def test_all_restraints_em2d_restraint(self):
        """Test _AllRestraints with IMP.em2d.PCAFitRestraint"""
        s = ihm.System()
        comps = IMP.mmcif.data._ComponentMapper(s)
        m = IMP.Model()
        image1 = self.get_input_file_name('image_1.pgm')
        image2 = self.get_input_file_name('image_2.pgm')
        r = MockPCAFitRestraint(m, [image1, image2])
        rm = IMP.mmcif.restraint._AllRestraints(s, comps)
        # Each image should map to a separate IHM restraint
        wr1, wr2 = list(rm.handle(r, ["model0", "model1"]))
        self.assertEqual(type(wr1), IMP.mmcif.restraint._EM2DRestraint)
        self.assertEqual(type(wr1.dataset), ihm.dataset.EM2DClassDataset)
        self.assertAlmostEqual(
            wr1.fits["model0"].cross_correlation_coefficient, 0.4, delta=1e-3)
        self.assertEqual(len(wr1.fits["model0"].rot_matrix), 3)
        self.assertEqual(len(wr1.fits["model0"].tr_vector), 3)

        self.assertEqual(type(wr2), IMP.mmcif.restraint._EM2DRestraint)
        self.assertEqual(type(wr2.dataset), ihm.dataset.EM2DClassDataset)
        self.assertAlmostEqual(
            wr2.fits["model0"].cross_correlation_coefficient, 0.3, delta=1e-3)
        self.assertEqual(len(wr2.fits["model0"].rot_matrix), 3)
        self.assertEqual(len(wr2.fits["model0"].tr_vector), 3)

    def test_all_restraints_saxs(self):
        """Test _AllRestraints with SAXS restraint"""
        s = ihm.System()
        comps = IMP.mmcif.data._ComponentMapper(s)
        m = IMP.Model()
        dat_filename = self.get_input_file_name('6lyz.pdb.dat')
        r = MockSAXSRestraint(m, dat_filename)
        rm = IMP.mmcif.restraint._AllRestraints(s, comps)
        wr, = list(rm.handle(r, ["model0", "model1"]))
        self.assertEqual(type(wr), IMP.mmcif.restraint._SAXSRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.SASDataset)
        self.assertIsNone(wr.fits["model0"].chi_value)
        self.assertIsNone(wr.fits["model1"].chi_value)

    def test_all_restraints_cross_link(self):
        """Test _AllRestraints with cross-link restraint"""
        conv = IMP.mmcif.Writer()
        m = IMP.Model()
        h = make_model(m, chains=[("Rpb1", "AMT", "X"),
                                  ("Rpb2", "ACC", "Z")])
        conv.add_model([h], [])

        xl_csv = self.get_input_file_name('test.xlms.csv')
        r = MockCrossLinkRestraint(m, xl_csv)
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
        rm = IMP.mmcif.restraint._AllRestraints(conv.system, conv._components)
        wr, = list(rm.handle(r, ["model0", "model1"]))
        self.assertEqual(type(wr), IMP.mmcif.restraint._CrossLinkRestraint)
        self.assertEqual(type(wr.dataset), ihm.dataset.CXMSDataset)
        self.assertEqual(len(wr.experimental_cross_links), 1)
        self.assertEqual(len(wr.cross_links), 1)

    def test_all_restraints_zaxial(self):
        """Test _RestraintMapper with ZAxialPositionRestraint"""
        s = ihm.System()
        comps = IMP.mmcif.data._ComponentMapper(s)
        m = IMP.Model()
        r = MockZAxialRestraint(m)
        rm = IMP.mmcif.restraint._AllRestraints(s, comps)
        wr, = list(rm.handle(r, ["model0", "model1"]))
        self.assertEqual(type(wr), IMP.mmcif.restraint._ZAxialRestraint)
        self.assertIsNone(wr.dataset)


if __name__ == '__main__':
    IMP.test.main()
