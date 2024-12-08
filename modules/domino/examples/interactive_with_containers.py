## \example domino/interactive_with_containers.py
# Run domino storing the intermediate and final results in an HDF5
# database. This has the advantage that if you interrupt the run at any
# point, you have a list of everything computed so far and so can get a
# better idea of what is going on.

import IMP.domino
import IMP.algebra
import IMP.container
import IMP
import RMF
import sys

IMP.setup_from_argv(sys.argv, "interactive with containers")

m = IMP.Model()

# create some particles
ps = IMP.get_indexes([IMP.core.XYZ.setup_particle(IMP.Particle(m))
                      for x in range(0, 3)])

s = IMP.core.HarmonicDistancePairScore(1, 1)
lpc = IMP.container.ListPairContainer(
    m, [(ps[i[0]], ps[i[1]]) for i in [(0, 1), (1, 2)]])
print([(m.get_particle_name(p[0]), m.get_particle_name(p[1]))
       for p in lpc.get_contents()])
r = IMP.container.PairsRestraint(s, lpc)
r.set_maximum_score(.1)

space = IMP.domino.XYZStates(
    [IMP.algebra.Vector3D(i, 0, 0) for i in range(0, 6)])

pst = IMP.domino.ParticleStatesTable()
for p in ps:
    pst.set_particle_states(m.get_particle(p), space)

m.set_log_level(IMP.SILENT)

# make sure to break up the
mt = IMP.domino.get_merge_tree([r], pst)
try:
    IMP.show_graphviz(mt)
except:
    print("Unable to display graph using 'dot'")
rc = IMP.domino.RestraintCache(pst)
rc.add_restraints([r])
filters = [IMP.domino.RestraintScoreSubsetFilterTable(rc),
           IMP.domino.ExclusionSubsetFilterTable(pst)]
leaf_table = IMP.domino.BranchAndBoundAssignmentsTable(pst, filters)

# create a database to store the results
name = IMP.create_temporary_file_name("assignments", ".hdf5")
root = RMF.HDF5.create_file(name)

# recurse down the tree getting the assignments and printing them


def get_assignments(vertex):
    on = mt.get_out_neighbors(vertex)
    ss = mt.get_vertex_name(vertex)
    print("computing assignments for", ss)
    ssn = str(ss)
    dataset = root.add_child_index_data_set_2d(ssn)
    dataset.set_size([0, len(ss)])
    mine = IMP.domino.WriteHDF5AssignmentContainer(
        dataset, ss, pst.get_particles(), ssn)
    if len(on) == 0:
        # we have a leaf
        IMP.domino.load_leaf_assignments(ss, leaf_table, mine)
    else:
        # recurse on the two children
        (ss0, a0) = get_assignments(on[0])
        (ss1, a1) = get_assignments(on[1])
        IMP.domino.load_merged_assignments(ss0, a0, ss1, a1, filters, mine)
    print(ss, mine.get_number_of_assignments())
    # make sure that the cache is flushed
    del mine
    return (
        (ss, IMP.domino.ReadHDF5AssignmentContainer(
            dataset, ss, pst.get_particles(), ssn))
    )


# the root is the last vertex
all = get_assignments(mt.get_vertices()[-1])
all[1].set_was_used(True)

print('try: h5dump', name)
