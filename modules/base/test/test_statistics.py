from __future__ import print_function
import IMP.base
import IMP.test
import io
import sys

class Tests(IMP.test.TestCase):

    def test_statistics(self):
        """Test statistics"""
        if sys.version_info[0] >= 3:
            types = [io.BytesIO, io.StringIO]
        else:
            types = [io.BytesIO]
        for typ in types:
            sio = typ()
            IMP.base.show_timings(sio)
            print(sio.getvalue())
            IMP.base.clear_statistics()
            sio = typ()
            IMP.base.show_timings(sio)
            print(sio.getvalue())

if __name__ == '__main__':
    IMP.test.main()
