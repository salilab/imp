import IMP
import IMP.test
import IMP.core
import IMP.atom


class Tests(IMP.test.TestCase):

    def _create_rb(self, m):
        prb = IMP.Particle(m, "body")
        h0 = IMP.atom.Hierarchy.setup_particle(prb)
        core = IMP.Particle(m, "core")
        IMP.core.XYZR.setup_particle(core).set_radius(1)
        h0.add_child(IMP.atom.Hierarchy.setup_particle(core))
        ps = [core]
        IMP.atom.Mass.setup_particle(core, 1)
        for i in range(0, 3):
            ep = IMP.Particle(m, "ep" + str(i))
            d = IMP.core.XYZR.setup_particle(ep)
            d.set_coordinate(i, 1)
            d.set_radius(.1)
            IMP.atom.Mass.setup_particle(ep, 1)
            ps.append(d)
            h0.add_child(IMP.atom.Hierarchy.setup_particle(ep))
        IMP.core.RigidBody.setup_particle(
            prb,
            ps).set_coordinates_are_optimized(
            True)
        rbd = IMP.atom.RigidBodyDiffusion.setup_particle(prb)
        rbd.set_rotational_diffusion_coefficient(
            rbd.get_rotational_diffusion_coefficient() * 10)
        return prb, ep, core

    def test_bonded(self):
        """Check brownian dynamics with rigid bodies"""
        m = IMP.Model()
        m.set_log_level(IMP.SILENT)
        pa, ma, ca = self._create_rb(m)
        pb, mb, cb = self._create_rb(m)
        ps0 = IMP.core.HarmonicDistancePairScore(0, 10)
        ps1 = IMP.core.SoftSpherePairScore(100)
        r0 = IMP.core.PairRestraint(m, ps0, (ma, mb))
        IMP.display.Colored.setup_particle(ma, IMP.display.Color(1, 0, 0))
        IMP.display.Colored.setup_particle(mb, IMP.display.Color(1, 0, 0))
        r1 = IMP.core.PairRestraint(m, ps1, (ca, cb))
        bd = IMP.atom.BrownianDynamics(m)
        bb = IMP.algebra.get_unit_bounding_box_3d()
        for p in (pa, pb):
            rot = IMP.algebra.get_random_rotation_3d()
            tr = IMP.algebra.get_random_vector_in(bb)
            rf = IMP.algebra.ReferenceFrame3D(IMP.algebra.Transformation3D(rot,
                                                                           tr))
            IMP.core.RigidBody(pa).set_reference_frame(rf)
        sf = IMP.core.RestraintsScoringFunction([r0, r1])
        sf.set_log_level(IMP.SILENT)
        bd.set_scoring_function(sf)
        bd.set_maximum_time_step(10)
        IMP.set_log_level(IMP.VERBOSE)
        bd.optimize(10)
        print("going silent")
        IMP.set_log_level(IMP.SILENT)
        max_cycles = 5000
        round_cycles = 250
        total_cycles = 0
        e_threshold = 2
        for i in range(max_cycles // round_cycles):
            bd.optimize(round_cycles)
            energy = sf.evaluate(False)
            total_cycles += round_cycles
            print("energy after %d cycles = %.2f" \
                % (total_cycles, energy))
            if(energy < e_threshold):
                break
        self.assertLess(energy, e_threshold)

if __name__ == '__main__':
    IMP.test.main()
