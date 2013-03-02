import IMP.test
import IMP.algebra
import StringIO
import os
import math

displayit=False
if displayit:
    import IMP.display

from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def test_magnitude(self):
        """Check dense log grid of ints"""
        print "construct"
        bb=BoundingBox3D(Vector3D(1,1,1), Vector3D(15, 15, 15))
        sz=[5,5,5]
        le=LogEmbedding3D(bb, Vector3D(2.0, 2.0, 2.0), sz);
        g= DenseIntLogGrid3D(sz, le)
        bbo= g.get_bounding_box()
        print bb, bbo
        if displayit:
            w=IMP.display.PymolWriter(self.get_tmp_file_name("log.pym"))
            bbg= IMP.display.BoundingBoxGeometry(bb)
            bbg.set_color(IMP.display.get_display_color(0))
            bbg.set_name("in")
            w.add_geometry(bbg)
            bbog= IMP.display.BoundingBoxGeometry(bbo)
            bbog.set_color(IMP.display.get_display_color(1))
            bbog.set_name("out")
            w.add_geometry(bbog)
            for i in range(0, sz[0]):
                for j in range(0, sz[0]):
                    for k in range(0, sz[0]):
                        ei = ExtendedGridIndex3D(i,j,k)
                        gi= g.get_index(ei)
                        bbi= g.get_bounding_box(ei)
                        bbog= IMP.display.BoundingBoxGeometry(bbi)
                        bbog.set_name(str(ei))
                        w.add_geometry(bbog)
                        cg= IMP.display.PointGeometry(g.get_center(ei))
                        cg.set_name("center")
                        w.add_geometry(cg)
        self.assertAlmostEqual(bbo.get_corner(1)[0], 15, delta=.1)
    def test_embedding(self):
        """Test mixed log embedding"""
        eb= IMP.algebra.LogEmbedding3D(IMP.algebra.Vector3D(0,0,0),
                                       IMP.algebra.Vector3D(1,1,1),
                                       IMP.algebra.Vector3D(1,2,1))
        for i in range(0,10):
            gi=IMP.algebra.ExtendedGridIndex3D([i,i,i])
            center= eb.get_center(gi)
            print center



if __name__ == '__main__':
    IMP.test.main()
