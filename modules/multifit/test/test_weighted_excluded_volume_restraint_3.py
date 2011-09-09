import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.restrainer
import os
import time

class SampleTests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def _setup(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        #init IMP model ( the environment)
        m = IMP.Model()
        h0=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                              m)
        h1=IMP.atom.read_pdb(self.get_input_file_name("1z5s_C1.pdb"),
                              m)
        rb0 = IMP.atom.create_rigid_body(h0)
        rb1 = IMP.atom.create_rigid_body(h1)
        rb0.set_reference_frame(IMP.algebra.ReferenceFrame3D())
        rb1.set_reference_frame(IMP.algebra.ReferenceFrame3D())
        l0= IMP.atom.get_leaves(h0)
        l1= IMP.atom.get_leaves(h1)
        #set the restraint
        r=IMP.multifit.WeightedExcludedVolumeRestraint3(l0, l1)
        r.set_maximum_separation(10)
        r.set_complementarity_value(-10)
        r.set_maximum_penetration_score(30)
        r.set_interior_layer_thickness(5)
        r.set_complementarity_thickness(5)

        m.add_restraint(r)
        return (m, rb0, rb1, l0, l1)
    def test_weighted_excluded_volume_restraint(self):
        """Check that weighted excluded volume restraint 3 works"""
        (m, rb0, rb1, l0, l1)= self._setup()
        rb1.set_coordinates_are_optimized(True)
        IMP.set_log_level(IMP.TERSE)#VERBOSE)
        #mc= IMP.core.MonteCarlo(m)
        #mv = IMP.core.RigidBodyMover(rb1, 500, .2)
        #mv.set_log_level(IMP.VERBOSE)
        #mc.add_mover(mv)
        #mc.optimize(100)
        for i in range(48,58):
            tr= IMP.algebra.Transformation3D(IMP.algebra.Vector3D(i,0,0))
            nrf=IMP.algebra.ReferenceFrame3D(tr)
            rb0.set_reference_frame(nrf)
            s=m.evaluate(False)
            print "score is", s
            IMP.atom.write_pdb(l0, "a"+str(i)+".pdb")
            IMP.atom.write_pdb(l1, "b"+str(i)+".pdb")
            if s < 0:
                break
        print "Final score", m.evaluate(False)
        lsc= IMP.container.ListSingletonContainer(l0+ l1)
        evr= IMP.core.ExcludedVolumeRestraint(lsc, 1)
        IMP.set_check_level(IMP.USAGE)
        evr.set_model(m)
        self.assertEqual(evr.evaluate(False), 0)
        mr= IMP.core.RigidMembersRefiner()
        hub= IMP.core.HarmonicUpperBoundSphereDistancePairScore(6, 1)
        kcp= IMP.core.KClosePairsPairScore(hub, mr)
        self.assertEqual(kcp.evaluate((rb0, rb1), None), 0)
if __name__ == '__main__':
    IMP.test.main()
