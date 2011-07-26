import sys
import os
import re
import subprocess
from IMP.parallel.slave import Slave
from IMP.parallel.local_slave import LocalSlave
from IMP.parallel.slave_array import SlaveArray

class _Popen4(subprocess.Popen):
    """Utility class to provide a portable way to spawn a child process and
       communicate with its stdin and combined stdout/stderr."""

    def __init__(self, cmd):
        # shell isn't needed on Win32, and may not be found under wine anyway
        shell = (sys.platform != "win32")
        subprocess.Popen.__init__(self, cmd, shell=shell, stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT)

    def require_clean_exit(self):
        """Make sure the child exited with a zero return code"""
        r = self.wait()
        if r != 0:
            raise IOError("Process failed with exit status %d" % r)


class _SGEQsubSlave(Slave):
    def __init__(self, array):
        Slave.__init__(self)
        self._jobid = None
        self._array = array

    def _start(self, command, unique_id, output):
        Slave._start(self, command, unique_id, output)
        self._array._slave_started(unique_id, output)

    def __repr__(self):
        jobid = self._jobid
        if jobid is None:
            jobid = '(unknown)'
        return "<SGE qsub slave, ID %s>" % jobid


class SGEQsubSlaveArray(SlaveArray):
    standard_options = '-j y -cwd -r n -o sge-errors'

    def __init__(self, numslave, options):
        self._numslave = numslave
        self._options = options
        self._starting_slaves = []
        self._jobid = None

    def _get_slaves(self):
        """Return a list of Slave objects contained within this array"""
        return [_SGEQsubSlave(self) for x in range(self._numslave)]

    def _slave_started(self, command, output):
        self._starting_slaves.append((command, output))

    def _start(self, command):
        qsub = "qsub -S /bin/sh %s %s -t 1-%d" % \
               (self._options, self.standard_options,
                len(self._starting_slaves))
        print qsub
        a = _Popen4(qsub)
        (inp, out) = (a.stdin, a.stdout)
        slave_uid = " ".join([repr(s[0]) for s in self._starting_slaves])
        slave_out = " ".join([repr(s[1]) for s in self._starting_slaves])
        inp.write("#!/bin/sh\n")
        inp.write("uid=( '' %s )\n" % slave_uid)
        inp.write("out=( '' %s )\n" % slave_out)
        inp.write("myuid=${uid[$SGE_TASK_ID]}\n")
        inp.write("myout=${out[$SGE_TASK_ID]}\n")
        inp.write("%s $myuid > $myout 2>&1\n" % command)
        inp.close()
        outlines = out.readlines()
        out.close()
        for line in outlines:
            print line.rstrip('\r\n')
        a.require_clean_exit()
        self._set_jobid(outlines)
        self._starting_slaves = []

    def _set_jobid(self, outlines):
        """Try to figure out the job ID from the SGE qsub output"""
        if len(outlines) > 0:
            m = re.compile(r"\d+").search(outlines[0])
            if m:
                self._jobid = int(m.group())
                for (num, slave) in enumerate(self._starting_slaves):
                    slave._jobid = "%d.%d" % (self._jobid, num+1)


class _SGEPESlave(Slave):
    def __init__(self, host):
        Slave.__init__(self)
        self._host = host

    def _start(self, command, unique_id, output):
        Slave._start(self, command, unique_id, output)
        cmdline = "qrsh -inherit %s %s %s" % (self._host, command, unique_id)
        print "%s > %s" % (cmdline, output)
        fp = open(output, 'w')
        subprocess.Popen(cmdline, shell=True, stdout=fp,
                         stderr=subprocess.STDOUT)

    def __repr__(self):
        return "<SGE PE slave on %s>" % self._host


class SGEPESlaveArray(SlaveArray):

    def _get_slaves(self):
        slaves = []

        pe = os.environ['PE_HOSTFILE']
        fh = open(pe, "r")
        while True:
            line = fh.readline()
            if line == '':
                break
            (node, num, queue) = line.split(None, 2)
            for i in range(int(num)):
                slaves.append(SGEPESlave(node))
        # Replace first slave with a local slave, as this is ourself, and SGE
        # won't let us start this process with qrsh (as we are already
        # occupying the slot)
        if len(slaves) > 0:
            slaves[0] = LocalSlave()
        return slaves
