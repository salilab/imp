import IMP
import IMP.test
import IMP.core
import IMP.display
import os
import StringIO

class Tests(IMP.test.TestCase):
    def test_5(self):
        """Testing the pivy writer and frames"""
        try:
            import pivy
        except ImportError:
            self.skipTest("pivy Python module unavailable")
        g0=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(10,10,10), 3))
        g0.set_color(IMP.display.Color(0,1,0))
        g1=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1,1,1), 2))
        segment=IMP.algebra.Segment3D(IMP.algebra.Vector3D(10,10,10),
                                      IMP.algebra.Vector3D(1,1,1))
        g2= IMP.display.CylinderGeometry(IMP.algebra.Cylinder3D(segment, 1))
        g2.set_color(IMP.display.Color(0,0,1))

        gx=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(3,0,0), 1))
        gx.set_color(IMP.display.Color(1,0,0))
        gy=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,3,0), 1))
        gy.set_color(IMP.display.Color(0,1,0))
        gz=IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,3), 1))
        gz.set_color(IMP.display.Color(0,0,1))
        if "DISPLAY" not in os.environ.keys():
            self.skipTest("no DISPLAY variable")
        pw= IMP.display.PivyWriter()
        pw.add_geometry([g0, g1, g2, gx, gy, gz])
        pw.show()
if __name__ == '__main__':
    IMP.test.main()
