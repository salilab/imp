import IMP.test
import IMP.algebra
import StringIO
import os
import math


class Tests(IMP.test.TestCase):
    def test_magnitude(self):
        """Check sparse grid of ints"""
        print "construct"
        g= IMP.algebra.SparseUnboundedIntGrid3D(33.4, IMP.algebra.Vector3D(-4, -5, -6), 0)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-100,-100,-100),
                                      IMP.algebra.Vector3D(100,100,100))
        vs=[]
        for i in range(5):
            v= IMP.algebra.get_random_vector_in(bb)
            evi= g.get_extended_index(v)
            if g.get_has_index(evi):
                vi= g.get_index(evi)
                g[vi]=g[vi]+1
                self.assertGreater(g[v], 1)
            else:
                g.add_voxel(evi, 1)
                self.assertEqual(g[v], 1)
            vs.append(v)
        count=0
        print "list"
        for i in  g.get_indexes(g.get_extended_index(bb.get_corner(0)),
                                g.get_extended_index(bb.get_corner(1))):
            #print g[i]
            pass

        print "check"
        for j, i in enumerate(g.get_extended_indexes(g.get_extended_index(bb.get_corner(0)),
                                                     g.get_extended_index(bb.get_corner(1)))):
            #print i
            cbb= g.get_bounding_box(i)
            cbb+=-.1
            for v in vs:
                if cbb.get_contains(vs):
                    self.assertGreater(g[g.get_index(i)], 0)
            if g.get_has_index(i):
                print "adding", i, g[g.get_index(i)]
                count= count+ g[g.get_index(i)]
        print "asserting"
        self.assertEqual(count, len(vs))
        print "subtracting"
        for i in  g.get_all_indexes():
            print i, g[i]
            #print g[i]
            count= count-g[i]
        self.assertEqual(count,0)


if __name__ == '__main__':
    IMP.test.main()
