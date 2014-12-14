from __future__ import print_function
import IMP.base
import IMP.test
import io

class Tests(IMP.test.TestCase):

    def test_statistics(self):
        """Test statistics"""
        sio = io.StringIO()
        IMP.base.show_timings(sio)
        print(sio.getvalue())
        IMP.base.clear_statistics()
        sio = io.StringIO()
        IMP.base.show_timings(sio)
        print(sio.getvalue())

if __name__ == '__main__':
    IMP.test.main()
