from __future__ import print_function
import sys
import traceback
import socket
import threading
from IMP.parallel import NetworkError
from IMP.parallel.manager_communicator import ManagerCommunicator
from IMP.parallel.util import _TaskWrapper, _ContextWrapper
from IMP.parallel.util import _ErrorWrapper, _HeartBeat, _WorkerAction


class _HeartBeatThread(threading.Thread):

    """Periodically send a 'heartbeat' back to the manager, so that it can
       distinguish between failed nodes and long calculations"""
    timeout = 300

    def __init__(self, manager):
        threading.Thread.__init__(self)
        self._manager = manager
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
                self._manager._send(_HeartBeat())


class WorkerHandler(object):

    def __init__(self, manager_addr):
        self._manager_addr = manager_addr

    def run(self):
        print("Connect back to manager at %s:%d with ID %s"
              % tuple(self._manager_addr))
        lock = threading.Lock()
        manager = ManagerCommunicator(self._manager_addr, lock)
        hb = _HeartBeatThread(manager)
        hb.start()
        try:
            self._handle_network_io(manager)
        finally:
            hb.cancel()

    def _send_exception_to_manager(self, manager, exc):
        try:
            exc_type, exc_value, tb = sys.exc_info()
            manager._send(_ErrorWrapper(
                exc, traceback.format_exception(exc_type, exc_value, tb)))
        except socket.error:
            # ignore errors encountered while trying to send error to manager
            pass

    def _handle_network_io(self, manager):
        setup_args = ()
        while True:
            try:
                obj = manager._recv()
            except NetworkError:
                break
            try:
                if isinstance(obj, _ContextWrapper):
                    if obj.obj is None:
                        setup_args = ()
                    else:
                        setup_args = obj.obj()
                elif isinstance(obj, _TaskWrapper):
                    manager._send(obj.obj(*setup_args))
                elif isinstance(obj, _WorkerAction):
                    obj.execute()
            except NetworkError:
                raise
            except Exception as detail:
                # Send all other exceptions back to the manager and reraise
                self._send_exception_to_manager(manager, detail)
                raise


def main():
    h = WorkerHandler([sys.argv[-3], int(sys.argv[-2]), sys.argv[-1]])
    h.run()


if __name__ == '__main__':
    main()
