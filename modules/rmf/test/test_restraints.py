import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *


class Tests(IMP.test.TestCase):

    def _write_restraint(self, name):
        f = RMF.create_rmf_file(name)
        m = IMP.kernel.Model()
        p = IMP.kernel.Particle(m)
        IMP.rmf.add_particles(f, [p])
        r = IMP.kernel._ConstRestraint(1, [p])
        r.evaluate(False)
        IMP.rmf.add_restraint(f, r)
        IMP.rmf.save_frame(f, str(0))

    def _read_restraint(self, name):
        IMP.base.add_to_log(IMP.base.TERSE, "Starting reading back\n")
        f = RMF.open_rmf_file_read_only(name)
        m = IMP.kernel.Model()
        ps = IMP.rmf.create_particles(f, m)
        r = IMP.rmf.create_restraints(f, m)[0]
        IMP.rmf.load_frame(f, RMF.FrameID(0))
        print [IMP.kernel.Particle.get_from(x).get_index() for x in r.get_inputs()]
        print [x.get_index() for x in ps]
        self.assertEqual(r.get_inputs(), ps)

    def test_0(self):
        """Test writing restraints rmf"""
        RMF.set_log_level("Off")
        for suffix in IMP.rmf.suffixes:
            name = self.get_tmp_file_name("restr." + suffix)
            print "#### write"
            self._write_restraint(name)
            print "#### read"
            self._read_restraint(name)

    def test_1(self):
        for suffix in IMP.rmf.suffixes:
            """Test writing restraints to rmf with no particles"""
            nm = self.get_tmp_file_name("restrnp." + suffix)
            print nm
            f = RMF.create_rmf_file(nm)
            m = IMP.kernel.Model()
            p = IMP.kernel.Particle(m)
            r = IMP.kernel._ConstRestraint(m, [], 1)
            r.set_name("R")
            r.evaluate(False)
            IMP.rmf.add_restraint(f, r)
            IMP.rmf.save_frame(f, str(0))
            del f
            print "#### opening"
            f = RMF.open_rmf_file_read_only(nm)
            rr = IMP.rmf.create_restraints(f, m)
            IMP.rmf.load_frame(f, RMF.FrameID(0))
            self.assertEqual(rr[0].evaluate(False), r.evaluate(False))

    def test_2(self):
        """Test writing dynamic restraints"""
        RMF.set_log_level("Off")
        for suffix in IMP.rmf.suffixes:
            RMF.HDF5.set_show_errors(True)
            path = self.get_tmp_file_name("dynamic_restraints." + suffix)
            print path
            f = RMF.create_rmf_file(path)
            IMP.base.set_log_level(IMP.base.SILENT)
            m = IMP.kernel.Model()
            ps = [IMP.kernel.Particle(m) for i in range(0, 10)]
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
            print scores
            print rr[0].get_name(), rr[0].get_type_name()
            print path
            for i in range(0, 11):
                IMP.rmf.load_frame(f, RMF.FrameID(i))
                print i, scores[i], rr[0].evaluate(False)
                self.assertAlmostEqual(
                    scores[i],
                    rr[0].evaluate(False),
                    delta=.01)

    def test_3(self):
        """Test that simple pair restaints don't generate subnodes"""
        m = IMP.kernel.Model()

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
        print path
        rh = RMF.create_rmf_file(path)
        particles = [m.get_particle(pi0), m.get_particle(pi1)]
        IMP.rmf.add_hierarchies(rh, particles)
        r = IMP.core.PairRestraint(ps, particles)
        IMP.rmf.add_restraint(rh, r)
        IMP.rmf.save_frame(rh, "frame")
        rn = rh.get_root_node().get_children()[2]
        RMF.show_hierarchy(rh.get_root_node())
        self.assertEqual(rn.get_name(), r.get_name())
        self.assertEqual(rn.get_children(), [])

if __name__ == '__main__':
    unittest.main()
