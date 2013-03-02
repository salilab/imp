import IMP
import IMP.test
import IMP.core
import IMP.display
import StringIO
import re

class Tests(IMP.test.TestCase):
    def test_5(self):
        """Testing the CMM writer and frames"""
        name=self.get_tmp_file_name("cmm.%1%.py")
        m= IMP.Model()
        p=IMP.Particle(m)
        d= IMP.core.XYZR.setup_particle(p)
        w=IMP.display.CMMWriter(name)
        names=[]
        for i in range(0,5):
            w.set_frame(i)
            g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(i,i,i), i))
            names.append(w.get_current_file_name())
            w.add_geometry(g)
            g= IMP.core.XYZRGeometry(p)
            w.add_geometry(g)
        del w
        for n in names:
            print n
            contents= open(n, "r").read()
            print contents
    def test_0(self):
        """Testing the CMM writer without frames"""
        name=self.get_tmp_file_name("cmm.py")
        m= IMP.Model()
        p=IMP.Particle(m)
        d= IMP.core.XYZR.setup_particle(p)
        w=IMP.display.CMMWriter(name)
        g= IMP.core.XYZRGeometry(p)
        print "first"#, w.get_current_index()
        w.add_geometry(g)
        g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 1))
        print "second", w.get_current_index()
        w.add_geometry(g)
        print "done",  w.get_current_index()
        del w
        print name
        contents= open(name, "r").read()
        cre=re.compile('id="([0123456789]*)"')
        print "contents"
        print contents
        print "matches"
        for m in cre.findall(contents):
            print m
            self.assertLess(int(m), 3)

    def test_1(self):
        """Testing the CMM writer with a singlet file but frames"""
        name=self.get_tmp_file_name("cmmframes.py")
        m= IMP.Model()
        p=IMP.Particle(m)
        d= IMP.core.XYZR.setup_particle(p)
        w=IMP.display.CMMWriter(name)
        for i in range(0,10):
            w.set_frame(i)
            g= IMP.core.XYZRGeometry(p)
            print "first"#, w.get_current_index()
            w.add_geometry(g)
            g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 1))
            print "second", w.get_current_index()
            w.add_geometry(g)
            print "done",  w.get_current_index()
        del w
        print name
        contents= open(name, "r").read()
        cre=re.compile('id="([0123456789]*)"')
        print "contents"
        print contents
        print "matches"
        self.assertEqual(len(cre.findall(contents)), 2)

if __name__ == '__main__':
    IMP.test.main()
