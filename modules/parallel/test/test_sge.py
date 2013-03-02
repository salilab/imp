import IMP
import IMP.test
import IMP.parallel
import time
import sys
import os
import tempfile
import shutil

class FakeSGEEnvironment(object):
    def __init__(self, jobid=None):
        self.tmpdir = tempfile.mkdtemp()
        self._oldpath = os.environ['PATH']
        os.environ['PATH'] = self.tmpdir + ':' + self._oldpath
        for cmd in ('qsub', 'qdel', 'qrsh'):
            fname = os.path.join(self.tmpdir, cmd)
            fh = open(fname, 'w')
            fh.write("#!/bin/sh\necho $@ >> %s-commands\n" % fname)
            fh.write("cat >> %s-input\n" % fname)
            if jobid is not None and cmd == 'qsub':
                fh.write('echo "Your job has been submitted: %s"\n' \
                         % str(jobid))
            fh.close()
            os.chmod(fname, 493) # 493 = 0755 (but works in Python 2 & 3)

    def get_commands(self, cmd):
        fname = os.path.join(self.tmpdir, cmd)
        return open("%s-commands" % fname).readlines()

    def get_input(self, cmd):
        fname = os.path.join(self.tmpdir, cmd)
        return open("%s-input" % fname).readlines()

    def __del__(self):
        shutil.rmtree(self.tmpdir, ignore_errors=True)
        os.environ['PATH'] = self._oldpath


class Tests(IMP.test.TestCase):
    """Test SGE slaves"""

    def assertSGEOutputEqual(self, sge, command, expected):
        for i in range(20):
            try:
                c = sge.get_commands(command)[0].rstrip('\r\n')
            except IOError:
                c = None
            if c == expected:
                break
            time.sleep(0.05) # Wait for output files to be created
        self.assertEqual(c, expected)

    def test_pe_slave(self):
        """Test _SGEPESlave class"""
        if sys.platform == 'win32':
            self.skipTest("Cannot test SGE on Windows")
        sge = FakeSGEEnvironment()
        s = IMP.parallel._SGEPESlave("testnode")
        x = repr(s)
        s._start("testpath", "testid", "testoutput")
        self.assertSGEOutputEqual(sge, 'qrsh',
                                  '-inherit -V testnode testpath testid')
        os.unlink('testoutput')

    def test_pe_slave_array(self):
        """Test SGEPESlaveArray class"""
        hostfile = 'hostfile'
        open(hostfile, 'w').write('node1 1 long.q\nnode2 1 long.q\n'
                                  'node3 2 short.q\n')
        os.environ['PE_HOSTFILE'] = hostfile

        s = IMP.parallel.SGEPESlaveArray()
        slaves = s._get_slaves()
        self.assertEqual(len(slaves), 4)
        self.assertEqual(type(slaves[0]), IMP.parallel.LocalSlave)
        for host, slave in zip(('node2', 'node3', 'node3'), slaves[1:]):
            self.assertEqual(slave._host, host)
        os.unlink(hostfile)


if __name__ == '__main__':
    IMP.test.main()
