import socket
from IMP.parallel.communicator import Communicator

class MasterCommunicator(Communicator):
    connect_timeout = 600

    def __init__(self, master_addr, lock):
        Communicator.__init__(self)
        self._master_addr = master_addr
        self._connect_to_master()
        self._lock = lock

    def _connect_to_master(self):
        host, port, identifier = self._master_addr
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(self.connect_timeout)
        s.connect((host, port))
        s.sendall(identifier)
        s.settimeout(None)
        self._socket = s

    def _send(self, obj):
        # Since the slave is multi-threaded, must lock all access
        self._lock.acquire()
        try:
            Communicator._send(self, obj)
        finally:
            self._lock.release()
