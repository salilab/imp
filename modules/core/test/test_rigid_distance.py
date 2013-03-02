import IMP
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Checking rigid distance pair score"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        name=self.get_input_file_name("input.pdb")
        p0= IMP.kernel._create_particles_from_pdb(name, m)
        p1= IMP.kernel._create_particles_from_pdb(self.get_input_file_name("input.pdb"), m)
        print len(p0), "particles", name
        print len(p1), "particles", name
        rb0= IMP.core.RigidBody.setup_particle(IMP.Particle(m),p0)
        rb1= IMP.core.RigidBody.setup_particle(IMP.Particle(m),p1)
        randt=IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(), IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0), IMP.algebra.Vector3D(100,100,100))))
        IMP.core.RigidBody(rb0).set_reference_frame(IMP.algebra.ReferenceFrame3D(randt))
        sdps= IMP.core.SphereDistancePairScore(IMP.core.Linear(0,1))
        tr= IMP.core.TableRefiner()
        tr.add_particle(p0[0], p0)
        tr.add_particle(p1[0], p1)
        rdps= IMP.core.RigidBodyDistancePairScore(sdps, tr)
        v= rdps.evaluate((p0[0], p1[0]), None)
        dm= 1000000
        bp=None
        for l0 in p0:
            for l1 in p1:
                d= sdps.evaluate((l0, l1), None)
                if d< dm:
                    print "found ", l0.get_name(), l1.get_name(), d
                    dm=d
        self.assertAlmostEqual(v, dm, delta=.1)


    def test_rops_against_one(self):
        """Checking rigid distance pair score against one"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        p0= IMP.kernel._create_particles_from_pdb(self.get_input_file_name("input.pdb"), m)
        print len(p0), "particles"
        p1= IMP.Particle(m)
        randt=IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0), IMP.algebra.Vector3D(100,100,100)))
        IMP.core.XYZR.setup_particle(p1, IMP.algebra.Sphere3D(randt, 3))
        rb=IMP.core.RigidBody.setup_particle(IMP.Particle(m), p0)
        sdps= IMP.core.SphereDistancePairScore(IMP.core.Linear(0,1))
        tr= IMP.core.TableRefiner();
        tr.add_particle(p0[0], p0)
        rdps= IMP.core.RigidBodyDistancePairScore(sdps, tr)
        v= rdps.evaluate((p0[0], p1), None)
        dm= 1000000
        bp=None
        for l0 in p0:
            d= sdps.evaluate((l0, p1), None)
            if d< dm:
                dm=d
        self.assertAlmostEqual(v, dm, delta=.1)


if __name__ == '__main__':
    IMP.test.main()
