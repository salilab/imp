import unittest
import IMP

model = IMP.Model()

print "adding particles"
particles = []
particles.append(IMP.Particle())
model.add_particle(particles[0]);
particles[0].add_float("x", 0.5, True)
particles[0].add_float("y", 0.0, True)
particles[0].add_float("z", 0.0, True)
particles[0].add_float("radius", 1.6)
particles[0].add_int("protein", 1)
particles[0].show()

particles.append(IMP.Particle())
model.add_particle(particles[1]);
particles[1].add_float("x", -0.5, True)
particles[1].add_float("y", 0.0, True)
particles[1].add_float("z", 0.0, True)
particles[1].add_float("radius", 1.0, False)
particles[1].add_int("protein", 1)
particles[1].show()

print "adding restraints"
rs = IMP.RestraintSet("dist_rsrs")
model.add_restraint_set(rs)

score_func = IMP.Harmonic()
dist_rsr = IMP.DistanceRestraint(model, particles[0], particles[1], 3.0, 0.1, score_func)
rs.add_restraint(dist_rsr)

print "adding optimizer"
steepest_descent = IMP.SteepestDescent()


print "optimizing the model"
steepest_descent.optimize(model, 50)

model.show()
print "Score: ", model.evaluate(False)

model_data = model.get_model_data()
for i in range(0,2):
    print "Particle", i
    idx = particles[i].get_float_index("x")
    print "  X:", model_data.get_float(idx), " dS/dX:", model_data.get_deriv(idx);
    idx = particles[i].get_float_index("y")
    print "  Y:", model_data.get_float(idx), " dS/dY:", model_data.get_deriv(idx);
    idx = particles[i].get_float_index("z")
    print "  Z:", model_data.get_float(idx), " dS/dZ:", model_data.get_deriv(idx);
    idx = particles[i].get_float_index("radius")
    print "  radius:", model_data.get_float(idx);

print "done"
