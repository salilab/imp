import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra



class WLCTests(IMP.test.TestCase):
    """Tests for WLC unary function"""

    def _create_rigid_body(self):
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        rbp= IMP.Particle(m)
        rbxyz= IMP.core.XYZDecorator.create(rbp)
        hd= IMP.core.HierarchyDecorator.create(rbp)
        xyzs= []
        tr= IMP.core.RigidBodyTraits("myrb")
        for i in range(0, 4):
            mp= IMP.Particle(m)
            mxyz= IMP.core.XYZDecorator.create(mp,
                                               IMP.algebra.random_vector_in_unit_box())
            chd= IMP.core.HierarchyDecorator.create(mp)
            hd.add_child(chd)
            #IMP.algebra.Vector3D(i%2, (i+1)%3, i)
            xyzs.append(mxyz)
            mxyz.show()
            print
        pr= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        rbd= IMP.core.RigidBodyDecorator.create(rbp, pr, tr)
        return (m, rbd, pr, xyzs)


    def test_rigid(self):
        """Test rigid bodies"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        rbd.show()
        #r= IMP.algebra.Rotation3D(0.437645, -0.422742, -0.778777, 0.152519)
        r=IMP.algebra.random_rotation()
        print "center rotated is "
        r.rotate(rbd.get_coordinates()).show()
        print
        print "transformed"
        tvs= IMP.algebra.Vector3Ds()
        for d in xyzs:
            d.set_coordinates(r.rotate(d.get_coordinates()))
            d.show()
            print
            tvs.append(d.get_coordinates())
        ud= IMP.core.UpdateRigidBodyOrientation(pr, rbd.get_traits())
        ud.apply(rbd.get_particle())
        print "final coordinates"
        rbd.show()
        print "at end"
        for i in range(0, len(xyzs)):
            d= xyzs[i]
            v=tvs[i]
            d.show()
            print
            self.assertInTolerance((d.get_coordinates()-v).get_squared_magnitude(),
                                   0, .1)

    def _create_restraints(self, m, xyzs, traits):
        r= IMP.algebra.random_rotation()
        t= IMP.algebra.random_vector_in_unit_box()
        tr= IMP.algebra.Transformation3D(r, t)
        for i in range(0,3):
            md= IMP.core.RigidMemberDecorator(xyzs[i].get_particle(), traits)
            iv= md.get_internal_coordinates()
            dt= IMP.core.DistanceToSingletonScore(IMP.core.Harmonic(0,1),
                                                  tr.transform(iv))
            sr= IMP.core.SingletonRestraint(dt, xyzs[i].get_particle())
            m.add_restraint(sr)
        return tr

    def _check_optimization(self, m, xyzs, tr, rbd, steps):
        cg= IMP.core.ConjugateGradients()
        cg.set_model(m)
        rbd.show()
        print "evaluating"
        m.evaluate(True)
        rbd.show()
        cg.optimize(1)
        rbd.show()
        IMP.set_log_level(IMP.SILENT)
        cg.optimize(steps)
        IMP.set_log_level(IMP.VERBOSE)
        print "testing"
        for p in xyzs:
            md= IMP.core.RigidMemberDecorator(p.get_particle(), rbd.get_traits())
            iv= md.get_internal_coordinates()
            tv= tr.transform(iv)
            cv= md.get_coordinates()
            print md
            print iv
            print tv
            print cv
            self.assertInTolerance((tv-cv).get_squared_magnitude(),
                                   0, .1)
        print "done"

    def _test_snapped(self):
        """Test rigid body optimization with snapping"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        for d in xyzs:
            d.set_coordinates_are_optimized(True)
        rbd.set_coordinates_are_optimized(False)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        tr= self._create_restraints(m, xyzs, rbd.get_traits())
        rbus= IMP.core.UpdateRigidBodyOrientation(pr, rbd.get_traits())
        sss= IMP.core.SingletonScoreState(rbus, None, rbd.get_particle())
        m.add_score_state(sss)
        self._check_optimization(m, xyzs, tr, rbd, 10000)

    def test_optimized(self):
        """Test rigid body direct optimization"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        for d in xyzs:
            d.set_coordinates_are_optimized(False)
        rbd.set_coordinates_are_optimized(True)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        tr= self._create_restraints(m, xyzs, rbd.get_traits())
        rbus= IMP.core.AccumulateRigidBodyDerivatives(pr, rbd.get_traits())
        rm= IMP.core.UpdateRigidBodyMembers(pr, rbd.get_traits())
        sss= IMP.core.SingletonScoreState(rm, rbus, rbd.get_particle())
        m.add_score_state(sss)
        rbd.show()
        self._check_optimization(m, xyzs, tr, rbd, 100)

if __name__ == '__main__':
    unittest.main()
