"""@namespace IMP.parallel.manager_communicator
   Classes for communicating from the manager to workers."""

import socket
from IMP.parallel import _Communicator


class ManagerCommunicator(_Communicator):

    """For communicating from the manager to workers."""

    connect_timeout = 600

    def __init__(self, manager_addr, lock):
        _Communicator.__init__(self)
        self._manager_addr = manager_addr
        self._connect_to_manager()
        self._lock = lock

    def _connect_to_manager(self):
        host, port, identifier = self._manager_addr
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(self.connect_timeout)
        s.connect((host, port))
        s.sendall(identifier.encode('ascii'))
        s.settimeout(None)
        self._socket = s

    def _send(self, obj):
        # Since the worker is multi-threaded, must lock all access
        self._lock.acquire()
        try:
            _Communicator._send(self, obj)
        finally:
            self._lock.release()
