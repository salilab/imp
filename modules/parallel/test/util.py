import os
import sys
import time
import IMP.parallel

# Set a shorter default heartbeat timeout, so failures to start up slaves
# don't hang the tests for a very long time
class Manager(IMP.parallel.Manager):
    heartbeat_timeout = 1.0

# On Windows sytems, we cannot delete a file if another process has it open.
# This leads to occasional failures due to a race between the master and the
# slave. Work around this by catching the exception and retrying.
if sys.platform == 'win32':
    def unlink(filename):
        for i in range(10):
            try:
                os.unlink(filename)
                return
            except WindowsError:
                time.sleep(0.2)
        os.unlink(filename)

else:
    def unlink(filename):
        os.unlink(filename)
