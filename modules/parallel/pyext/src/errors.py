class Error(Exception):
    """Base class for all errors specific to the IMP.parallel module"""
    pass

class _NoMoreTasksError(Error):
    pass

class NoMoreSlavesError(Error):
    """Error raised if all slaves failed, so tasks cannot be run"""
    pass

class _NetworkError(Error):
    pass

class RemoteError(Error):
    """Error raised if a slave has an unhandled exception"""
    def __init__(self, exc, traceback, slave):
        self.exc = exc
        self.traceback = traceback
        self.slave = slave

    def __str__(self):
        errstr = str(self.exc.__class__).replace("exceptions.", "")
        return "%s: %s from %s\nRemote traceback:\n%s" \
               % (errstr, str(self.exc), str(self.slave), self.traceback)
