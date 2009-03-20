import unittest
import IMP, IMP.test
import IMP.core
import IMP.display
import os.path
from IMP.algebra import *

class TestBL(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)

    def _testit(self, w):
        print "create sg"
        sg=IMP.display.SphereGeometry(Sphere3D(Vector3D(1,2,3), 4))
        sg.set_name("sphere")
        print "setting color"
        sg.set_color(IMP.display.Color(1,0,0))
        print "add 0"
        w.add_geometry(sg)
        sg=IMP.display.CylinderGeometry(Cylinder3D(Segment3D(IMP.algebra.Vector3D(1,2,3),
                                                             IMP.algebra.Vector3D(4,5,6)), 1))
        sg.set_name("cylinder")
        sg.set_color(IMP.display.Color(0,1,0))
        print "add 1"
        w.add_geometry(sg)
        sg=IMP.display.TriangleGeometry(IMP.algebra.Vector3D(1,2,3),
                                        IMP.algebra.Vector3D(4,5,6),
                                        IMP.algebra.Vector3D(0,8,9))
        sg.set_color(IMP.display.Color(0,0,1))
        w.add_geometry(sg)
    def test_1(self):
        """Testing the VRML writer"""
        nm=self.get_tmp_file_name("test.vrml")
        w=IMP.display.VRMLWriter()
        w.set_file_name(nm)
        self._testit(w)

    def test_2(self):
        """Testing the CMM writer"""
        nm=self.get_tmp_file_name("test.cmm")
        w=IMP.display.CMMWriter()
        w.set_file_name(nm)
        self._testit(w)


    def test_3(self):
        """Testing the Bild writer"""
        nm=self.get_tmp_file_name("test.bild")
        w=IMP.display.BildWriter()
        w.set_file_name(nm)
        self._testit(w)

    def test_4(self):
        """Testing the chimera writer"""
        nm=self.get_tmp_file_name("test.chimera.py")
        w=IMP.display.ChimeraWriter()
        w.set_file_name(nm)
        self._testit(w)
    def test_5(self):
        """Testing the CGO writer"""
        nm=self.get_tmp_file_name("test.pymol.py")
        w=IMP.display.CGOWriter()
        w.set_file_name(nm)
        self._testit(w)


if __name__ == '__main__':
    unittest.main()
