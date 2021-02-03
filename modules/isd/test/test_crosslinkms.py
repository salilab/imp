from __future__ import print_function
import IMP
import IMP.core
import IMP.isd
import IMP.test
from random import sample
from math import pi, log, exp


def setupnuisance(m, initialvalue, minvalue, maxvalue, isoptimized=True):

    nuisance = IMP.isd.Scale.setup_particle(IMP.Particle(m), initialvalue)
    if minvalue:
        nuisance.set_lower(minvalue)
    if maxvalue:
        nuisance.set_upper(maxvalue)

    nuisance.set_is_optimized(nuisance.get_nuisance_key(), isoptimized)

    return nuisance


class CrossLinkMS(object):
    """Defining a dummy python restraint
    with the same interface as the one that has to be tested
    """

    def __init__(self,length,slope):
        self.pairs=[]
        self.length=length
        self.slope=slope

    def add_contribution(self,p1,p2,sigma1,sigma2,psi):
        self.pairs.append((p1,p2,sigma1,sigma2,psi))

    def get_probability(self):

        onemprob = 1.0

        for (p1,p2,sigma1,sigma2,psi) in self.pairs:
            if p1 is not p2:
                dist = IMP.core.get_distance(IMP.core.XYZ(p1), IMP.core.XYZ(p2))
            else:
                R=IMP.core.XYZR(p1).get_radius()
                dist=36.0/35.0*R
            if dist <= 0.001: dist = 0.001
            psi = psi.get_scale()
            sigmai = sigma1.get_scale()
            sigmaj = sigma2.get_scale()

            voli = 4.0 / 3.0 * pi * sigmai * sigmai * sigmai
            volj = 4.0 / 3.0 * pi * sigmaj * sigmaj * sigmaj

            fi = 0
            fj = 0

            if dist < sigmai + sigmaj:
                xlvol = 4.0 / 3.0 * pi * (self.length / 2) * (self.length / 2) * \
                    (self.length / 2)
                fi = min(voli, xlvol)
                fj = min(volj, xlvol)
            else:
                di = dist - sigmaj - self.length / 2
                dj = dist - sigmai - self.length / 2
                fi = self.sphere_cap(sigmai, self.length / 2, abs(di))
                fj = self.sphere_cap(sigmaj, self.length / 2, abs(dj))

            pofr = fi * fj / voli / volj
            onemprob = onemprob * \
                (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi))
                 * exp(-self.slope * dist))

        prob = 1.0 - onemprob

        return prob


    def sphere_cap(self,r1, r2, d):
        sc = 0.0
        if d <= max(r1, r2) - min(r1, r2):
            sc = min(4.0 / 3 * pi * r1 * r1 * r1,
                     4.0 / 3 * pi * r2 * r2 * r2)
        elif d >= r1 + r2:
            sc = 0
        else:
            sc = (pi / 12 / d * (r1 + r2 - d) * (r1 + r2 - d)) * \
                 (d * d + 2 * d * r1 - 3 * r1 * r1 + 2 * d * r2 + 6 * r1 * r2 -
                  3 * r2 * r2)
        return sc


