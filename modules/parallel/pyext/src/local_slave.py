from IMP.parallel.slave import Slave
from IMP.parallel.subproc import _run_background

class LocalSlave(Slave):
    """A slave running on the same machine"""

    def _start(self, command, unique_id, output):
        Slave._start(self, command, unique_id, output)
        cmdline = "%s %s" % (command, unique_id)
        _run_background(cmdline, output)

    def __repr__(self):
        return "<LocalSlave>"
