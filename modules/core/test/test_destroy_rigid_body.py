import IMP
import IMP.test
import IMP.core
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Tests for RigidBody function"""

    def _check_distance(self, pa, pb, idist):
        dist= IMP.core.get_distance(IMP.core.XYZ(pa),
                                IMP.core.XYZ(pb))
        print dist
        self.assertAlmostEqual(dist, idist, delta=.1)
    def _check_not_distance(self, pa, pb, idist):
        dist= IMP.core.get_distance(IMP.core.XYZ(pa),
                                IMP.core.XYZ(pb))
        print "not", dist
        self.assertGreater((dist-idist)**2, .01)
    def _perturb_rb(self, rb):
        r= IMP.algebra.get_random_rotation_3d()
        t= IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                                                      IMP.algebra.Vector3D(10,10,10)))
        rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(r,t)))
    def _perturb_xyz(self, xyz):
        t= IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                                                      IMP.algebra.Vector3D(10,10,10)))
        xyz.set_coordinates(t)

    def _perturb_all(self, all):
        for a in all:
            if IMP.core.RigidBody.particle_is_instance(a):
                rb= IMP.core.RigidBody(a)
                self._perturb_rb(rb)
            else:
                self._perturb_xyz(IMP.core.XYZ(a))
        all[0].get_model().update()

    def test_create_one_from_pdb(self):
        """Testing destroy rigid body"""
        m= IMP.Model()
        ps=[]
        ds=[]
        for i in range(0,4):
            p= IMP.Particle(m)
            ps.append(p)
            d= IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(i, 0, 0))
            ds.append(d)
        p0= IMP.Particle(m)
        p1= IMP.Particle(m)
        p01= IMP.Particle(m)
        rb0= IMP.core.RigidBody.setup_particle(p0, [ps[0], ps[1]])
        rb1= IMP.core.RigidBody.setup_particle(p1, [ps[2], ps[3]])
        rb01= IMP.core.RigidBody.setup_particle(p01, [rb0, rb1])
        print rb0, rb1, rb01
        print IMP.core.RigidMember(rb0).get_internal_transformation(), IMP.core.RigidMember(rb1).get_internal_transformation()
        dg= IMP.get_dependency_graph(m)
        dg.show()
        #IMP.show_graphviz(dg)
        ordered= IMP.get_update_order(m.get_score_states())
        print [o.get_name() for o in ordered]
        self._perturb_all(ds+[rb0, rb1, rb01])
        print rb0, rb1, rb01
        self._check_distance(ps[0], ps[1], 1)
        self._check_distance(ps[2], ps[3], 1)
        self._check_distance(ps[1], ps[2], 1)

        IMP.core.RigidBody.teardown_particle(rb01)
        self._perturb_all(ds+[rb0, rb1, rb01])
        self._check_distance(ps[0], ps[1], 1)
        self._check_distance(ps[2], ps[3], 1)
        self._check_not_distance(ps[1], ps[2], 1)

        IMP.core.RigidBody.teardown_particle(rb0)
        self._perturb_all(ds+[rb0, rb1, rb01])
        self._check_not_distance(ps[0], ps[1], 1)
        self._check_distance(ps[2], ps[3], 1)
        self._check_not_distance(ps[1], ps[2], 1)

if __name__ == '__main__':
    IMP.test.main()
