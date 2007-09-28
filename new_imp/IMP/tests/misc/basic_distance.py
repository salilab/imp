import unittest

# set the appropriate search path
import sys
sys.path.append("../../")
sys.path.append("../python_libs/")
import imp2

model = imp2.Model()

print "adding particles"
particles = []
particles.append(imp2.Particle())
model.add_particle(particles[0]);
particles[0].add_float("X", 0.5, True)
particles[0].add_float("Y", 0.0, True)
particles[0].add_float("Z", 0.0, True)
particles[0].add_float("radius", 1.6)
particles[0].add_int("protein", 1)
particles[0].show()

particles.append(imp2.Particle())
model.add_particle(particles[1]);
particles[1].add_float("X", -0.5, True)
particles[1].add_float("Y", 0.0, True)
particles[1].add_float("Z", 0.0, True)
particles[1].add_float("radius", 1.0, False)
particles[1].add_int("protein", 1)
particles[1].show()

print "adding restraints"
rs = imp2.Restraint_Set("dist_rsrs")
model.add_restraint_set(rs)

score_func = imp2.Harmonic()
dist_rsr = imp2.RSR_Distance(model, particles[0], particles[1], 3.0, 0.1, score_func)
rs.add_restraint(dist_rsr)

print "adding optimizer"
steepest_descent = imp2.Steepest_Descent()


print "optimizing the model"
steepest_descent.optimize(model, 50)

model.show()
print "Score: ", model.evaluate(False)

model_data = model.get_model_data()
for i in range(0,2):
    print "Particle", i
    idx = particles[i].float_index("X")
    print "  X:", model_data.get_float(idx), " dS/dX:", model_data.get_deriv(idx);
    idx = particles[i].float_index("Y")
    print "  Y:", model_data.get_float(idx), " dS/dY:", model_data.get_deriv(idx);
    idx = particles[i].float_index("Z")
    print "  Z:", model_data.get_float(idx), " dS/dZ:", model_data.get_deriv(idx);
    idx = particles[i].float_index("radius")
    print "  radius:", model_data.get_float(idx);

print "done"
