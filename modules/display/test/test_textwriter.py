import IMP
import IMP.test
import IMP.core
import IMP.display
import os

class DummyWriter(IMP.display.TextWriter):
    def __init__(self, name):
        IMP.display.TextWriter.__init__(self, name)
    def do_open(self):
        print "opening"
        print >> self.get_stream(), "open"
    def do_close(self):
        print "closing"
        print >> self.get_stream(), "close"
    def poke(self):
        print >> self.get_stream(), "hi"
    def close(self):
        print "bye"
        self.do_close()

class Tests(IMP.test.TestCase):
    def _testopen(self, fname):
        open(fname, "r")

    def test_3(self):
        """Testing the text writer"""
        w= DummyWriter(self.get_tmp_file_name("test.%1%.dum"))
        w.set_frame(0)
        w.poke()
        w.set_frame(1)
        w.poke()
        w.set_frame(2)
        w.poke()
        # to not bother about working out how to make sure that the director
        # objects are all cleanup up properly
        w.set_frame(3)
        print "deleted"
        c0=open(self.get_tmp_file_name("test.0.dum"), "r").read()
        c1=open(self.get_tmp_file_name("test.1.dum"), "r").read()
        c2=open(self.get_tmp_file_name("test.2.dum"), "r").read()
        self.assertEqual(c0, c1)
        self.assertEqual(c1, c2)
        self.assertEqual(c0.find("open"), 0)
        self.assertGreater(c0.find("close"), 0)
        self.assertRaises(IOError, self._testopen, self.get_tmp_file_name("test.%1%.dum"))
        print "done"

if __name__ == '__main__':
    IMP.test.main()
