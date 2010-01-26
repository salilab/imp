import unittest
import IMP
import IMP.test
import IMP.algebra
import math

def get_nn(vs, i):
    bd= 1000000
    bi=-1
    cv= vs[i]
    for ci in range(0, len(vs)):
        if ci == i:
            continue
        cd= (vs[i]-vs[ci]).get_squared_magnitude()
        if bd > cd:
            #print "best is " + str(ci)
            bd= cd
            bi=ci
    return bi

class ConeTests(IMP.test.TestCase):

    def test_cone_construction(self):
        """Check that nearest neighbor works"""
        nump=60
        vs= IMP.algebra.Vector3Ds()
        for i in range(0, nump):
            v= IMP.algebra.random_vector_in_unit_box()
            vs.append(v)
        nn= IMP.algebra.NearestNeighbor3D(vs)
        for i in range(0, nump):
            cnn= nn.get_nearest_neighbor(i)
            self.assert_(cnn!= i)
            rnn= get_nn(vs, i)
            #print i
            #print cnn
            #print rnn
            #print
            self.assertEqual(rnn, cnn)

if __name__ == '__main__':
    unittest.main()
