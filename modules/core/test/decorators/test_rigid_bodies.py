import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.display



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
        for i in range(0, 10):
            mp= IMP.Particle(m)
            mxyz= IMP.core.XYZRDecorator.create(mp,
                                               IMP.algebra.random_vector_in_unit_box(), 0.1)
            chd= IMP.core.HierarchyDecorator.create(mp)
            hd.add_child(chd)
            #IMP.algebra.Vector3D(i%2, (i+1)%3, i)
            xyzs.append(mxyz)
            mxyz.show()
            print
        pr= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        rbd= IMP.core.RigidBodyDecorator.create(rbp, pr, tr)
        return (m, rbd, pr, xyzs)


    def _test_rigid(self):
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
        targets=[]
        for d in xyzs:
            md= IMP.core.RigidMemberDecorator(d.get_particle(), traits)
            iv= md.get_internal_coordinates()
            targets.append(tr.transform(iv))
        return (tr, targets)

    def _write_state(self, name, xyzs, targets):
        w= IMP.display.BildWriter()
        w.set_file_name(name)
        r=.1
        #c= IMP.algebra.Vector3D(.8, .2, .2)
        w.add_geometry(IMP.display.SphereGeometry(targets[0],r,
                                                  IMP.algebra.Vector3D(.9, .2, .2)))
        w.add_geometry(IMP.display.SphereGeometry(targets[1],r,
                                                  IMP.algebra.Vector3D(.7*.9, .2, .2)))
        w.add_geometry(IMP.display.SphereGeometry(targets[2],r,
                                                  IMP.algebra.Vector3D(.4*.9, .2, .2)))
        for i in range(0,len(xyzs)):
            g=IMP.display.XYZRGeometry(xyzs[i])
            if i==0:
                g.set_color(.2, .8, .2)
            elif i==1:
                g.set_color(.2, .7*.8, .2)
            elif i==2:
                g.set_color(.2, .4*.8, .2)
            w.add_geometry(g)
        w.set_file_name("")

    def _check_optimization(self, m, xyzs, rbd, targets, steps):
        cg= IMP.core.SteepestDescent()
        cg.set_model(m)
        #rbd.show()
        print "evaluating"
        #m.evaluate(True)
        #rbd.show()
        #cg.optimize(1)
        #m.evaluate(True)
        #rbd.show()
        #w= IMP.display.BildWriter()
        #l= IMP.display.LogOptimizerState(w, "opt_state.%03d.bild")
        IMP.set_log_level(IMP.SILENT)
        cg.set_threshold(0)
        cg.optimize(steps)
        IMP.set_log_level(IMP.VERBOSE)
        print "testing"
        self._write_state("final_config.bild", xyzs, targets)
        #m.evaluate(False)
        cg.optimize(1)
        self._write_state("real_config.bild", xyzs, targets)
        for i in range(0,len(xyzs)):
            md= xyzs[i]
            tx= targets[i]
            mc= md.get_coordinates()
            mc.show(); print
            tx.show(); print
            self.assertInTolerance((mc-tx).get_squared_magnitude(),
                                   0, .1)
        print "done"

    def test_snapped(self):
        """Test rigid body optimization with snapping"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        for d in xyzs:
            d.set_coordinates_are_optimized(True)
        rbd.set_coordinates_are_optimized(False)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        (tr, targets)= self._create_restraints(m, xyzs, rbd.get_traits())
        rbus= IMP.core.UpdateRigidBodyOrientation(pr, rbd.get_traits())
        sss= IMP.core.SingletonScoreState(rbus, None, rbd.get_particle())
        m.add_score_state(sss)
        self._check_optimization(m, xyzs, rbd, targets, 1000)

    def _test_optimized(self):
        """Test rigid body direct optimization"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        for d in xyzs:
            d.set_coordinates_are_optimized(False)
        rbd.set_coordinates_are_optimized(True)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        (tr, targets)= self._create_restraints(m, xyzs, rbd.get_traits())
        rbus= IMP.core.AccumulateRigidBodyDerivatives(pr, rbd.get_traits())
        rm= IMP.core.UpdateRigidBodyMembers(pr, rbd.get_traits())
        sss= IMP.core.SingletonScoreState(rm, rbus, rbd.get_particle())
        m.add_score_state(sss)
        rbd.show()
        self._check_optimization(m, xyzs, rbd, targets, 10000)

if __name__ == '__main__':
    unittest.main()
