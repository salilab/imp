import IMP
import IMP.test
import IMP.core
import IMP.display
import StringIO

class TestBL(IMP.test.TestCase):
    def test_5(self):
        """Testing the CMM writer and frames"""
        name=self.get_tmp_file_name("cmm.%1%.py")
        w=IMP.display.CMMWriter(name)
        names=[]
        for i in range(0,5):
            w.set_frame(i)
            g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(i,i,i), i))
            names.append(w.get_current_file_name())
            w.add_geometry(g)
        del w
        for n in names:
            print n
            contents= open(n, "r").read()
            print contents
    def test_0(self):
        """Testing the CMM writer without frames"""
        name=self.get_tmp_file_name("cmm.py")
        w=IMP.display.CMMWriter(name)
        g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), 1))
        w.add_geometry(g)
        del w
        print name
        contents= open(name, "r").read()
        print contents

if __name__ == '__main__':
    IMP.test.main()
