import IMP
import IMP.test
import IMP.algebra
import math

def get_nn(vs, i):
    bd= 1000000
    bi=[]
    cv= vs[i]
    for ci in range(0, len(vs)):
        if ci == i:
            continue
        cd= (vs[i]-vs[ci]).get_magnitude()
        if cd < 1.1*bd:
            #print "best is " + str(ci)
            bd= cd
            bi=[ci]
    for ci in range(0, len(vs)):
        if ci == i:
            continue
        cd= (vs[i]-vs[ci]).get_magnitude()
        if cd < 1.1*bd:
            #print "best is " + str(ci)
            bd= cd
            bi.append(ci)
    return bi

class Tests(IMP.test.TestCase):
    def test_nn_query(self):
        """Check that NN queries work"""
        bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-50,-50,-50),
                                     IMP.algebra.Vector3D(50,50,50))

        vecs=[]
        for i in range(10):
            vecs.append(IMP.algebra.get_random_vector_in(bb))
        nn= IMP.algebra.NearestNeighbor3D(vecs)
        for i in range(10):
            v = IMP.algebra.get_random_vector_in(bb)
            n1 = nn.get_nearest_neighbor(v)
            #find the closest point
            closet_v=0
            min_dist=IMP.algebra.get_distance(v,vecs[closet_v])
            for j in range(1,10):
                if min_dist>IMP.algebra.get_distance(v,vecs[j]):
                    min_dist=IMP.algebra.get_distance(v,vecs[j])
                    closet_v=j
            print n1,closet_v
            self.assertEqual(n1,closet_v)
    def test_nn_functionality(self):
        """Check that nearest neighbor works"""
        nump=60
        vs= []
        for i in range(0, nump):
            v= IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
            vs.append(v)
        nn= IMP.algebra.NearestNeighbor3D(vs)
        for i in range(0, nump):
            cnn= nn.get_nearest_neighbor(i)
            self.assertNotEqual(cnn, i)
            rnn= get_nn(vs, i)
            #print i
            print "start", i
            print "vs", vs[i]
            print "cnn", cnn
            print "rnn", rnn
            print vs[cnn], (vs[i]-vs[cnn]).get_magnitude()
            for x in rnn:
                print vs[x], (vs[x]-vs[i]).get_magnitude()
            #print
            self.assertNotEqual(rnn.index(cnn), -1)

if __name__ == '__main__':
    IMP.test.main()
