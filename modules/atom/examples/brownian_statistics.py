## \example atom/brownian_statistics.py
## This example prints out various statistics about a prospective Brownian dynamics simulation. You can use the statistics to determine if the time step is likely to be sufficiently short given the forces and particle sizes involved.

from IMP.atom import *

# fs
time_step=1000
# angstrom
minimum_particle_radius=10
# kCal/mol/A
maximum_spring_constant=1

maximum_diffusion_coefficient=\
    get_einstein_diffusion_coefficient(minimum_particle_radius)
expected_delta= get_diffusion_length(maximum_diffusion_coefficient,
                                     time_step)

expected_rotational_delta=\
    get_diffusion_angle(maximum_diffusion_coefficient,
                        time_step) * minimum_particle_radius

expected_spring_diffusion_length=\
    get_diffusion_length(maximum_diffusion_coefficient,
                         .5*maximum_spring_constant*4*expected_delta**2,
                         time_step)

print "with a time step of", time_step, "fs"
print "an object of radius", minimum_particle_radius, "A will move",\
    expected_delta, "A and a point on its surface will move",\
    expected_rotational_delta,"A more"
print "the motion from fluctuations in the spring compression will be",\
    expected_spring_diffusion_length, "A"
print "and a compression of 10% of the radius will induce a motion of",\
    get_diffusion_length(maximum_diffusion_coefficient,
                         .5*maximum_spring_constant*\
                             (.1*minimum_particle_radius)**2,
                         time_step), "A"
