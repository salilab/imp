import sys
import unittest
import IMP.utils
import IMP.test, IMP
import IMP.domino
import IMP.core
import IMP.algebra
import IMP.atom
'''
Test kmeans on a set of particles randomly distributed around three centers
'''
class KMeansTests(IMP.test.TestCase):
    def load_particles(self):
        self.centers=[]
        self.ps=[]
        self.cluster_points=[]
        for i in xrange(5):
            p = IMP.Particle(self.m)
            v = IMP.algebra.Vector3D(10.*i,10.*i,10.*i)
            d= IMP.core.XYZDecorator.create(p, v)
            self.centers.append(p)
            self.ps.append(p)
            self.cluster_points.append([])
            self.cluster_points[-1].append(p)
            #add particles around the center
            for j in xrange(50):
                p1 = IMP.Particle(self.m)
                self.cluster_points[-1].append(p1)
                d2= IMP.core.XYZDecorator.create(p1,IMP.algebra.random_vector_in_sphere(v,3.))
                self.ps.append(p1)

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.NONE)
        self.m = IMP.Model()
        self.load_particles()
        self.atts=IMP.FloatKeys()
        self.atts.append(IMP.FloatKey("x"))
        self.atts.append(IMP.FloatKey("y"))
        self.atts.append(IMP.FloatKey("z"))

    def test_kmeans(self):
        km = IMP.domino.KMLProxy()
        km.initialize(self.m,self.ps,self.atts,len(self.centers))
        km.run()
        #check that points that should be in the same cluster are in the same cluster
        for i in xrange(len(self.cluster_points)):
            for j in xrange(len(self.cluster_points[i])):
                for k in xrange(j+1,len(self.cluster_points[i])):
                    self.assert_(km.get_particle_assignment(self.cluster_points[i][j]) ==
                                 km.get_particle_assignment(self.cluster_points[i][k]),
                                     "particles should be part of the same cluster ")
        #now check that the points that should be of different cluster do indeed belong
        #to different clusters
        for i in xrange(len(self.cluster_points)):
            for j in xrange(i+1,len(self.cluster_points)):
                self.assert_(not(km.get_particle_assignment(self.cluster_points[i][0]) ==
                                 km.get_particle_assignment(self.cluster_points[j][0])),
                             "particles should not be part of the same cluster ")

if __name__ == '__main__':
    unittest.main()