class TestXLRestraintSimple(IMP.test.TestCase):
    def test_score_simple(self):
        """
        Test the straight pairwise restraint
        """
        #IMP.test.TestCase.setUp(self)

        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)

        slope = 0.01
        length = 10

        xyz1 = IMP.core.XYZ.setup_particle(p1)
        xyz2 = IMP.core.XYZ.setup_particle(p2)

        xyz1.set_coordinates((0, 0, 0))
        xyz2.set_coordinates((0, 0, 0))


        sigma1 = setupnuisance(m, 5, 0, 100, False)
        sigma2 = setupnuisance(m, 5, 0, 100, False)
        psi = setupnuisance(m, 0.1, 0.0, 0.5, False)

        dr = IMP.isd.CrossLinkMSRestraint(m, length, slope)
        dr.add_contribution((p1, p2), (sigma1, sigma2), psi)
        self.assertEqual(dr.get_number_of_contributions(), 1)
        self.assertEqual(dr.get_contribution_particle_indexes(0),
                         (p1.get_index(), p2.get_index()))
        self.assertEqual(dr.get_contribution_psi_index(0),
                         psi.get_particle_index())
        self.assertEqual(dr.get_contribution_sigma_indexes(0),
                         (sigma1.get_particle_index(),
                          sigma2.get_particle_index()))
        self.assertAlmostEqual(dr.get_slope(), 0.01, delta=1e-4)
        self.assertAlmostEqual(dr.get_length(), 10.0, delta=1e-4)

        lw = IMP.isd.LogWrapper([dr],1.0)

        # initialize also a restraint which output -log(prob)
        dr_lp = IMP.isd.CrossLinkMSRestraint(m, length, slope, True)
        dr_lp.add_contribution((p1, p2), (sigma1, sigma2), psi)

        testdr= CrossLinkMS(length, slope)
        testdr.add_contribution(xyz1,xyz2,sigma1,sigma2,psi)

        maxdist = 40.0
        npoints = 100

        sigmas1=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        sigmas2=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        psis=sample([0.01,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.49],5)

        for s1 in sigmas1:
            sigma1.set_scale(s1)
            for s2 in sigmas2:
                sigma2.set_scale(s2)
                for p1 in psis:
                    psi.set_scale(p1)
                    for i in range(npoints):
                        xyz2.set_coordinates(
                            IMP.algebra.Vector3D(maxdist / npoints * float(i), 0.0, 0.0))
                        dist = IMP.core.get_distance(xyz1, xyz2)
                        scoretest = - \
                            log(testdr.get_probability())
                        score = lw.unprotected_evaluate(None)
                        score_lp = dr_lp.unprotected_evaluate(None)
                        self.assertAlmostEqual(score,scoretest,places=4)
                        self.assertAlmostEqual(score_lp,scoretest,places=4)


    def test_score_multiple_restraints(self):
        """Intensive random test, it tests manifold ambiguity, sameparticle, particle positions
        totality of the score, individual scores, get_log_prob=True, multiple radii, multiple sigma,
        multiple psi"""
        import random

        m = IMP.Model()

        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(30, 30, 30))


        restraints=[]
        restraints_lp=[]
        test_restraints=[]
        for n in range(100):
            length=random.uniform(1,40)
            slope=random.uniform(0,0.1)
            dr = IMP.isd.CrossLinkMSRestraint(
                                    m,
                                    length,
                                    slope)

            dr_lp = IMP.isd.CrossLinkMSRestraint(
                                    m,
                                    length,
                                    slope,
                                    True)

            testdr= CrossLinkMS(length, slope)

            ambiguity=random.randint(1,6)
            for a in range(ambiguity):
                sameparticle=bool(random.randint(0,1))


                p1=IMP.Particle(m)
                d1 = IMP.core.XYZR.setup_particle(p1)
                d1.set_radius(random.uniform(1.0,10.0))
                d1.set_coordinates(IMP.algebra.get_random_vector_in(bb))
                d1.set_coordinates_are_optimized(True)
                s1 = setupnuisance(m, random.uniform(1.0,11.0), 0, 100, False)

                if sameparticle:
                    p2=p1
                    s2=s1
                else:
                    p2=IMP.Particle(m)
                    d2 = IMP.core.XYZR.setup_particle(p2)
                    d2.set_radius(random.uniform(1.0,10.0))
                    d2.set_coordinates(IMP.algebra.get_random_vector_in(bb))
                    d2.set_coordinates_are_optimized(True)
                    s2 = setupnuisance(m, random.uniform(1.0,11.0), 0, 100, False)

                psi = setupnuisance(m, random.uniform(0.01,0.49), 0.0, 0.5, False)

                dr.add_contribution((p1, p2), (s1, s2), psi)
                dr_lp.add_contribution((p1, p2), (s1, s2), psi)
                testdr.add_contribution(p1,p2,s1,s2,psi)

            restraints.append(dr)
            restraints_lp.append(dr_lp)
            test_restraints.append(testdr)

        lw = IMP.isd.LogWrapper(restraints,1.0)
        restraint_set_lp=IMP.RestraintSet(restraints_lp,1.0)
        test_score=sum([-log(r.get_probability()) for r  in test_restraints])
        for nr,r in enumerate(restraints):
            score=-log(r.unprotected_evaluate(None))
            score_lp=restraints_lp[nr].unprotected_evaluate(None)
            score_test=-log(test_restraints[nr].get_probability())
            self.assertAlmostEqual(score,score_test,places=4)
            self.assertAlmostEqual(score_lp,score_test,places=4)
        self.assertAlmostEqual(lw.unprotected_evaluate(None),restraint_set_lp.unprotected_evaluate(None),places=4)
        self.assertAlmostEqual(lw.unprotected_evaluate(None),test_score,places=4)
        self.assertAlmostEqual(restraint_set_lp.unprotected_evaluate(None),test_score,places=4)



    def test_score_same_bead(self):
        """
        This test checks when the cross-linked residues are assigned
        to the same particle
        """
        IMP.test.TestCase.setUp(self)

        m = IMP.Model()
        p1 = IMP.Particle(m)

        slope = 0.01
        length = 10

        xyz1 = IMP.core.XYZR.setup_particle(p1)

        xyz1.set_coordinates((0, 0, 0))


        sigma1 = setupnuisance(m, 5, 0, 100, False)
        psi = setupnuisance(m, 0.1, 0.0, 0.5, False)

        dr = IMP.isd.CrossLinkMSRestraint(m, length, slope)
        dr.add_contribution((p1, p1), (sigma1, sigma1), psi)
        lw = IMP.isd.LogWrapper([dr],1.0)

        # initialize also a restraint which output -log(prob)
        dr_lp = IMP.isd.CrossLinkMSRestraint(m, length, slope, True)
        dr_lp.add_contribution((p1, p1), (sigma1, sigma1), psi)

        testdr= CrossLinkMS(length, slope)
        testdr.add_contribution(xyz1,xyz1,sigma1,sigma1,psi)

        maxradius = 40.0
        npoints = 100

        sigmas1=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        psis=sample([0.01,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.49],5)

        for s1 in sigmas1:
            sigma1.set_scale(s1)
            for p1 in psis:
                psi.set_scale(p1)
                for i in range(npoints):
                    radius=maxradius / npoints * float(i)
                    xyz1.set_radius(radius)
                    scoretest = - \
                        log(testdr.get_probability())
                    score = lw.unprotected_evaluate(None)
                    score_lp = dr_lp.unprotected_evaluate(None)
                    self.assertAlmostEqual(score,scoretest,places=4)
                    self.assertAlmostEqual(score_lp,scoretest,places=4)

    def test_score_two_fold_ambiguity(self):
        IMP.test.TestCase.setUp(self)

        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        p3 = IMP.Particle(m)

        slope = 0.01
        length = 10

        xyz1 = IMP.core.XYZ.setup_particle(p1)
        xyz2 = IMP.core.XYZ.setup_particle(p2)
        xyz3 = IMP.core.XYZ.setup_particle(p3)

        xyz1.set_coordinates((0, 0, 0))
        xyz2.set_coordinates((0, 0, 0))
        xyz3.set_coordinates((40, 0, 0))


        sigma1 = setupnuisance(m, 5, 0, 100, False)
        sigma2 = setupnuisance(m, 5, 0, 100, False)
        sigma3 = setupnuisance(m, 5, 0, 100, False)
        psi = setupnuisance(m, 0.1, 0.0, 0.5, False)

        dr = IMP.isd.CrossLinkMSRestraint(m, length, slope)
        dr.add_contribution((p1, p2), (sigma1, sigma2), psi)
        dr.add_contribution((p3, p2), (sigma3, sigma2), psi)
        lw = IMP.isd.LogWrapper([dr],1.0)

        testdr= CrossLinkMS(length, slope)
        testdr.add_contribution(xyz1,xyz2,sigma1,sigma2,psi)
        testdr.add_contribution(xyz3,xyz2,sigma3,sigma2,psi)

        # initialize also a restraint which output -log(prob)
        dr_lp = IMP.isd.CrossLinkMSRestraint(m, length, slope, True)
        dr_lp.add_contribution((p1, p2), (sigma1, sigma2), psi)
        dr_lp.add_contribution((p3, p2), (sigma3, sigma2), psi)

        maxdist = 40.0
        npoints = 30

        sigmas1=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        sigmas2=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        sigmas3=sample([0.01,0.1,0.5,1.0,5.0,10.0,50.0,100.0],5)
        psis=sample([0.01,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.49],5)

        for s1 in sigmas1:
            sigma1.set_scale(s1)
            for s2 in sigmas2:
                sigma2.set_scale(s2)
                for s3 in sigmas3:
                    sigma3.set_scale(s3)
                    for p1 in psis:
                        psi.set_scale(p1)
                        for i in range(npoints):
                            xyz2.set_coordinates(
                                IMP.algebra.Vector3D(maxdist / npoints * float(i), 0.0, 0.0))
                            dist = IMP.core.get_distance(xyz1, xyz2)
                            scoretest = - \
                                log(testdr.get_probability())
                            score = lw.unprotected_evaluate(None)
                            score_lp = dr_lp.unprotected_evaluate(None)
                            self.assertAlmostEqual(score,scoretest,places=4)
                            self.assertAlmostEqual(score_lp,scoretest,places=4)

if __name__ == '__main__':
    IMP.test.main()
