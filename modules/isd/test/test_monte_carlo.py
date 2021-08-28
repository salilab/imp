import IMP
import IMP.test
import IMP.core
import IMP.isd
import IMP.container

def setupnuisance(m, initialvalue, minvalue, maxvalue, isoptimized=True):

    nuisance = IMP.isd.Scale.setup_particle(IMP.Particle(m), initialvalue)
    if minvalue:
        nuisance.set_lower(minvalue)
    if maxvalue:
        nuisance.set_upper(maxvalue)

    nuisance.set_is_optimized(nuisance.get_nuisance_key(), isoptimized)

    return nuisance


def setup_system(coords):
    m = IMP.Model()
    mc = IMP.core.MonteCarlo(m)
    ps = []
    for i in range(len(coords)):
        p = IMP.Particle(m)
        d = IMP.core.XYZR.setup_particle(p)
        ps.append(d)
        d.set_coordinates(coords[i])
        d.set_radius(.1)
        d.set_coordinates_are_optimized(True)
    cent = IMP.core.Centroid.setup_particle(IMP.Particle(m), [ps[5], ps[6]])

    sigma1 = setupnuisance(m, 1., 0, 100, False)
    sigma2 = setupnuisance(m, 1., 0, 100, False)
    psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
    r1 = IMP.isd.CrossLinkMSRestraint(m, 5.0, 0.5)
    r1.add_contribution((ps[0], ps[1]), (sigma1, sigma2), psi)
    r2 = IMP.isd.CrossLinkMSRestraint(m, 1.0, 0.01)
    r2.add_contribution((ps[1], ps[2]), (sigma1, sigma2), psi)
    r3 = IMP.isd.CrossLinkMSRestraint(m, 1.0, 0.01)
    r3.add_contribution((ps[2], ps[3]), (sigma1, sigma2), psi)
    r4 = IMP.isd.CrossLinkMSRestraint(m, 1.0, 0.01)
    r4.add_contribution((ps[4], cent), (sigma1, sigma2), psi)
    lw = IMP.isd.LogWrapper([r1, r2, r3, r4], 1.0)
    mc.set_scoring_function(lw)
    ms = [IMP.core.BallMover(m, x, 0.05) for x in ps[:5]]
    ms.append(IMP.core.BallMover(m, ps[5:8], 0.05))
    mv = IMP.core.SerialMover(ms)
    mc.add_mover(mv)
    # Always reject upwards moves
    mc.set_kt(0.)
    mc.set_return_best(False)
    return m, mc


class Tests(IMP.test.TestCase):

    def test_moved_same_trajectory(self):
        """MonteCarlo trajectory should not be changed by set_score_moved()"""
        # This test is very similar to that in
        # modules/core/test/test_monte_carlo.py but uses XLMS restraints
        # together with LogWrapper rather than harmonic restraints with
        # RestraintSet.
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m1, mc1 = setup_system(coords)
        m2, mc2 = setup_system(coords)

        # Same seeed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)


if __name__ == '__main__':
    IMP.test.main()
