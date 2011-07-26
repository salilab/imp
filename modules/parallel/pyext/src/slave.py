import time
from IMP.parallel import slavestate
from IMP.parallel.communicator import Communicator, _ContextWrapper
from IMP.parallel.communicator import _TaskWrapper, _HeartBeat

class Slave(Communicator):
    """Representation of a single slave, on the master"""

    def __init__(self):
        Communicator.__init__(self)
        self._state = slavestate.init
        self._context = None
        self._task = None
        self.update_contact_time()

    def _start(self, command, unique_id, output):
        """Start the slave running on the remote host; override in subclasses"""
        self._state = slavestate.started

    def _accept_connection(self, sock):
        self._socket = sock
        self._state = slavestate.connected
        self.update_contact_time()

    def update_contact_time(self):
        self.last_contact_time = time.time()

    def get_contact_timed_out(self, timeout):
        return (time.time() - self.last_contact_time) > timeout

    def _start_task(self, task, context):
        if not self.ready_for_task(context) and not self.ready_for_task(None):
            raise TypeError("%s not ready for task" % str(self))
        if self._context != context:
            self._context = context
            self._send(_ContextWrapper(context._startup))
        self._state = slavestate.running_task
        self._task = task
        self._send(_TaskWrapper(task))

    def _get_finished_task(self):
        while True:
            r = self._recv()
            self.update_contact_time()
            if isinstance(r, _HeartBeat):
                if not self.get_data_pending():
                    return None
            else:
                break
        task = self._task
        task._results = r
        self._task = None
        self._state = slavestate.connected
        return task

    def _kill(self):
        task = self._task
        self._task = None
        self._context = None
        self._state = slavestate.dead
        return task

    def ready_to_start(self):
        return self._state == slavestate.init

    def ready_for_task(self, context):
        return self._state == slavestate.connected \
               and self._context == context

    def running_task(self, context):
        return self._state == slavestate.running_task \
               and self._context == context
