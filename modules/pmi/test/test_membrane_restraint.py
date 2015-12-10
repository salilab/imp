import IMP
import IMP.atom
import IMP.pmi
import IMP.test
import IMP.pmi.restraints.proteomics
import IMP.pmi.io

m = IMP.Model()

atom=IMP.Particle(m)

d=IMP.core.XYZ.setup_particle(atom)

p=IMP.Particle(m)

z_center=IMP.isd.Nuisance.setup_particle(p)
z_center.set_nuisance(0.0)

r=IMP.pmi.restraints.proteomics.MembraneRestraint(m,z_center)
r.add_particles_inside([atom])

for z in range(-500,500):
    IMP.core.XYZ(atom).set_z(z)
    print z,r.unprotected_evaluate(None)
