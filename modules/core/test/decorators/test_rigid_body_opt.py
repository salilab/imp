import unittest
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.display
import IMP.misc



class WLCTests(IMP.test.TestCase):
    """Tests for WLC unary function"""

    def _setup(self):
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        return m
    def _create_children(self, m):
        xyzs= []
        tr= IMP.core.RigidBodyTraits("myrb")
        for i in range(0, 10):
            mp= IMP.Particle(m)
            mxyz= IMP.core.XYZRDecorator.create(mp,
                                               IMP.algebra.Sphere3D(IMP.algebra.random_vector_in_unit_box(), 0.01))
            chd= IMP.core.HierarchyDecorator.create(mp)
            xyzs.append(mxyz)
            mxyz.show()
            print
        return (xyzs, tr)

    def _create_hierarchy(self, m, xyzs):
        rbp= IMP.Particle(m)
        rbxyz= IMP.core.XYZDecorator.create(rbp)
        hd= IMP.core.HierarchyDecorator.create(rbp)
        for p in xyzs:
            hd.add_child(IMP.core.HierarchyDecorator(p.get_particle()))
        pr= IMP.core.ChildrenParticleRefiner(IMP.core.HierarchyDecorator.get_default_traits())
        return (rbp, pr)

    def _create_restraints(self, m, xyzs, traits):
        targets=[]
        for i in range(0,len(xyzs)):
            md= IMP.core.XYZDecorator(xyzs[i].get_particle())
            iv= md.get_coordinates()
            dt= IMP.core.DistanceToSingletonScore(IMP.core.Harmonic(0,1),
                                                  iv)
            targets.append(iv)
            sr= IMP.core.SingletonRestraint(dt, xyzs[i].get_particle())
            m.add_restraint(sr)
        return targets

    def _transform_particles(self, rbp, tr, xyzs):
        rbd=IMP.core.RigidBodyDecorator(rbp, tr)
        r= IMP.algebra.random_rotation()
        q= r.get_quaternion()
        q[0]= q[0]*10
        q= q.get_unit_vector()
        r= IMP.algebra.Rotation3D(q[0], q[1], q[2], q[3])
        #t= IMP.algebra.random_vector_in_unit_box()
        t= IMP.algebra.Vector3D(1,0,0)
        tr= IMP.algebra.Transformation3D(r, t)
        rbd.set_transformation(tr)
        targets=[]
        for i in range(0,len(xyzs)):
            md= IMP.core.XYZDecorator(xyzs[i].get_particle())
            iv= md.get_coordinates()
            t=tr.transform(iv)
            md.set_coordinates(t)
        return tr


    def _write_state(self, name, rbd, pr, xyzs, targets):
        w= IMP.display.BildWriter()
        w.set_file_name(self.get_tmp_file_name(name))
        r=.01
        #c= IMP.algebra.Vector3D(.8, .2, .2)
        sg=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(targets[0],r))
        sg.set_color(IMP.display.Color(.9, .2, .2))
        w.add_geometry(sg)
        sg=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(targets[1],r))
        sg.set_color(IMP.display.Color(.7*.9, .2, .2))
        w.add_geometry(sg)
        sg=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(targets[2],r))
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

    def _check_optimization(self, m, xyzs, rbp, traits, trans,
                            targets, steps):
        rbd= IMP.core.RigidBodyDecorator(rbp, traits)
        cg= IMP.core.SteepestDescent()
        cg.set_model(m)
        print "check move"
        cg.optimize(10)
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
        trans.show(); print
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
        l= IMP.core.ListSingletonContainer()
        l.add_particle(rbd.get_particle())
        IMP.core.setup_rigid_bodies(m, l, pr, rbd.get_traits(),
                         True)
        self._check_optimization(m, xyzs, rbd, pr, tr, targets, 100)

    def test_optimized(self):
        """Test rigid body direct optimization via setup"""
        m= self._setup()
        (xyzs, tr) = self._create_children(m)
        (rbp, pr)= self._create_hierarchy(m, xyzs)
        for d in xyzs:
            d.set_coordinates_are_optimized(False)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        targets= self._create_restraints(m, xyzs, tr)
        l= IMP.core.ListSingletonContainer()
        l.add_particle(rbp)
        IMP.core.setup_rigid_bodies(m, l, pr, tr,
                         False)
        IMP.core.RigidBodyDecorator(rbp, tr).set_coordinates_are_optimized(True)
        trans= self._transform_particles(rbp, tr, xyzs)
        self._check_optimization(m, xyzs, rbp, tr, trans, targets, 100)

    def test_optimized_setup(self):
        """Test rigid body direct optimization via setup"""
        m= self._setup()
        (xyzs, tr) = self._create_children(m)
        for d in xyzs:
            d.set_coordinates_are_optimized(False)
        # apply restraints to 3 particles
        # optimize then check if the remainder are in place
        targets= self._create_restraints(m, xyzs, tr)
        ps= IMP.Particles()
        for d in xyzs:
            ps.append(d.get_particle())
        rbp= IMP.core.create_rigid_body(m, ps, tr,
                         False)
        IMP.core.RigidBodyDecorator(rbp, tr).set_coordinates_are_optimized(True)
        trans= self._transform_particles(rbp, tr, xyzs)
        self._check_optimization(m, xyzs, rbp, tr, trans, targets, 100)

if __name__ == '__main__':
    unittest.main()
