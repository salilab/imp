import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import math

class AngleRestraintTests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def test_ev(self):
        """Testing excluded volume restraint"""
        m= IMP.Model()
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        p1= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        r0s= IMP.core.create_rigid_body(p0.get_particle(),
                                        IMP.core.get_leaves(p0))
        r1s= IMP.core.create_rigid_body(p1.get_particle(),
                                        IMP.core.get_leaves(p1))
        for p in IMP.core.get_leaves(p0)+IMP.core.get_leaves(p1):
            d= IMP.core.XYZR.create(p)
            d.set_radius(1)
        m.add_score_state(r0s)
        m.add_score_state(r1s)
        rb0= IMP.core.RigidBody(p0.get_particle())
        rb1= IMP.core.RigidBody(p1.get_particle())
        rb0.set_coordinates_are_optimized(True)
        rb1.set_coordinates_are_optimized(True)
        sc= IMP.core.ListSingletonContainer()
        fps=[]
        for i in range(0,10):
            p= IMP.Particle(m)
            d= IMP.core.XYZR.create(p)
            d.set_radius(10)
            sc.add_particle(p)
            fps.append(p)
            d.set_coordinates_are_optimized(True)
        for p in sc.get_particles():
            d= IMP.core.XYZ(p)
            d.set_coordinates(IMP.algebra.random_vector_in_box(IMP.algebra.Vector3D(0,0,0),
                                                               IMP.algebra.Vector3D(20,20,20)))
        sc.add_particle(rb0.get_particle())
        sc.add_particle(rb1.get_particle())
        r= IMP.core.ExcludedVolumeRestraint(sc)
        r.set_log_level(IMP.SILENT)
        m.add_restraint(r)
        o= IMP.core.MonteCarlo()
        # very dumb op
        bm= IMP.core.BallMover(sc, IMP.core.XYZ.get_xyz_keys(), 100)
        o.add_mover(bm)
        o.set_model(m)
        IMP.set_log_level(IMP.VERBOSE)
        o.optimize(10)
        for pa in fps+ list(IMP.core.get_leaves(p1)):
            for pb in fps+ list(IMP.core.get_leaves(p0)):
                if pa == pb: continue
                else:
                    r0 = IMP.core.XYZR(pa)
                    r1 = IMP.core.XYZR(pb)
                    d= IMP.core.distance(r0, r1)
                    self.assert_(d > -.1)

if __name__ == '__main__':
    unittest.main()
