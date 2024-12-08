import unittest
import IMP.rmf
import IMP.test
import IMP.isd
import IMP.container
import RMF
from IMP.algebra import *

class MockRestraint(IMP.Restraint):

    def __init__(self, m, ps, val):
        self.ps = ps
        self._dyn_info = "foo"
        IMP.Restraint.__init__(self, m, "MockRestraint %1%")

    def unprotected_evaluate(self, accum):
        return 0.

    def get_version_info(self):
        return IMP.VersionInfo("IMP authors", "0.1")

    def do_show(self, fh):
        fh.write('MockRestraint')

    def do_get_inputs(self):
        return self.ps

    def get_static_info(self):
        i = IMP.RestraintInfo()
        if len(self.ps) >= 5:
            # Include a duplicate (which will use an Alias node in RMF)
            i.add_particle_indexes("static particles",
                    [self.ps[1], self.ps[2], self.ps[3], self.ps[3]])
        i.add_int("test stat int", 9)
        i.add_float("test stat float", 48.4)
        i.add_string("stat type", "MockRestraint")
        i.add_filename("test stat filename", "/foo")
        i.add_floats("test stat floats", [42., 199.5])
        i.add_ints("test stat ints", [42, 199])
        i.add_strings("test stat strings", ["bar2", "baz2"])
        i.add_filenames("test stat filenames", ["foo2", "bar2"])
        return i

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        if len(self.ps) >= 5:
            i.add_particle_indexes("dynamic particles",
                                   [self.ps[4]])
        i.add_int("test dyn int", 5)
        i.add_float("test dyn float", 42.4)
        i.add_string("dyn string", self._dyn_info)
        i.add_filename("test dyn filename", "/foobar")
        i.add_floats("test dyn floats", [42., 99.5])
        i.add_ints("test dyn ints", [42, 99])
        i.add_strings("test dyn strings", ["bar", "baz"])
        i.add_filenames("test dyn filenames", ["foo", "bar"])
        return i


class MockRestraintSet(IMP.RestraintSet):

    def __init__(self, m, ps, val):
        IMP.RestraintSet.__init__(self, m, "MockRestraintSet %1%")

    def get_dynamic_info(self):
        i = IMP.RestraintInfo()
        i.add_int("test int", 99)
        return i


