import IMP
import IMP.test
import IMP.core
import IMP.display
import StringIO

class Tests(IMP.test.TestCase):
    def test_5(self):
        """Testing the Chimera writer and frames"""
        name=self.get_tmp_file_name("chimera.%1%.py")
        w=IMP.display.ChimeraWriter(name)
        names=[]
        for i in range(0,5):
            w.set_frame(i)
            g= IMP.display.PointGeometry(IMP.algebra.Vector3D(i,i,i))
            names.append(w.get_current_file_name())
            w.add_geometry(g)
        del w
        for n in names:
            print n
            contents= open(n, "r").read()
            print contents

if __name__ == '__main__':
    IMP.test.main()
