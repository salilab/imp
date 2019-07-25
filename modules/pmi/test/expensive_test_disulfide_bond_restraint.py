from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.container
import IMP.pmi
import IMP.pmi.io
import IMP.pmi.io.crosslink
import IMP.pmi.restraints
import IMP.pmi.restraints.crosslinking_new
from math import *

def sphere_cap(r1, r2, d):
    sc = 0.0
    if d <= max(r1, r2) - min(r1, r2):
        sc = min(4.0 / 3 * pi * r1 * r1 * r1,
                      4.0 / 3 * pi * r2 * r2 * r2)
    elif d >= r1 + r2 :
        sc = 0
    else:
        sc = (pi / 12 / d * (r1 + r2 - d) * (r1 + r2 - d)) * \
             (d * d + 2 * d * r1 - 3 * r1 * r1 + 2 * d * r2 + 6 * r1 * r2 -
              3 * r2 * r2)
    return sc

def get_probability(xyz1s,xyz2s,sigma1s,sigma2s,psis,length,slope):
    onemprob = 1.0

    for n in range(len(xyz1s)):
        xyz1=xyz1s[n]
        xyz2=xyz2s[n]
        sigma1=sigma1s[n]
        sigma2=sigma2s[n]
        psi = psis[n]
        psi = psi.get_scale()
        dist=IMP.core.get_distance(xyz1, xyz2)

        sigmai = sigma1.get_scale()
        sigmaj = sigma2.get_scale()
        voli = 4.0 / 3.0 * pi * sigmai * sigmai * sigmai
        volj = 4.0 / 3.0 * pi * sigmaj * sigmaj * sigmaj
        fi = 0
        fj = 0
        if dist < sigmai + sigmaj :
            xlvol = 4.0 / 3.0 * pi * (length / 2) * (length / 2) * \
                           (length / 2)
            fi = min(voli, xlvol)
            fj = min(volj, xlvol)
        else:
            di = dist - sigmaj - length / 2
            dj = dist - sigmai - length / 2
            fi = sphere_cap(sigmai, length / 2, abs(di))
            fj = sphere_cap(sigmaj, length / 2, abs(dj))
        pofr = fi * fj / voli / volj

        factor = (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi))*exp(-slope*dist))
        onemprob = onemprob * factor
    prob = 1.0 - onemprob
    return prob

def log_evaluate(restraints):
    prob = 1.0
    score = 0.0

    for r in restraints:
        prob *= r.unprotected_evaluate(None)
        if prob<=0.0000000001:
            score=score-log(prob)
            prob=1.0

    score=score-log(prob)
    return score


class Tests(IMP.test.TestCase):

    def test_restraint_probability_beads(self):
        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st1 = simo.create_state()
        for molname in ("ProtA", "ProtB"):
            mol = st1.create_molecule(molname, "A" * 10)
            mol.add_representation(mol.get_non_atomic_residues(),
                                   resolutions=[10])
        root_hier = simo.build()

        xl = IMP.pmi.restraints.crosslinking_new.DisulfideCrossLinkRestraint(
            root_hier,
            (1,1,"ProtA"),
            (1,1,"ProtB"),
            label="DisulfideBond1",
            resolution=1,
            slope=0.01)
        xl.add_to_model()

        p1=xl.xl["Particle1"]
        p2=xl.xl["Particle2"]
        sigma=xl.xl["Sigma"]
        psi=xl.xl["Psi"]

        xyz1=IMP.core.XYZ(p1)
        xyz2=IMP.core.XYZ(p2)

        for d in range(100):
            xyz2.set_coordinates((0,d,0))
            test_prob=get_probability([xyz1],[xyz2],[sigma],[sigma],[psi],6.5,0.01)
            self.assertAlmostEqual(xl.rs.unprotected_evaluate(None),-log(test_prob),delta=0.00001)


if __name__ == '__main__':
    IMP.test.main()
