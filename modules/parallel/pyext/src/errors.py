class NetworkError(Exception):
    pass

class RemoteError(Exception):
    def __init__(self, exc, traceback, slave):
        self.exc = exc
        self.traceback = traceback
        self.slave = slave

    def __str__(self):
        errstr = str(self.exc.__class__).replace("exceptions.", "")
        return "%s: %s from %s\nRemote traceback:\n%s" \
               % (errstr, str(self.exc), str(self.slave), self.traceback)
