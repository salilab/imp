from __future__ import print_function
import IMP.test
import ihm
import ihm.reader
import ihm.dumper
import sys

if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class Tests(IMP.test.TestCase):

    def test_read_write(self):
        """Test basic read/write of IHM mmCIF files"""
        s = ihm.System(title='test system')
        sio = StringIO()
        ihm.dumper.write(sio, [s])

        sio.seek(0)
        news, = ihm.reader.read(sio)
        self.assertEqual(news.title, 'test system')


if __name__ == '__main__':
    IMP.test.main()
