## \example kernel/nup84_cg.py
## Use connectivity data taken from the Nup84 complex to provide coarse grained structure models of the complex. This example shows a (simple) full IMP modeling protocol from choice of representation and scoring to sampling and clustering and analysis of the solutions. The data and representation is similar to that of the \ref modules/restrainer/nup84_complex_in_bead_representation.py "Nup84 example in IMP.restrainer".

import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics
import IMP.example

# the spring constant to use, it doesn't really matter
k=100
# the target resolution for the representation, this is used to specify how detailed
# the representation used should be
resolution=300
# the box to perform everything in
bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-300,-300,-300),
                             IMP.algebra.Vector3D(300, 300, 300))

IMP.random_number_generator.seed(2025)
# this function creates the molecular hierarchies for the various involved proteins
def create_representation():
    m= IMP.Model()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
    all.set_name("the universe")
    # create a protein, represented as a set of connected balls of appropriate
    # radii and number, chose by the resolution parameter and the number of
    # amino acids.
    def create_protein(name, ds):
        h=IMP.atom.create_protein(m, name, resolution, ds)
        # for convenience, have one molecular hierarchy containing all molecules
        all.add_child(h)
        r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c)\
                                                  for c in h.get_children()],
                                                 k)
        if r:
            m.add_restraint(r)
            # only allow the particles to separate by 1 angstrom
            m.set_maximum_score(r, k)
    create_protein("Nup85", 570)
    ct= IMP.atom.Selection(all, molecule="Nup85", terminus= IMP.atom.Selection.C)
    d= IMP.core.XYZ(ct.get_selected_particles()[0])
    # pin NUP85 to remove a degree of freedom
    d.set_coordinates(IMP.algebra.Vector3D(1,0,0))
    d.set_coordinates_are_optimized(False)
    create_protein("Nup84", 460)
    create_protein("Nup145C", 442)
    create_protein("Nup120", [0, 500, 761])
    create_protein("Nup133", [0, 450, 778, 1160])
    create_protein("Seh1", 351)
    create_protein("Sec13", 379)
    return (m, all)


# create the needed restraints and add them to the model
def create_restraints(m, all):
    def add_connectivity_restraint(s):
        tr= IMP.core.TableRefiner()
        rps=[]
        for sc in s:
            ps= sc.get_selected_particles()
            rps.append(ps[0])
            tr.add_particle(ps[0], ps)
        # duplicate the IMP.atom.create_connectivity_restraint functionality
        score= IMP.core.KClosePairsPairScore(IMP.core.HarmonicSphereDistancePairScore(0,1),
                                             tr)
        r= IMP.core.ConnectivityRestraint(score, IMP.container.ListSingletonContainer(rps))
        m.add_restraint(r)
        m.set_maximum_score(r, k)
    def add_distance_restraint(s0, s1):
        r=IMP.atom.create_distance_restraint(s0,s1, 0, k)
        m.add_restraint(r)
        # only allow the particles to separate by one angstrom
        m.set_maximum_score(r, k)
    evr=IMP.atom.create_excluded_volume_restraint([all])
    m.add_restraint(evr)
    # a Selection allows for natural specification of what the restraints act on
    S= IMP.atom.Selection
    s0=S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)])
    s1=S(hierarchy=all, molecule="Nup84", molecules=[])
    s2=S(hierarchy=all, molecule="Sec13")
    add_connectivity_restraint([s0,s1,s2])
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C",
                             residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Nup85"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C",
                             residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Nup120",
                             residue_indexes= [(500, 762)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup84"),
                           S(hierarchy=all, molecule="Nup133",
                             residue_indexes=[(778, 1160)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup85"),
                           S(hierarchy=all, molecule="Seh1"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C",
                             residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Sec13"))
    #for l in IMP.atom.get_leaves(all):
    #    r= IMP.example.ExampleRestraint(l, k)
    #    m.add_restraint(r)
    #    m.set_maximum_score(k)


# find acceptable conformations of the model
def get_conformations(m):
    sampler= IMP.core.MCCGSampler(m)

    sampler.set_bounding_box(bb)
    # magic numbers, experiment with them and make them large enough for things to work
    sampler.set_number_of_conjugate_gradient_steps(100)
    sampler.set_number_of_monte_carlo_steps(10)
    sampler.set_number_of_attempts(20)
    # We don't care to see the output from the sampler
    sampler.set_log_level(IMP.SILENT)

    # return the IMP.ConfigurationSet storing all the found configurations that
    # meet the various restraint maximum scores.
    cs= sampler.get_sample()
    return cs

# cluster the conformations and write them to a file
def analyze_conformations(cs, all, gs):
    # we want to cluster the configurations to make them easier to understand
    # in the case, the clustering is pretty meaningless
    embed= IMP.statistics.ConfigurationSetXYZEmbedding(cs,
                 IMP.container.ListSingletonContainer(IMP.atom.get_leaves(all)), True)
    cluster= IMP.statistics.create_lloyds_kmeans(embed, 10, 10000)
    # dump each cluster center to a file so it can be viewed.
    w= IMP.display.PymolWriter("cluster.pym")
    for i in range(cluster.get_number_of_clusters()):
        rep= cluster.get_cluster_representative(i)
        cs.load_configuration(rep)
        w.set_frame(i)
        for g in gs:
            w.add_geometry(g)



# now do the actual work
(m,all)= create_representation()
IMP.atom.show_molecular_hierarchy(all)
create_restraints(m, all)


# in order to display the results, we need something that maps the particles onto
# geometric objets. The IMP.display.Geometry objects do this mapping.
# IMP.core.XYZRGeometry map an IMP.core.XYZR particle onto a sphere
gs=[]
for i in range(all.get_number_of_children()):
    color= IMP.display.get_display_color(i)
    n= all.get_child(i)
    name= n.get_name()
    g= IMP.atom.HierarchyGeometry(n)
    g.set_color(color)
    gs.append(g)

cs= get_conformations(m)

print "found", cs.get_number_of_configurations(), "solutions"

# for each of the configuration, dump it to a file to view in pymol
w= IMP.display.PymolWriter("config.pym")
for i in range(0, cs.get_number_of_configurations()):
    cs.load_configuration(i)
    w.set_frame(i)
    for g in gs:
        w.add_geometry(g)

analyze_conformations(cs, all, gs)
