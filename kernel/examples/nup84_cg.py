import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics
import IMP.example

k=100
resolution=1000

def create_representation():
    m= IMP.Model()
    all=IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
    all.set_name("the universe")
    def create_protein(name, ds):
        h=IMP.atom.create_protein(m, name, resolution, ds)
        leaves= IMP.atom.get_leaves(h)
        all.add_child(h)
        r=IMP.atom.create_connectivity_restraint([IMP.atom.Selection(c) for c in h.get_children()],
                                                 k)
        if r:
            m.add_restraint(r)
            #m.set_maximum_score(r, k)
    create_protein("Nup85", 570)
    ct= IMP.atom.Selection(all, molecule="Nup85", terminus= IMP.atom.Selection.C)
    d= IMP.core.XYZ(ct.get_particles()[0])
    d.set_coordinates(IMP.algebra.Vector3D(1,0,0))
    d.set_coordinates_are_optimized(False)
    create_protein("Nup84", 460)
    create_protein("Nup145C", 442)
    create_protein("Nup120", [0, 761]) # 500,
    create_protein("Nup133", [0, 1160]) #450, 778,
    create_protein("Seh1", 351)
    create_protein("Sec13", 379)
    return (m, all)

def create_restraints(m, all):
    # actually, just call IMP.atom.create_connectivity_restraint, but...
    def add_connectivity_restraint(s):
        tr= IMP.core.TableRefiner()
        rps=[]
        for sc in s:
            ps= sc.get_particles()
            rps.append(ps[0])
            tr.add_particle(ps[0], ps)
        score= IMP.core.KClosePairsPairScore(IMP.core.HarmonicSphereDistancePairScore(0,1),
                                             tr)
        r= IMP.core.ConnectivityRestraint(score, IMP.container.ListSingletonContainer(rps))
        m.add_restraint(r)
        #m.set_maximum_score(r, k)
    def add_distance_restraint(s0, s1):
        r=IMP.atom.create_distance_restraint(s0,s1, 0, k)
        m.add_restraint(r)
        #m.set_maximum_score(r, k)
    evr=IMP.atom.create_excluded_volume_restraint([all])
    m.add_restraint(evr)
    S= IMP.atom.Selection
    s0=S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)])
    s1=S(hierarchy=all, molecule="Nup84", molecules=[])
    s2=S(hierarchy=all, molecule="Sec13")
    add_connectivity_restraint([s0,s1,s2])
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Nup85"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Nup120", residue_indexes= [(500, 762)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup84"),
                           S(hierarchy=all, molecule="Nup133", residue_indexes=[(778, 1160)]))
    add_distance_restraint(S(hierarchy=all, molecule="Nup85"),
                           S(hierarchy=all, molecule="Seh1"))
    add_distance_restraint(S(hierarchy=all, molecule="Nup145C", residue_indexes=[(0,423)]),
                           S(hierarchy=all, molecule="Sec13"))
    #for l in IMP.atom.get_leaves(all):
    #    r= IMP.example.ExampleRestraint(l, k)
    #    m.add_restraint(r)
    #    m.set_maximum_score(k)


(m,all)= create_representation()
IMP.atom.show_molecular_hierarchy(all)
create_restraints(m, all)


gs=[]
for i in range(all.get_number_of_children()):
    color= IMP.display.get_display_color(i)
    n= all.get_child(i)
    name= n.get_name()
    for l in IMP.atom.get_leaves(n):
        g= IMP.display.XYZRGeometry(l)
        g.set_color(color)
        g.set_name(name)
        gs.append(g)

bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                             IMP.algebra.Vector3D(300, 300, 300))

sampler= IMP.core.MCCGSampler(m)

sampler.set_bounding_box(bb)
sampler.set_number_of_conjugate_gradient_steps(1000)
sampler.set_number_of_monte_carlo_steps(10)
sampler.set_number_of_attempts(100)
sampler.set_log_level(IMP.VERBOSE)
cs= sampler.get_sample()
print "found", cs.get_number_of_configurations(), "solutions"
for i in range(0, cs.get_number_of_configurations()):
    cs.load_configuration(i)
    w= IMP.display.PymolWriter("config.%d.pym"%i)
    for g in gs:
        w.add_geometry(g)


embed= IMP.statistics.ConfigurationSetXYZEmbedding(cs, IMP.container.ListSingletonContainer(IMP.atom.get_leaves(all)), True)
cluster= IMP.statistics.get_lloyds_kmeans(embed, 10, 10000)
for i in range(cluster.get_number_of_clusters()):
    center= cluster.get_cluster_center(i)
    cs.load_configuration(i)
    w= IMP.display.PymolWriter("cluster.%d.pym"%i)
    for g in gs:
        w.add_geometry(g)
