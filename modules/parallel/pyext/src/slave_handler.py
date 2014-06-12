import sys
import traceback
import socket
import threading
from IMP.parallel import NetworkError
from IMP.parallel.master_communicator import MasterCommunicator
from IMP.parallel.util import _TaskWrapper, _ContextWrapper
from IMP.parallel.util import _ErrorWrapper, _HeartBeat, _SlaveAction


class _HeartBeatThread(threading.Thread):

    """Periodically send a 'heartbeat' back to the master, so that it can
       distinguish between failed nodes and long calculations"""
    timeout = 300

    def __init__(self, master):
        threading.Thread.__init__(self)
        self._master = master
        self._event = threading.Event()

    def cancel(self):
        """Stop the heartbeat"""
        self._event.set()

    def run(self):
        while True:
            self._event.wait(self.timeout)
            if self._event.isSet():
                break
            else:
                self._master._send(_HeartBeat())


class SlaveHandler(object):

    def __init__(self, master_addr):
        self._master_addr = master_addr

    def run(self):
        print "Connect back to master at %s:%d with ID %s" \
              % tuple(self._master_addr)
        lock = threading.Lock()
        master = MasterCommunicator(self._master_addr, lock)
        hb = _HeartBeatThread(master)
        hb.start()
        try:
            self._handle_network_io(master)
        finally:
            hb.cancel()

    def _send_exception_to_master(self, master, exc):
        try:
            exc_type, exc_value, tb = sys.exc_info()
            master._send(_ErrorWrapper(exc,
                                       traceback.format_exception(exc_type, exc_value, tb)))
        except socket.error:
            # ignore errors encountered while trying to send error to master
            pass

    def _handle_network_io(self, master):
        setup_args = ()
        while True:
            try:
                obj = master._recv()
            except NetworkError:
                break
            try:
                if isinstance(obj, _ContextWrapper):
                    if obj.obj is None:
                        setup_args = ()
                    else:
                        setup_args = obj.obj()
                elif isinstance(obj, _TaskWrapper):
                    master._send(obj.obj(*setup_args))
                elif isinstance(obj, _SlaveAction):
                    obj.execute()
            except NetworkError:
                raise
            except Exception, detail:
                # Send all other exceptions back to the master and reraise
                self._send_exception_to_master(master, detail)
                raise


def main():
    h = SlaveHandler([sys.argv[-3], int(sys.argv[-2]), sys.argv[-1]])
    h.run()

if __name__ == '__main__':
    main()
