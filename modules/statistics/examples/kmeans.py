import IMP.algebra
import IMP.statistics

# generate some clusters of points
vs= IMP.algebra.Vector3Ds()
centers=(IMP.algebra.Vector3D(0,0,0),
         IMP.algebra.Vector3D(10,15,20),
         IMP.algebra.Vector3D(60,30,12))
for i in range(0,3):
    for j in range(0,100):
        vs.push_back(IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(centers[i], 10)))

# cluster them into 3 clusters
e= IMP.statistics.Vector3DEmbedding(vs)
c= IMP.statistics.get_lloyds_kmeans(e,
                                    3, 1000)

# print out the cluster results
print c.get_cluster_center(0)
print c.get_cluster_center(1)
print c.get_cluster_center(2)
