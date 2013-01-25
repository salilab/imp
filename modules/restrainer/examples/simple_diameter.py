## \example restrainer/simple_diameter.py
## This example shows how to create simple IMP::core::DiameterRestraint.
##

#-- File: simple_diameter.py --#

import IMP
import IMP.core
import IMP.restrainer

imp_model = IMP.Model()
ps = IMP.core.create_xyzr_particles(imp_model, 50, 1.0)
diameter = 10

sd = IMP.restrainer.create_simple_diameter(ps, diameter)

r = sd.get_restraint()
h = sd.get_harmonic_upper_bound()

sd.set_mean(10.0)
sd.set_standard_deviation(3.5)
sd.set_k(0.1)

imp_model.add_restraint(r)
r.show()

imp_model.evaluate(False)
