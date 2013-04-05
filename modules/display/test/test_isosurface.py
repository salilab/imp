import IMP
import IMP.test
import IMP.core
import IMP.display
import os

class Tests(IMP.test.TestCase):
    def test_3(self):
        """Testing isosurface geometry"""
        if not IMP.display.IMP_DISPLAY_HAS_IMP_CGAL:
            self.skipTest("IMP.cgal module disabled")
        g=IMP.algebra.DenseDoubleGrid3D(2.5, IMP.algebra.BoundingBox3D((-10, -10, -10),
                                                                     (10,10,10)))
        for i in g.get_all_indexes():
            c= g.get_center(i)
            m= c.get_magnitude()
            g[i]=100-m
        #for i in g.get_all_indexes():
        #    print i, g.get_center(i), g[i]
        gg= IMP.display.IsosurfaceGeometry(g, 95.0)
        gg.set_name("isosurface")
        w= IMP.display.PymolWriter(self.get_tmp_file_name("iso.pym"))
        w.add_geometry(gg)

if __name__ == '__main__':
    IMP.test.main()
