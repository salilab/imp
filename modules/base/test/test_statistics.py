import IMP.base
import IMP.test
import StringIO

class Tests(IMP.test.TestCase):

    def test_statistics(self):
        """Test statistics"""
        sio = StringIO.StringIO()
        IMP.base.show_timings(sio)
        print sio.getvalue()
        IMP.base.clear_statistics()
        sio = StringIO.StringIO()
        IMP.base.show_timings(sio)
        print sio.getvalue()

if __name__ == '__main__':
    IMP.test.main()
