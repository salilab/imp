import IMP
import IMP.test
import IMP.core
import IMP.display
import StringIO
import re

class Tests(IMP.test.TestCase):
    def test_5(self):
        """Testing the jet color map"""
        w= IMP.display.PymolWriter(self.get_tmp_file_name("jet.pym"))
        for i in range(0,100):
            f=float(i)/100
            print f
            g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,i), 5))
            g.set_color(IMP.display.get_jet_color(f))
            w.add_geometry(g)

if __name__ == '__main__':
    IMP.test.main()
