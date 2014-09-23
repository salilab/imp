import IMP
import IMP.core
import IMP.isd
from math import pi, log, exp


def setupnuisance(m, initialvalue, minvalue, maxvalue, isoptimized=True):

        nuisance = IMP.isd.Scale.setup_particle(IMP.Particle(m), initialvalue)
        if minvalue:
            nuisance.set_lower(minvalue)
        if maxvalue:
            nuisance.set_upper(maxvalue)

        nuisance.set_is_optimized(nuisance.get_nuisance_key(), isoptimized)
        nuisance = nuisance
        return nuisance


def sphere_cap(r1, r2, d):
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


def get_probability(xyz1, xyz2, sigma1, sigma2, psi, lenght, slope):

    onemprob = 1.0

    for k in range(0, 1):
        dist = IMP.core.get_distance(xyz1, xyz2)
        psi = psi.get_scale()
        sigmai = sigma1.get_scale()
        sigmaj = sigma2.get_scale()

        voli = 4.0 / 3.0 * pi * sigmai * sigmai * sigmai
        volj = 4.0 / 3.0 * pi * sigmaj * sigmaj * sigmaj

        fi = 0
        fj = 0

        if dist < sigmai + sigmaj:
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
        onemprob = onemprob * \
            (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi))
             * exp(-slope * dist))

    prob = 1.0 - onemprob

    return prob


m = IMP.Model()
p1 = IMP.kernel.Particle(m)
p2 = IMP.kernel.Particle(m)
p3 = IMP.kernel.Particle(m)

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
dr.add_contribution((p1.get_index(), p2.get_index()),
                    (sigma1.get_particle_index(), sigma2.get_particle_index()),
                    psi.get_particle_index())
dr.add_contribution((p3.get_index(), p2.get_index()),
                    (sigma3.get_particle_index(), sigma2.get_particle_index()),
                    psi.get_particle_index())

lw = IMP.isd.LogWrapper([dr], 1.0)

maxdist = 40.0
npoints = 100

dists = []
scores = []
for i in range(npoints):
    xyz2.set_coordinates(
        IMP.algebra.Vector3D(maxdist / npoints * float(i), 0.0, 0.0))
    dist = IMP.core.get_distance(xyz1, xyz2)
    scoretest = - \
        log(get_probability(xyz1, xyz2, sigma1, sigma2, psi, length, slope))
    score = lw.unprotected_evaluate(None)

    print dist, scoretest, score
