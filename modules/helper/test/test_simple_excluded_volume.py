import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper
import math

class SimpleExclusionVolumeTests(IMP.test.TestCase):
    """Tests for simple exclusion volume"""
    def test_ev(self):
        """Testing excluded volume restraint"""
        m= IMP.Model()
        print "read"
        sel = IMP.atom.CAlphaSelector()
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m, sel)
        p1= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m, sel)
        print 'create'
        mhs = IMP.atom.Hierarchies()
        mhs.append(p0)
        mhs.append(p1)
        ps = IMP.Particles()
        for mh in mhs:
            ps.append(mh.get_particle())

        rbs = IMP.helper.set_rigid_bodies(mhs)

        print "radius"
        for p in IMP.core.get_leaves(p0)+IMP.core.get_leaves(p1):
            d= IMP.core.XYZR.setup_particle(p.get_particle())
            d.set_radius(1)
        print "add ss"
        sc= IMP.core.ListSingletonContainer()
        fps=IMP.core.XYZRs()
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.setup_particle(p)
            d.set_radius(10)
            sc.add_particle(p)
            fps.append(d)
            d.set_coordinates_are_optimized(True)
        for p in ps:
            d= IMP.core.XYZ(p)
            d.set_coordinates(IMP.algebra.random_vector_in_box(
                              IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(20,20,20)))

        sev = IMP.helper.create_simple_excluded_volume_on_rigid_bodies(rbs)
        test = IMP.helper.create_simple_excluded_volume_on_molecules(mhs)

        r = sev.get_restraint()
        r_test = test.get_restraint()

        r.set_log_level(IMP.SILENT)

        print "mc"
        o= IMP.core.MonteCarlo()

        # very dumb op
        sc.add_particles(ps)
        bm= IMP.core.BallMover(sc, IMP.core.XYZ.get_xyz_keys(), 100)
        o.add_mover(bm)
        o.set_model(m)
        IMP.set_log_level(IMP.VERBOSE)
        print "opt"
        o.optimize(10)
        print "inspect"
        pas = fps+ IMP.core.XYZRs(IMP.core.get_leaves(p1))
        pbs = fps+ IMP.core.XYZRs(IMP.core.get_leaves(p0))
        for pa in pas:
            for pb in pbs:
                if pa == pb: continue
                else:
                    #print pa
                    #print pb
                    d= IMP.core.distance(pa, pb)
                    self.assert_(d > -.1)

if __name__ == '__main__':
    unittest.main()
