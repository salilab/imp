import os
import sys
import time
import IMP.parallel

# Set a shorter default heartbeat timeout, so failures to start up slaves
# don't hang the tests for a very long time


class Manager(IMP.parallel.Manager):
    heartbeat_timeout = 5.0

# On Windows sytems, we cannot delete a file if another process has it open.
if sys.platform == 'win32':
    def unlink(filename):
        pass

else:
    def unlink(filename):
        os.unlink(filename)
