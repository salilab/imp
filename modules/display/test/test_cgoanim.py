import unittest
import IMP
import IMP.test
import IMP.core
import IMP.display

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def _testit(self, w, x):
        print "create sg"
        sg=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(x,2,3), 4))
        sg.set_name("sphere")
        print "setting color"
        sg.set_color(IMP.display.Color(1,0,0))
        print "add 0"
        w.add_geometry(sg)
        sg=IMP.display.CylinderGeometry(IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(x,2,3),
                                                             IMP.algebra.Vector3D(x+3,5,6)), 1))
        sg.set_name("cylinder")
        sg.set_color(IMP.display.Color(0,1,0))
        print "add 1"
        w.add_geometry(sg)
        sg=IMP.display.TriangleGeometry(IMP.algebra.Vector3D(x+1,2,3),
                                        IMP.algebra.Vector3D(x+4,5,6),
                                        IMP.algebra.Vector3D(x,8,9))
        sg.set_color(IMP.display.Color(0,0,1))
        w.add_geometry(sg)

    def test_5(self):
        """Testing the CGOAnimation writer"""
        nm=self.get_tmp_file_name("test.pymol.py")
        w=IMP.display.CGOAnimationWriter(nm)
        for i in range(0,20):
            w.set_file_name(nm)
            self._testit(w,i)


if __name__ == '__main__':
    unittest.main()
