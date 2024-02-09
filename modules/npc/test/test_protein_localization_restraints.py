import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.npc
import pickle


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
    for typ in ('int', 'float', 'string', 'filename', 'floats', 'filenames',
                'particle_indexes'):
        for i in range(getattr(info, 'get_number_of_' + typ)()):
            key = getattr(info, 'get_%s_key' % typ)(i)
            value = getattr(info, 'get_%s_value' % typ)(i)
            d[key] = value
    return d


class Tests(IMP.test.TestCase):

    def _check_pickle(self, r, score):
        """Check that the restraint r can be (un-)pickled"""
        r.set_name('foo')
        self._check_pickle_non_polymorphic(r, score)
        self._check_pickle_polymorphic(r, score)

    def _check_pickle_non_polymorphic(self, r, score):
        self.assertAlmostEqual(r.evaluate(False), score, delta=1e-3)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(False), score, delta=1e-3)

    def _check_pickle_polymorphic(self, r, score):
        sf = IMP.core.RestraintsScoringFunction([r])
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")
        self.assertAlmostEqual(newr.evaluate(False), score, delta=1e-3)

    def test_z_axial(self):
        """Test ZAxialPositionRestraint"""
        m, p = setup_system()
        r = IMP.npc.ZAxialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.ZAxialPositionRestraint')
        self._check_pickle(r, score=3.920)

    def test_z_axial_lower(self):
        """Test ZAxialPositionLowerRestraint"""
        m, p = setup_system()
        r = IMP.npc.ZAxialPositionLowerRestraint(m, [p], 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.ZAxialPositionLowerRestraint')
        self._check_pickle(r, score=3.920)

    def test_z_axial_upper(self):
        """Test ZAxialPositionUpperRestraint"""
        m, p = setup_system()
        r = IMP.npc.ZAxialPositionUpperRestraint(m, [p], -5.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper bound'], -5.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.ZAxialPositionUpperRestraint')
        self._check_pickle(r, score=58.32)

    def test_y_axial(self):
        """Test YAxialPositionRestraint"""
        m, p = setup_system()
        r = IMP.npc.YAxialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.YAxialPositionRestraint')
        self._check_pickle(r, score=3.920)

    def test_y_axial_lower(self):
        """Test YAxialPositionLowerRestraint"""
        m, p = setup_system()
        r = IMP.npc.YAxialPositionLowerRestraint(m, [p], 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.YAxialPositionLowerRestraint')
        self._check_pickle(r, score=3.920)

    def test_y_axial_upper(self):
        """Test YAxialPositionUpperRestraint"""
        m, p = setup_system()
        r = IMP.npc.YAxialPositionUpperRestraint(m, [p], -2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper bound'], -2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.YAxialPositionUpperRestraint')
        self._check_pickle(r, score=11.52)

    def test_xy_radial(self):
        """Test XYRadialPositionRestraint"""
        m, p = setup_system()
        r = IMP.npc.XYRadialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.XYRadialPositionRestraint')
        self._check_pickle(r, score=0.32)

    def test_xy_radial_lower(self):
        """Test XYRadialPositionLowerRestraint"""
        m, p = setup_system()
        r = IMP.npc.XYRadialPositionLowerRestraint(m, [p], 10.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 10.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.XYRadialPositionLowerRestraint')
        self._check_pickle(r, score=141.12)

    def test_xy_radial_upper(self):
        """Test XYRadialPositionUpperRestraint"""
        m, p = setup_system()
        r = IMP.npc.XYRadialPositionUpperRestraint(m, [p], 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.XYRadialPositionUpperRestraint')
        self._check_pickle(r, score=0.32)

    def test_membrane_surface_localization_restraint(self):
        """Test MembraneSurfaceLocationRestraint"""
        m, p = setup_system()
        r = IMP.npc.MembraneSurfaceLocationRestraint(m, [p], 40.0, 10.0,
                                                     3.0, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['major radius'], 40.0, delta=1e-4)
        self.assertAlmostEqual(info['minor radius'], 10.0, delta=1e-4)
        # thickness is half of user value
        self.assertAlmostEqual(info['thickness'], 1.5, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.MembraneSurfaceLocationRestraint')
        self._check_pickle(r, score=1568.0)

    def test_membrane_exclusion_restraint(self):
        """Test MembraneExclusionRestraint"""
        m, p = setup_system()
        p.set_coordinates([40., 2., 3.])
        r = IMP.npc.MembraneExclusionRestraint(m, [p], 40.0, 10.0, 3.0, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['major radius'], 40.0, delta=1e-4)
        self.assertAlmostEqual(info['minor radius'], 10.0, delta=1e-4)
        # thickness is half of user value
        self.assertAlmostEqual(info['thickness'], 1.5, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.MembraneExclusionRestraint')
        self._check_pickle(r, score=144.5)

    def test_membrane_surface_location_conditional_restraint(self):
        """Test MembraneSurfaceLocationConditionalRestraint"""
        m, p1 = setup_system()
        p2 = IMP.core.XYZR.setup_particle(
           IMP.Particle(m),
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4,0,0.), 0.4))
        r = IMP.npc.MembraneSurfaceLocationConditionalRestraint(
            m, [p1], [p2], 40.0, 10.0, 3.0, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['major radius'], 40.0, delta=1e-4)
        self.assertAlmostEqual(info['minor radius'], 10.0, delta=1e-4)
        # thickness is half of user value
        self.assertAlmostEqual(info['thickness'], 1.5, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'],
                         'IMP.npc.MembraneSurfaceLocationConditionalRestraint')
        p1ind, = info['particles1']
        self.assertEqual(p1ind, p1.get_particle_index())
        p2ind, = info['particles2']
        self.assertEqual(p2ind, p2.get_particle_index())
        self._check_pickle(r, score=1352.0)

    def test_x_axial(self):
        """Test XAxialPositionRestraint"""
        m, p = setup_system()
        p.set_coordinates([0.0,2.0,2.0])
        r = IMP.npc.XAxialPositionRestraint(m, [p], 1.0, 2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['upper bound'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.XAxialPositionRestraint')
        self._check_pickle(r, score=3.920)


    def test_x_axial_lower(self):
        """Test XAxialPositionLowerRestraint"""
        m, p = setup_system()
        p.set_coordinates([0.0,2.0,2.0])
        r = IMP.npc.XAxialPositionLowerRestraint(m, [p], 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['lower bound'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.XAxialPositionLowerRestraint')
        self._check_pickle(r, score=3.920)


    def test_x_axial_upper(self):
        """Test XAxialPositionUpperRestraint"""
        m, p = setup_system()
        p.set_coordinates([0.0,2.0,2.0])
        r = IMP.npc.XAxialPositionUpperRestraint(m, [p], -2.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['upper bound'], -2.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.XAxialPositionUpperRestraint')
        self._check_pickle(r, score=11.52)


    def test_overall(self):
        """Test OverallPositionRestraint"""
        m, p = setup_system()
        p.set_coordinates([0.0,1.0,2.0])
        r = IMP.npc.OverallPositionRestraint(m, [p], 2.0, 0.0, 3.0, 1.0, True, 0.5)
        info = _parse_restraint_info(r.get_static_info())
        self.assertAlmostEqual(info['X start'], 2.0, delta=1e-4)
        self.assertAlmostEqual(info['Y start'], 0.0, delta=1e-4)
        self.assertAlmostEqual(info['Z start'], 3.0, delta=1e-4)
        self.assertAlmostEqual(info['tolerance'], 1.0, delta=1e-4)
        self.assertAlmostEqual(info['sigma'], 0.5, delta=1e-4)
        self.assertEqual(info['type'], 'IMP.npc.OverallPositionRestraint')
        self._check_pickle(r, score=2.203)


if __name__ == '__main__':
    IMP.test.main()
