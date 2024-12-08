## \example kernel/basic_optimization.py
# This illustrates a basic main loop for optimization and searching for the
# best scoring conformation.
#

import IMP.container
import IMP.statistics
import sys

IMP.setup_from_argv(sys.argv, "Basic optimization")

# Make 100 particles randomly distributed in a cubic box
m = IMP.Model()
lsc = IMP.container.ListSingletonContainer(m)
b = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))
for i in range(100):
    p = m.add_particle("p")
    lsc.add(p)
    d = IMP.core.XYZR.setup_particle(m, p,
              IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(b), 1))
    d.set_coordinates_are_optimized(True)

ps = IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(1, 1))
r = IMP.container.PairsRestraint(ps, IMP.container.ClosePairContainer(lsc, 2.0))
sf = IMP.core.RestraintsScoringFunction([r])
# we don't want to see lots of log messages about restraint evaluation
m.set_log_level(IMP.WARNING)

# the container (lsc) stores a list of particle indices
pis = lsc.get_contents()

s = IMP.core.MCCGSampler(m)
s.set_scoring_function(sf)
s.set_number_of_attempts(10)
# but we do want something to watch
s.set_log_level(IMP.TERSE)
s.set_number_of_monte_carlo_steps(10)
# find some configurations which move the particles far apart
configs = s.create_sample()
for i in range(0, configs.get_number_of_configurations()):
    configs.load_configuration(i)
    # print out the sphere containing the point set
    # - Why? - Why not?
    sphere = IMP.core.get_enclosing_sphere(IMP.get_particles(m, pis))
    print(sphere)

# cluster the solutions based on their coordinates
e = IMP.statistics.ConfigurationSetXYZEmbedding(configs, lsc)

# of course, this doesn't return anything of interest since the points are
# randomly distributed, but, again, why not?
clustering = IMP.statistics.create_lloyds_kmeans(e, 3, 1000)
for i in range(0, clustering.get_number_of_clusters()):
    # load the configuration for a central point
    configs.load_configuration(clustering.get_cluster_representative(i))
    sphere = IMP.core.get_enclosing_sphere(IMP.get_particles(m, pis))
    print(sphere)
