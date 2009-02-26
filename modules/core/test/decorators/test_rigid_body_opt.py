import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.gsl
import IMP.display
import IMP.misc



class WLCTests(IMP.test.TestCase):
    """Tests for WLC unary function"""

    def _create_log(self, name, xyzs, color):
        w= IMP.display.BildWriter()
        l= IMP.display.LogOptimizerState(w, self.get_tmp_file_name(name))
        for p in xyzs:
            sg=IMP.display.XYZRGeometry(p)
            if p==xyzs[0] or p==xyzs[1] or p==xyzs[2]:
                sg.set_color(IMP.display.Color(0,0,1))
            else:
                sg.set_color(color);
            l.add_geometry(sg)
        l.set_skip_steps(10)
        return l

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
                                               IMP.algebra.random_vector_in_unit_box(), 0.01)
            chd= IMP.core.HierarchyDecorator.create(mp)
            hd.add_child(chd)
            #IMP.algebra.Vector3D(i%2, (i+1)%3, i)
            xyzs.append(mxyz)
            mxyz.show()
            print
        pr= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        rbd= IMP.core.RigidBodyDecorator.create(rbp, pr, tr)
        w= IMP.display.BildWriter()
        rbd.set_transformation(IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(1,0,0,0),
                                                            IMP.algebra.Vector3D(0,0,0)))
        um= IMP.core.UpdateRigidBodyMembers(pr, tr)
        um.apply(rbd.get_particle())
        um.thisown=False
        w.set_file_name(self.get_tmp_file_name('rigidbody.bild'))
        for p in xyzs:
            rm= IMP.core.RigidMemberDecorator(p.get_particle(), tr)
            v= rm.get_internal_coordinates()
            self.assertInTolerance((rm.get_internal_coordinates()-rm.get_coordinates()).get_magnitude(), 0, .1)
            sg=IMP.display.SphereGeometry(v,.01)
            sg.set_color(IMP.display.Color(1,0,0));
            w.add_geometry(sg)
        sg=IMP.display.SphereGeometry(IMP.algebra.Vector3D(0,0,0),.01)
        sg.set_color(IMP.display.Color(1,1,0));
        w.add_geometry(sg)
        w.set_file_name("")

        sa= IMP.misc.StateAdaptor(self._create_log("before.%04d.bild", xyzs, IMP.display.Color(1,0,0)),
                                  self._create_log("after.%04d.bild", xyzs, IMP.display.Color(0,1,0)))
        m.add_score_state(sa)
        return (m, rbd, pr, xyzs)

    def _create_restraints(self, m, xyzs, traits):
        r= IMP.algebra.random_rotation()
        q= r.get_quaternion()
        q[0]= q[0]*10
        q= q.get_unit_vector()
        r= IMP.algebra.Rotation3D(q[0], q[1], q[2], q[3])
        #t= IMP.algebra.random_vector_in_unit_box()
        t= IMP.algebra.Vector3D(1,0,0)
        tr= IMP.algebra.Transformation3D(r, t)
        targets=[]
        for i in range(0,len(xyzs)):
            md= IMP.core.RigidMemberDecorator(xyzs[i].get_particle(), traits)
            iv= md.get_internal_coordinates()
            t=tr.transform(iv)
            dt= IMP.core.DistanceToSingletonScore(IMP.core.Harmonic(0,1),
                                                  t)
            targets.append(t)
            sr= IMP.core.SingletonRestraint(dt, xyzs[i].get_particle())
            m.add_restraint(sr)

        w= IMP.display.BildWriter()
        w.set_file_name(self.get_tmp_file_name('targets.bild'))
        for t in targets:
            sg=IMP.display.SphereGeometry(t,.01)
            sg.set_color(IMP.display.Color(.5,.5,.5));
            w.add_geometry(sg)
        return (tr, targets)

    def _write_state(self, name, rbd, pr, xyzs, targets):
        w= IMP.display.BildWriter()
        w.set_file_name(self.get_tmp_file_name(name))
        r=.01
        #c= IMP.algebra.Vector3D(.8, .2, .2)
        sg=IMP.display.SphereGeometry(targets[0],r)
        sg.set_color(IMP.display.Color(.9, .2, .2))
        w.add_geometry(sg)
        sg=IMP.display.SphereGeometry(targets[1],r)
        sg.set_color(IMP.display.Color(.7*.9, .2, .2))
        w.add_geometry(sg)
        sg=IMP.display.SphereGeometry(targets[2],r)
        sg.set_color(IMP.display.Color(.4*.9, .2, .2))
        w.add_geometry(sg)
        ge= IMP.display.RigidBodyDerivativeGeometry(rbd, pr)
        w.add_geometry(ge)
        for i in range(0,len(xyzs)):
            g=IMP.display.XYZRGeometry(xyzs[i])
            if i==0:
                g.set_color(IMP.display.Color(.2, .8, .2))
            elif i==1:
                g.set_color(IMP.display.Color(.2, .7*.8, .2))
            elif i==2:
                g.set_color(IMP.display.Color(.2, .4*.8, .2))
            w.add_geometry(g)
        w.set_file_name("")

    def _check_optimization(self, m, xyzs, rbd, pr, tr, targets, steps):
        cg= IMP.core.SteepestDescent()
        cg.set_model(m)
        l= IMP.display.LogOptimizerState(IMP.display.BildWriter(),
                                         self.get_tmp_file_name("deriv.%04d.bild"))
        l.add_geometry(IMP.display.RigidBodyDerivativeGeometry(rbd, pr))
        cg.add_optimizer_state(l)
        m.evaluate(True)
        l.update()

        #rbd.show()
        print "evaluating"
        #m.evaluate(True)
        #rbd.show()
        #cg.optimize(1)
        #m.evaluate(True)
        #rbd.show()
        #w= IMP.display.BildWriter()
        #l= IMP.display.LogOptimizerState(w, "opt_state.%03d.bild")
        for p in xyzs:
            p.show(); print
        IMP.set_log_level(IMP.VERBOSE)
        print "check move"
        cg.optimize(10)
        self._write_state("final_config.bild", rbd, pr, xyzs, targets)
        #m.evaluate(False)
        #print cg.optimize(1)
        #self._write_state("real_config.bild", rbd, pr, xyzs, targets)
        for i in range(0,len(xyzs)):
            md= xyzs[i]
            tx= targets[i]
            mc= md.get_coordinates()
            mc.show(); print ": ",
            tx.show(); print ": ",
            md.get_derivatives().show(); print
            self.assertInTolerance((mc-tx).get_squared_magnitude(),
                                   0, .1)
        return

        IMP.set_log_level(IMP.SILENT)
        cg.set_threshold(0)
        print "optimizing"
        for i in range(0, 10):
            cg.optimize(steps/10)
            print m.evaluate(False)
        IMP.set_log_level(IMP.VERBOSE)
        print "testing"

        print "Transforms are "
        tr.show(); print
        rbd.get_transformation().show(); print
        print "Energy is " + str(m.evaluate(False))

        print "done"

    def _test_snapped(self):
        """Test rigid body optimization with snapping"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        for d in xyzs:
            d.set_coordinates_are_optimized(True)
        rbd.set_coordinates_are_optimized(False)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        (tr, targets)= self._create_restraints(m, xyzs, rbd.get_traits())
        IMP.core.setup_rigid_body(m, rbd.get_particle(), pr, rbd.get_traits(),
                         True)
        self._check_optimization(m, xyzs, rbd, pr, tr, targets, 100)

    def test_optimized(self):
        """Test rigid body direct optimization"""
        (m, rbd, pr, xyzs)= self._create_rigid_body()
        for d in xyzs:
            d.set_coordinates_are_optimized(False)
        rbd.set_coordinates_are_optimized(True)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        (tr, targets)= self._create_restraints(m, xyzs, rbd.get_traits())
        IMP.core.setup_rigid_body(m, rbd.get_particle(), pr, rbd.get_traits(),
                         False)
        rbd.show()
        self._check_optimization(m, xyzs, rbd, pr, tr, targets, 100)

if __name__ == '__main__':
    unittest.main()
