from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.display


class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.VERBOSE)

    def _testit(self, w, cylinder=True, triangle=True):
        print("create sg")
        sg = IMP.display.SphereGeometry(
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        sg.set_name("sphere")
        print("setting color")
        sg.set_color(IMP.display.Color(1, 0, 0))
        print("add 0")
        w.add_geometry(sg)
        if cylinder:
            sg = IMP.display.CylinderGeometry(
                IMP.algebra.Cylinder3D(
                    IMP.algebra.Segment3D(IMP.algebra.Vector3D(1, 2, 3),
                                          IMP.algebra.Vector3D(4, 5, 6)), 1))
            sg.set_name("cylinder")
            sg.set_color(IMP.display.Color(0, 1, 0))
            print("add 1")
            w.add_geometry(sg)
        if triangle:
            vs = IMP.algebra.Triangle3D(IMP.algebra.Vector3D(1, 2, 3),
                                        IMP.algebra.Vector3D(4, 5, 6),
                                        IMP.algebra.Vector3D(0, 8, 9))
            sg = IMP.display.TriangleGeometry(vs)
            sg.set_color(IMP.display.Color(0, 0, 1))
            w.add_geometry(sg)

    def test_5(self):
        """Testing the CGO writer"""
        nm = self.get_tmp_file_name("test.pymol.pym")
        w = IMP.display.PymolWriter(nm)
        self._testit(w)


if __name__ == '__main__':
    IMP.test.main()
