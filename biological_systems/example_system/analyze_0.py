import IMP
import IMP.atom
import IMP.container
import IMP.display
import IMP.statistics
import IMP.example
import parameters
import setup
import glob
import os.path

# cluster the conformations and write them to a file
def analyze_conformations(cs, all, gs):
    # we want to cluster the configurations to make them easier to understand
    # in the case, the clustering is pretty meaningless
    embed= IMP.statistics.ConfigurationSetXYZEmbedding(cs,
                  IMP.container.ListSingletonContainer(IMP.atom.get_leaves(all)), True)
    k= parameters.number_of_clusters
    if cs.get_number_of_configurations() < k:
        k= cs.get_number_of_configurations()
    cluster= IMP.statistics.get_lloyds_kmeans(embed, k, 10000)
    # dump each cluster center to a file so it can be viewed.
    for i in range(cluster.get_number_of_clusters()):
        center= cluster.get_cluster_center(i)
        cs.load_configuration(i)
        w= IMP.display.PymolWriter(IMP.system.get_output_path("cluster.%d.pym"%i))
        for g in gs:
            w.add_geometry(g)


# now do the actual work
(m,all)= setup.create_representation()
gs= setup.create_geometry(all)

cs= IMP.ConfigurationSet(m)
for f in glob.glob(IMP.system.get_input_path("configurations_*.bimp")):
    print f
    IMP.read_configuration_set(f, IMP.atom.get_leaves(all), IMP.core.XYZ.get_xyz_keys(), cs)

analyze_conformations(cs, all, gs)
