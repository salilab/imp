"""Utilities"""

import IMP
import math

def set_restraint_set_is_active(model, restraint_set_name, is_active):
    """Set whether the given restraint set is active (True) or inactive
       (False)"""
    for i in range(len(model.restraint_sets)):
        if restraint_set_name == model.restraint_sets[i].name():
            model.restraint_sets[i].set_is_active(is_active)


def set_up_exclusion_volumes(model, particles, radius_name, rsrs, sd = 0.1):
    """Add all needed exclusion volumes to the restraint list"""
    for i in range(len(particles)-1):
        for j in range(i+1, len(particles)):
            mean = particles[i].get_value(radius_name) \
                   + particles[j].get_value(radius_name)
            sf = IMP.HarmonicLowerBound(mean, sd)
            rsrs.append(IMP.DistanceRestraint(sf, particles[i], particles[j]))


def write_pdb(model, fname):
    """Write PDB based on particle attributes."""
# based on:
# 0         1         2         3         4         5         6         7
# 01234567890123456789012345678901234567890123456789012345678901234567890123456789
# ATOM   1887  N   LEU A 411      39.891  85.957  24.294  1.00 46.58           N
    fp = open (fname, "w")
    i = 0
    for p in model.particles:
        fp.write("ATOM %6d%3s   %s %s%4d      %5.3f  %5.3f  %5.3f  1.00  0.00           %s\n" % (i, 'CA', 'ALA', chr(64 + p.get_int("chain")), p.get_int("aa_idx"), p.x(), p.y(), p.z(), 'C'))
        i = i + 1
    fp.close()

def show_distances(particles):
    """Show distances using IMP particles"""
    for i in range(0, len(particles)):
        for j in range(i + 1, len(particles)):
            dx = particles[i].x() - particles[j].x()
            dy = particles[i].y() - particles[j].y()
            dz = particles[i].z() - particles[j].z()

            print "(", i, ",", j, " : ", math.sqrt(dx*dx + dy*dy + dz*dz), \
                  ")  ",

        print ""
