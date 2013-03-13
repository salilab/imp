import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.rmf
import RMF

class Tests(IMP.test.TestCase):
    def _create_rb(self, m):
        prb= IMP.Particle(m, "body")
        h0= IMP.atom.Hierarchy.setup_particle(prb)
        core= IMP.Particle(m, "core")
        IMP.core.XYZR.setup_particle(core).set_radius(1)
        h0.add_child(IMP.atom.Hierarchy.setup_particle(core))
        ps=[core]
        IMP.atom.Mass.setup_particle(core, 1)
        for i in range(0,3):
            ep= IMP.Particle(m, "ep"+str(i))
            d=IMP.core.XYZR.setup_particle(ep)
            d.set_coordinate(i, 1)
            d.set_radius(.1)
            IMP.atom.Mass.setup_particle(ep, 1)
            ps.append(d)
            h0.add_child(IMP.atom.Hierarchy.setup_particle(ep))
        IMP.core.RigidBody.setup_particle(prb, ps).set_coordinates_are_optimized(True)
        rbd=IMP.atom.RigidBodyDiffusion.setup_particle(prb)
        rbd.set_rotational_diffusion_coefficient(rbd.get_rotational_diffusion_coefficient()*10)
        return prb, ep, core
    def test_bonded(self):
        """Check brownian dynamics with rigid bodies"""
        m = IMP.Model()
        RMF.set_log_level("Off")
        m.set_log_level(IMP.base.SILENT)
        pa, ma, ca=self._create_rb(m)
        pb, mb, cb=self._create_rb(m)
        ps0= IMP.core.HarmonicDistancePairScore(0, 10)
        ps1= IMP.core.SoftSpherePairScore(100)
        r0=IMP.core.PairRestraint(ps0, (ma, mb))
        IMP.display.Colored.setup_particle(ma, IMP.display.Color(1,0,0))
        IMP.display.Colored.setup_particle(mb, IMP.display.Color(1,0,0))
        r1= IMP.core.PairRestraint(ps1, (ca, cb))
        bd= IMP.atom.BrownianDynamics(m)
        bb= IMP.algebra.get_unit_bounding_box_3d()
        for p in (pa, pb):
            rot= IMP.algebra.get_random_rotation_3d()
            tr=IMP.algebra.get_random_vector_in(bb)
            rf= IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(rot,
                                                                          tr))
            IMP.core.RigidBody(pa).set_reference_frame(rf)
        rmf= RMF.create_rmf_file(self.get_tmp_file_name("bd_rb.rmf"))
        IMP.rmf.add_hierarchies(rmf, [pa, pb])
        IMP.rmf.add_restraints(rmf, [r0, r1])
        sf= IMP.core.RestraintsScoringFunction([r0, r1])
        sf.set_log_level(IMP.base.SILENT)
        os= IMP.rmf.SaveOptimizerState(rmf)
        os.set_log_level(IMP.base.SILENT)
        bd.set_scoring_function(sf)
        bd.add_optimizer_state(os)
        bd.set_maximum_time_step(10)
        IMP.base.set_log_level(IMP.base.VERBOSE)
        bd.optimize(10)
        print "going silent"
        IMP.base.set_log_level(IMP.base.SILENT)
        bd.optimize(1000)
        e= sf.evaluate(False)
        self.assertLess(e, 2)

if __name__ == '__main__':
    IMP.test.main()
