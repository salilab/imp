## \example statistics/kmeans.py
## Clustering is very simple. The example generates some random points in
## clusters and extracts the clusters. To cluster density, configurations
## or particles, replace the IMP.statistics.Vector3DEmbedding with a
## IMP.statistics.Vector3DEmbedding with a
## IMP::statistics::HighDensityEmbedding,
## IMP::statistics::ConfigurationSetXYZEmbedding or a
## IMP::statistics::ParticleEmbedding.

import IMP.algebra
import IMP.statistics

# generate some clusters of points
vs= []
centers=(IMP.algebra.Vector3D(0,0,0),
         IMP.algebra.Vector3D(10,15,20),
         IMP.algebra.Vector3D(60,30,12))
for i in range(0,3):
    for j in range(0,100):
        vs.append(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 10)))

# cluster them into 3 clusters
e= IMP.statistics.VectorDEmbedding(vs)
c= IMP.statistics.create_lloyds_kmeans(e,
                                    3, 1000)

# print out the cluster results
print c.get_cluster_center(0)
print c.get_cluster_center(1)
print c.get_cluster_center(2)
