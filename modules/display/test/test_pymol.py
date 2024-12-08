import IMP
import IMP.test
import IMP.core
import IMP.display
import io
import sys

class Tests(IMP.test.TestCase):

    def test_5(self):
        """Testing the CGO writer and frames"""
        buf = io.BytesIO()
        w = IMP.display.PymolWriter(buf)
        for i in range(0, 5):
            w.set_frame(i)
            g = IMP.display.PointGeometry(IMP.algebra.Vector3D(i, i, i))
            w.add_geometry(g)
        del w
        bstr = buf.getvalue()
        if sys.version_info[0] >= 3:
            bstr = bstr.decode('ascii')
        print(bstr)
        self.assertNotEqual(bstr.find("from pymol.cgo import"), -1)
        for i in range(0, 5):
            self.assertNotEqual(
                bstr.find("cmd.load_cgo(data[k], k, " + str(i + 1)),
                -1)

if __name__ == '__main__':
    IMP.test.main()