class Tests(IMP.test.TestCase):

    def _write_restraint(self, name, cls=IMP._ConstRestraint, extra_ps=0,
                         add_extra_mock_restraint_frame=False):
        f = RMF.create_rmf_file(name)
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.rmf.add_particles(f, [p])
        ps = [p]
        # extra_ps are particles referenced by the restraint but not
        # explicitly added to the RMF
        if extra_ps:
            p1 = IMP.Particle(m, "extra0")
            IMP.atom.Mass.setup_particle(p1, 42.0)
            p2 = IMP.Particle(m, "extra1")
            IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(1,2,3))
            IMP.core.XYZR.setup_particle(p2, 4)
            p3 = IMP.Particle(m, "extra2")
            g = IMP.core.Gaussian.setup_particle(p3)
            g.set_variances([106.783, 55.2361, 20.0973])
            rot = IMP.algebra.Rotation3D([0.00799897, 0.408664,
                                          -0.845514, -0.343563])
            tran = IMP.algebra.Vector3D(101.442, 157.066, 145.694)
            tr = IMP.algebra.Transformation3D(rot, tran)
            g.set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
            p4 = IMP.Particle(m, "extra3")
            scale = IMP.isd.Scale.setup_particle(p4, 1.0)
            scale.set_lower(0.0)
            scale.set_upper(10.0)
            ps.extend([p1, p2, p3, p4])
        r = cls(m, ps, 1)
        r.evaluate(False)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, str(0))
        if extra_ps:
            scale.set_scale(0.5)
            IMP.rmf.save_frame(f, str(1))
        elif add_extra_mock_restraint_frame:
            r._dyn_info = "bar"
            IMP.rmf.save_frame(f, str(1))

    def _read_restraint(self, name, extra_ps=0, inputs=True):
        IMP.add_to_log(IMP.TERSE, "Starting reading back\n")
        f = RMF.open_rmf_file_read_only(name)
        m = IMP.Model()
        ps = IMP.rmf.create_particles(f, m)
        self.assertEqual(len(ps), 1)
        r = IMP.rmf.create_restraints(f, m)[0]
        IMP.rmf.load_frame(f, RMF.FrameID(0))
        print([IMP.Particle.get_from(x).get_index() for x in r.get_inputs()])
        print([x.get_index() for x in ps])
        if inputs:
            self.assertEqual(len(r.get_inputs()), 1 + extra_ps)
        return m, r, f

    def test_0(self):
        """Test writing restraints rmf"""
        RMF.set_log_level("Off")
        for suffix in IMP.rmf.suffixes:
            name = self.get_tmp_file_name("restr" + suffix)
            print("#### write")
            self._write_restraint(name)
            print("#### read")
            self._read_restraint(name)

    def test_1(self):
        for suffix in IMP.rmf.suffixes:
            """Test writing restraints to rmf with no particles"""
            nm = self.get_tmp_file_name("restrnp" + suffix)
            print(nm)
            f = RMF.create_rmf_file(nm)
            m = IMP.Model()
            p = IMP.Particle(m)
            r = IMP._ConstRestraint(m, [], 1)
            r.set_name("R")
            r.evaluate(False)
            IMP.rmf.add_restraint(f, r)
            IMP.rmf.save_frame(f, str(0))
            del f
            print("#### opening")
            f = RMF.open_rmf_file_read_only(nm)
            rr = IMP.rmf.create_restraints(f, m)
            IMP.rmf.load_frame(f, RMF.FrameID(0))
            self.assertEqual(rr[0].evaluate(False), r.evaluate(False))

    def test_2(self):
        """Test writing dynamic restraints"""
        RMF.set_log_level("Off")
        for suffix in IMP.rmf.suffixes:
            RMF.HDF5.set_show_errors(True)
            path = self.get_tmp_file_name("dynamic_restraints" + suffix)
            print(path)
            f = RMF.create_rmf_file(path)
            IMP.set_log_level(IMP.SILENT)
            m = IMP.Model()
            ps = [IMP.Particle(m) for i in range(0, 10)]
            ds = [IMP.core.XYZR.setup_particle(p) for p in ps]
            for d in ds:
                d.set_radius(1)
            IMP.rmf.add_particles(f, ds)
            cpc = IMP.container.ClosePairContainer(ps, 0)
            r = IMP.container.PairsRestraint(
                IMP.core.SoftSpherePairScore(1),
                cpc, "PR")
            bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                           IMP.algebra.Vector3D(10, 10, 10))
            r.evaluate(False)
            IMP.rmf.add_restraint(f, r)
            scores = []
            for i in range(0, 10):
                for d in ds:
                    d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
                scores.append(r.evaluate(False))
                IMP.rmf.save_frame(f, str(i))
                RMF.show_hierarchy_with_values(f.get_root_node())
            for i, d in enumerate(ds):
                d.set_x(i * 10)
            scores.append(r.evaluate(False))
            IMP.rmf.save_frame(f, str(10))

            del f
            del r
            f = RMF.open_rmf_file_read_only(path)
            bps = IMP.rmf.create_particles(f, m)
            rr = IMP.rmf.create_restraints(f, m)
            print(scores)
            print(rr[0].get_name(), rr[0].get_type_name())
            print(path)
            for i in range(0, 11):
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print(i, scores[i], rr[0].evaluate(False))
                self.assertAlmostEqual(
                    scores[i],
                    rr[0].evaluate(False),
                    delta=.01)

    def test_3(self):
        """Test that simple pair restaints don't generate subnodes"""
        m = IMP.Model()

        def _cp(m, n):
            pi = m.add_particle(n)
            IMP.atom.Mass.setup_particle(m, pi, 1)
            IMP.core.XYZR.setup_particle(
                m,
                pi,
                IMP.algebra.get_unit_sphere_3d())
            IMP.atom.Hierarchy.setup_particle(m, pi)
            return pi
        pi0 = _cp(m, "p0")
        pi1 = _cp(m, "p1")
        ps = IMP.core.SoftSpherePairScore(1)
        path = self.get_tmp_file_name("pair_restraint.rmf3")
        print(path)
        rh = RMF.create_rmf_file(path)
        particles = [m.get_particle(pi0), m.get_particle(pi1)]
        IMP.rmf.add_hierarchies(rh, particles)
        r = IMP.core.PairRestraint(m, ps, particles)
        IMP.rmf.add_restraint(rh, r)
        IMP.rmf.save_frame(rh, "frame")
        rn = rh.get_root_node().get_children()[2]
        RMF.show_hierarchy(rh.get_root_node())
        self.assertEqual(rn.get_name(), r.get_name())
        self.assertEqual([x for x in rn.get_children()], [])

    def test_static_info(self):
        """Test static restraint info"""
        for suffix in IMP.rmf.suffixes:
            name = self.get_tmp_file_name("static_info" + suffix)
            self._write_restraint(name, cls=MockRestraint)
            m, r, rmf = self._read_restraint(name)
            info = r.get_static_info()
            self.assertEqual(info.get_number_of_int(), 1)
            self.assertEqual(info.get_int_key(0), "test stat int")
            self.assertEqual(info.get_int_value(0), 9)
            self.assertEqual(info.get_number_of_float(), 1)
            self.assertEqual(info.get_float_key(0), "test stat float")
            self.assertAlmostEqual(info.get_float_value(0), 48.4, delta=0.01)
            self.assertEqual(info.get_number_of_string(), 1)
            self.assertEqual(info.get_string_key(0), "stat type")
            self.assertEqual(info.get_string_value(0), "MockRestraint")
            self.assertEqual(info.get_number_of_filename(), 1)
            self.assertEqual(info.get_filename_key(0), "test stat filename")
            self.assertEqual(info.get_filename_value(0), "/foo")
            self.assertEqual(info.get_number_of_floats(), 1)
            self.assertEqual(info.get_floats_key(0), "test stat floats")
            val = info.get_floats_value(0)
            self.assertEqual(len(val), 2)
            self.assertAlmostEqual(val[0], 42., delta=1e-6)
            self.assertAlmostEqual(val[1], 199.5, delta=1e-6)
            self.assertEqual(info.get_number_of_ints(), 1)
            self.assertEqual(info.get_ints_key(0), "test stat ints")
            self.assertEqual(list(info.get_ints_value(0)), [42, 199])
            self.assertEqual(info.get_number_of_strings(), 1)
            self.assertEqual(info.get_strings_key(0), "test stat strings")
            self.assertEqual(info.get_strings_value(0), ["bar2", "baz2"])
            self.assertEqual(info.get_number_of_filenames(), 1)
            self.assertEqual(info.get_filenames_key(0), "test stat filenames")
            self.assertEqual(len(info.get_filenames_value(0)), 2)

    def test_dynamic_info(self):
        """Test dynamic restraint info"""
        for suffix in IMP.rmf.suffixes:
            name = self.get_tmp_file_name("dynamic_info" + suffix)
            self._write_restraint(name, cls=MockRestraint,
                                  add_extra_mock_restraint_frame=True)
            m, r, rmf = self._read_restraint(name)
            info = r.get_dynamic_info()
            self.assertEqual(info.get_number_of_int(), 1)
            self.assertEqual(info.get_int_key(0), "test dyn int")
            self.assertEqual(info.get_int_value(0), 5)
            self.assertEqual(info.get_number_of_float(), 1)
            self.assertEqual(info.get_float_key(0), "test dyn float")
            self.assertAlmostEqual(info.get_float_value(0), 42.4, delta=0.01)
            self.assertEqual(info.get_number_of_string(), 1)
            self.assertEqual(info.get_string_key(0), "dyn string")
            self.assertEqual(info.get_string_value(0), "foo")
            self.assertEqual(info.get_number_of_filename(), 1)
            self.assertEqual(info.get_filename_key(0), "test dyn filename")
            self.assertEqual(info.get_filename_value(0), "/foobar")
            self.assertEqual(info.get_number_of_floats(), 1)
            self.assertEqual(info.get_floats_key(0), "test dyn floats")
            val = info.get_floats_value(0)
            self.assertEqual(len(val), 2)
            self.assertAlmostEqual(val[0], 42., delta=1e-6)
            self.assertAlmostEqual(val[1], 99.5, delta=1e-6)
            self.assertEqual(info.get_number_of_ints(), 1)
            self.assertEqual(info.get_ints_key(0), "test dyn ints")
            self.assertEqual(list(info.get_ints_value(0)), [42, 99])
            self.assertEqual(info.get_number_of_strings(), 1)
            self.assertEqual(info.get_strings_key(0), "test dyn strings")
            self.assertEqual(info.get_strings_value(0), ["bar", "baz"])
            self.assertEqual(info.get_number_of_filenames(), 1)
            self.assertEqual(info.get_filenames_key(0), "test dyn filenames")
            self.assertEqual(len(info.get_filenames_value(0)), 2)
            # Test per-frame values
            IMP.rmf.load_frame(rmf, RMF.FrameID(1))
            info = r.get_dynamic_info()
            self.assertEqual(info.get_number_of_string(), 1)
            self.assertEqual(info.get_string_key(0), "dyn string")
            self.assertEqual(info.get_string_value(0), "bar")

    def test_dynamic_info_restraint_set(self):
        """Test dynamic RestraintSet info"""
        for suffix in IMP.rmf.suffixes:
            name = self.get_tmp_file_name("dynamic_info_set" + suffix)
            self._write_restraint(name, cls=MockRestraintSet)
            m, r, rmf = self._read_restraint(name, inputs=False)
            info = r.get_dynamic_info()
            self.assertEqual(info.get_number_of_int(), 1)
            self.assertEqual(info.get_int_key(0), "test int")
            self.assertEqual(info.get_int_value(0), 99)

    def test_associated_particles(self):
        """Test handling of restraint associated particles"""
        for suffix in IMP.rmf.suffixes:
            name = self.get_tmp_file_name("assoc_ps" + suffix)
            self._write_restraint(name, cls=MockRestraint, extra_ps=4)
            m, r, rmf = self._read_restraint(name, extra_ps=4)
            # Dynamic particle indexes are not currently supported
            info = r.get_dynamic_info()
            self.assertEqual(info.get_number_of_particle_indexes(), 0)
            info = r.get_static_info()
            # Currently both "static" and "dynamic" particles show up
            # in static info
            self.assertEqual(info.get_number_of_particle_indexes(), 2)
            self.assertEqual(info.get_particle_indexes_key(0),
                             'static particles')
            self.assertEqual([m.get_particle_name(i)
                              for i in info.get_particle_indexes_value(0)],
                             ['extra0', 'extra1', 'extra2', 'extra2'])
            ind0, ind1, ind2, ind3 = info.get_particle_indexes_value(0)
            # last particle (ind3) should be an alias to ind2
            self.assertEqual(ind2, ind3)
            self.assertEqual(info.get_particle_indexes_key(1),
                             'dynamic particles')
            self.assertEqual([m.get_particle_name(i)
                              for i in info.get_particle_indexes_value(1)],
                             ['extra3'])
            ind3, = info.get_particle_indexes_value(1)
            # Make sure that particle attributes survive a round trip
            self.assertTrue(IMP.atom.Mass.get_is_setup(m, ind0))
            self.assertAlmostEqual(IMP.atom.Mass(m, ind0).get_mass(), 42.0,
                                   delta=1e-6)
            self.assertTrue(IMP.core.XYZR.get_is_setup(m, ind1))
            self.assertAlmostEqual(IMP.core.XYZR(m, ind1).get_radius(), 4.0,
                                   delta=1e-6)
            self.assertLess(IMP.algebra.get_distance(
                IMP.core.XYZR(m, ind1).get_coordinates(),
                IMP.algebra.Vector3D(1,2,3)), 1e-4)
            self.assertTrue(IMP.core.Gaussian.get_is_setup(m, ind2))
            g = IMP.core.Gaussian(m, ind2)
            self.assertLess(IMP.algebra.get_distance(
                g.get_variances(), [106.783, 55.2361, 20.0973]), 1e-4)
            tr = g.get_reference_frame().get_transformation_to()
            q = tr.get_rotation().get_quaternion()
            trans = tr.get_translation()
            self.assertLess(IMP.algebra.get_distance(
                q, [0.00799897, 0.408664, -0.845514, -0.343563]), 1e-4)
            self.assertLess(IMP.algebra.get_distance(
                trans, [101.442, 157.066, 145.694]), 1e-4)
            self.assertTrue(IMP.isd.Scale.get_is_setup(m, ind3))
            self.assertAlmostEqual(IMP.isd.Scale(m, ind3).get_scale(), 1.0,
                                   delta=1e-6)
            self.assertAlmostEqual(IMP.isd.Scale(m, ind3).get_lower(), 0.0,
                                   delta=1e-6)
            self.assertAlmostEqual(IMP.isd.Scale(m, ind3).get_upper(), 10.0,
                                   delta=1e-6)

            # Test per-frame values
            IMP.rmf.load_frame(rmf, RMF.FrameID(1))
            self.assertAlmostEqual(IMP.isd.Scale(m, ind3).get_scale(), 0.5,
                                   delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
