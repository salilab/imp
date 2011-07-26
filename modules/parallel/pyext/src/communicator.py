from IMP.parallel.errors import *
import socket
import xdrlib
try:
    import cPickle as pickle
except ImportError:
    import pickle


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


class Communicator(object):
    """Simple support for sending Python pickled objects over the network"""

    def __init__(self):
        self._socket = None
        self._ibuffer = ''

    def _send(self, obj):
        p = xdrlib.Packer()
        try:
            p.pack_string(pickle.dumps(obj, -1))
        # Python < 2.5 can fail trying to send Inf or NaN floats in binary
        # mode, so fall back to the old protocol in this case:
        except SystemError:
            p.pack_string(pickle.dumps(obj, 0))
        self._socket.sendall(p.get_buffer())

    def get_data_pending(self):
        return len(self._ibuffer) > 0

    def _recv(self):
        while True:
            try:
                obj, self._ibuffer = self._unpickle(self._ibuffer)
                if isinstance(obj, _ErrorWrapper):
                    raise RemoteError(obj.obj, obj.traceback, self)
                else:
                    return obj
            except (IndexError, EOFError):
                try:
                    data = self._socket.recv(4096)
                except socket.error, detail:
                    raise NetworkError("Connection lost to %s: %s" \
                                       % (str(self), str(detail)))
                if len(data) > 0:
                    self._ibuffer += data
                else:
                    raise NetworkError("%s closed connection" % str(self))

    def _unpickle(self, ibuffer):
        p = xdrlib.Unpacker(ibuffer)
        obj = p.unpack_string()
        return (pickle.loads(obj), ibuffer[p.get_position():])
