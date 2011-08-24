import socket
import sys
import random

class _ListenSocket(socket.socket):
    def __init__(self, host, timeout):
        socket.socket.__init__(self, socket.AF_INET, socket.SOCK_STREAM)
        self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.port = self._bind_to_random_port(host, timeout)
        self.settimeout(timeout)
        self.listen(15)

    def _bind_to_random_port(self, host, timeout):
        """Bind to a random high-numbered port"""
        tries = 0
        while True:
            port = random.randint(10000, 60000)
            try:
                self.bind((host, port))
            # gaierror is a subclass of error, so catch it separately
            except socket.gaierror:
                raise
            except socket.error:
                tries += 1
                if tries > 10: raise
            else:
                break
        return port


class _ContextWrapper(object):
    def __init__(self, obj):
        self.obj = obj


class _TaskWrapper(object):
    def __init__(self, obj):
        self.obj = obj


class _ErrorWrapper(object):
    def __init__(self, obj, traceback):
        self.obj = obj
        self.traceback = traceback


class _HeartBeat(object):
    pass


class _SlaveAction(object):
    pass


class _SetPathAction(_SlaveAction):
    def __init__(self, path):
        self.path = path
    def execute(self):
        sys.path.insert(0, self.path)
