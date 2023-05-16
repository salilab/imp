import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.npc


def setup_system():
    m = IMP.Model()
    p = IMP.core.XYZR.setup_particle(
        IMP.Particle(m),
        IMP.algebra.Sphere3D(IMP.algebra.Vector3D(2,0,0.), 0.4))
    return m, p


def _parse_restraint_info(info):
    """Convert RestraintInfo object to Python dict"""
    d = {}
    if info is None:
        return d
    info.set_was_used(True)
    for typ in ('int', 'float', 'string', 'filename', 'floats', 'filenames'):
        for i in range(getattr(info, 'get_number_of_' + typ)()):
            key = getattr(info, 'get_%s_key' % typ)(i)
            value = getattr(info, 'get_%s_value' % typ)(i)
            d[key] = value
    return d


class Tests(IMP.test.TestCase):

    def test_z_axial(self):
        """Test ZAxialPositionRestraint"""
        m, p = setup_system()
        r = IMP.npc.ZAxialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower_bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper_bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.ZAxialPositionRestraint')

    def test_z_axial_lower(self):
        """Test ZAxialPositionLowerRestraint"""
        m, p = setup_system()
        r = IMP.npc.ZAxialPositionLowerRestraint(m, [p], 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower_bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.ZAxialPositionLowerRestraint')

    def test_z_axial_upper(self):
        """Test ZAxialPositionUpperRestraint"""
        m, p = setup_system()
        r = IMP.npc.ZAxialPositionUpperRestraint(m, [p], 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper_bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.ZAxialPositionUpperRestraint')

    def test_y_axial(self):
        """Test YAxialPositionRestraint"""
        m, p = setup_system()
        r = IMP.npc.YAxialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower_bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper_bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.YAxialPositionRestraint')

    def test_y_axial_lower(self):
        """Test YAxialPositionLowerRestraint"""
        m, p = setup_system()
        r = IMP.npc.YAxialPositionLowerRestraint(m, [p], 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower_bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.YAxialPositionLowerRestraint')

    def test_y_axial_upper(self):
        """Test YAxialPositionUpperRestraint"""
        m, p = setup_system()
        r = IMP.npc.YAxialPositionUpperRestraint(m, [p], 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper_bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.YAxialPositionUpperRestraint')

    def test_xy_radial(self):
        """Test XYRadialPositionRestraint"""
        m, p = setup_system()
        r = IMP.npc.XYRadialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower_bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper_bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.XYRadialPositionRestraint')

    def test_xy_radial_lower(self):
        """Test XYRadialPositionLowerRestraint"""
        m, p = setup_system()
        r = IMP.npc.XYRadialPositionLowerRestraint(m, [p], 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower_bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.XYRadialPositionLowerRestraint')

    def test_xy_radial_upper(self):
        """Test XYRadialPositionUpperRestraint"""
        m, p = setup_system()
        r = IMP.npc.XYRadialPositionUpperRestraint(m, [p], 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper_bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.XYRadialPositionUpperRestraint')

    def test_membrane_surface_localization_restraint(self):
        """Test MembraneSurfaceLocationRestraint"""
        m, p = setup_system()
        r = IMP.npc.MembraneSurfaceLocationRestraint(m, [p], 40.0, 10.0,
                                                     3.0, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['major_radius'], 40.0, delta=1e-4)
        self.assertAlmostEqual(info['minor_radius'], 10.0, delta=1e-4)
        # thickness is half of user value
        self.assertAlmostEqual(info['thickness'], 1.5, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.MembraneSurfaceLocationRestraint')

    def test_membrane_exclusion_restraint(self):
        """Test MembraneExclusionRestraint"""
        m, p = setup_system()
        r = IMP.npc.MembraneExclusionRestraint(m, [p], 40.0, 10.0, 3.0, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['major_radius'], 40.0, delta=1e-4)
        self.assertAlmostEqual(info['minor_radius'], 10.0, delta=1e-4)
        # thickness is half of user value
        self.assertAlmostEqual(info['thickness'], 1.5, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.MembraneExclusionRestraint')


if __name__ == '__main__':
    IMP.test.main()
