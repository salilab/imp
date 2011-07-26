import subprocess
from IMP.parallel.slave import Slave

class LocalSlave(Slave):
    """A slave running on the same machine"""

    def _start(self, command, unique_id, output):
        Slave._start(self, command, unique_id, output)
        cmdline = "%s %s" % (command, unique_id)
        print "%s > %s" % (cmdline, output)
        fp = open(output, 'w')
        subprocess.Popen(cmdline, shell=True, stdout=fp,
                         stderr=subprocess.STDOUT)

    def __repr__(self):
        return "<LocalSlave>"
