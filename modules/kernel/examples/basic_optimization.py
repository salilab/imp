## \example kernel/basic_optimization.py
# This illustrates a basic main loop for optimization and searching for the
# best scoring conformation.
#

from __future__ import print_function
import IMP.example
import IMP.statistics
import sys

IMP.setup_from_argv(sys.argv, "Basic optimization")

(m, c) = IMP.example.create_model_and_particles()
ps = IMP.core.DistancePairScore(IMP.core.HarmonicLowerBound(1, 1))
r = IMP.container.PairsRestraint(ps, IMP.container.ClosePairContainer(c, 2.0))
sf = IMP.core.RestraintsScoringFunction([r])
# we don't want to see lots of log messages about restraint evaluation
m.set_log_level(IMP.WARNING)

# the container (c) stores a list of particle indices
pis = c.get_contents()

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
e = IMP.statistics.ConfigurationSetXYZEmbedding(configs, c)

# of course, this doesn't return anything of interest since the points are
# randomly distributed, but, again, why not?
clustering = IMP.statistics.create_lloyds_kmeans(e, 3, 1000)
for i in range(0, clustering.get_number_of_clusters()):
    # load the configuration for a central point
    configs.load_configuration(clustering.get_cluster_representative(i))
    sphere = IMP.core.get_enclosing_sphere(IMP.get_particles(m, pis))
    print(sphere)
