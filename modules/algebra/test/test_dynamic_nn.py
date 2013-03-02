import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):
    def test_nn_query(self):
        """Check that dynamic nearest neighbor queries work"""
        bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-10,-10,-10),
                                     IMP.algebra.Vector3D(10,10,10))

        vecs=[]
        for i in range(10):
            vecs.append(IMP.algebra.get_random_vector_in(bb))
        nn= IMP.algebra.DynamicNearestNeighbor3D(vecs, 2)
        print vecs
        for i in range(10):
            n1 = nn.get_in_ball(i, i)
            print "found", n1
            for ij, j in enumerate(vecs):
                if IMP.algebra.get_distance(vecs[i], j) < i*.98 and ij != i:
                    print ij, j, IMP.algebra.get_distance(vecs[i], j)
                    self.assertIn(ij, n1)
        for i in range(10):
            npt=IMP.algebra.get_random_vector_in(bb)
            vecs[i]=npt
            print "now", i, npt
            nn.set_coordinates(i, npt)
            n1 = nn.get_in_ball(i, i)
            print "xfound", n1
            for ij, j in enumerate(vecs):
                if IMP.algebra.get_distance(vecs[i], j) < i*.98 and ij != i:
                    print ij, j, IMP.algebra.get_distance(vecs[i], j)
                    self.assertIn(ij, n1)
if __name__ == '__main__':
    IMP.test.main()
