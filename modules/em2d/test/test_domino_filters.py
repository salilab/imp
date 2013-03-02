import IMP
import IMP.test
import IMP.em2d as em2d
import IMP.algebra as alg
import IMP.core as core
import IMP.domino as domino
import os
from math import *
import random

class Tests(IMP.test.TestCase):

    def test_distance_filter(self):
        """Test filtering distances with domino """
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m = IMP.Model()
        particles = [IMP.Particle(m) for i in range(0,3)]
        for i, p in enumerate(particles):
            p.set_name("pparticle_%d" % i)


        xyzs = [core.XYZ.setup_particle(p) for p in particles]
        positions = [alg.Vector3D(x*1.0,0.,0.) for x in range(0,100,10)]

        states = domino.XYZStates(positions)
        states_table = domino.ParticleStatesTable()
        for p in particles:
            states_table.set_particle_states(p, states)
            print p
        # Restraints
        sf = IMP.core.Harmonic(10.0, 1.0)
        r1=IMP.core.DistanceRestraint(sf, particles[0],particles[1])
        m.add_restraint(r1)
        r3=IMP.core.DistanceRestraint(sf, particles[1],particles[2])
        m.add_restraint(r3)
        ig = domino.get_interaction_graph( m.get_restraints(), states_table)
        # generate a junction tree from the interaction graph
        jt= domino.get_junction_tree(ig)
        # print jt.show_graphviz()

        # Filters
        maximum_distance = 21
        ftable2 = em2d.DistanceFilterTable(IMP.domino.Subset([particles[0], particles[1]]),
                                            states_table, maximum_distance)
        ftable2.set_name("my filtertable2")
        ftable3 = em2d.DistanceFilterTable(IMP.domino.Subset([particles[1], particles[2]]),
                                            states_table, maximum_distance)
        ftable3.set_name("my filtertable3")
        ftable1 = domino.ExclusionSubsetFilterTable(states_table)
        ftable1.set_name("my filtertable1")

        filter_tables = [ ftable2, ftable3, ftable1]

        assignments_table = \
            domino.BranchAndBoundAssignmentsTable(states_table, filter_tables)

        sampler = domino.DominoSampler(m, states_table)
        sampler.set_assignments_table(assignments_table)
        sampler.set_subset_filter_tables(filter_tables)

        configuration_set = sampler.get_sample()
        n = configuration_set.get_number_of_configurations()
        print "number of possible_configurations",n
        for i in range(n):
            configuration_set.load_configuration(i)
            # Check that the distance between the particles is correct
            self.assertLess(core.get_distance( xyzs[0], xyzs[1] ), maximum_distance)
            self.assertLess(core.get_distance( xyzs[1], xyzs[2] ), maximum_distance)



if __name__ == '__main__':
    IMP.test.main()
