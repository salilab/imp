import socket
import select
import sys
import random
from IMP.parallel.context import Context
from IMP.parallel.errors import _NetworkError
from IMP.parallel.util import _ListenSocket

class Manager(object):
    """Manages slaves and contexts"""
    connect_timeout = 7200

    # Note: must be higher than that in slave_handler._HeartBeatThread
    heartbeat_timeout = 7200

    def __init__(self, python=None, host=None):
        if python is None:
            self._python = sys.executable
        else:
            self._python = python
        self._host = host
        self._all_slaves = []
        self._starting_slaves = {}
        self._slave_arrays = []
        if host:
            self._host = host
        else:
            # Get primary IP address of this machine
            self._host = socket.gethostbyname_ex(socket.gethostname())[-1][0]
        self._listen_sock = _ListenSocket(self._host, self.connect_timeout)

    def add_slave(self, slave):
        if hasattr(slave, '_get_slaves'):
            self._slave_arrays.append(slave)
        else:
            self._all_slaves.append(slave)

    def get_context(self, startup=None):
        return Context(self, startup)

    def _get_results_unordered(self, context):
        """Run all of a context's tasks, and yield results"""
        self._send_tasks_to_slaves(context)
        try:
            while True:
                for task in self._get_finished_tasks(context):
                    yield task._results
        except _NoMoreTasksError:
            return

    def _start_all_slaves(self):
        for array in self._slave_arrays:
            self._all_slaves.extend(array._get_slaves())

        command = "%s -m IMP.parallel.slave_handler %s %d" \
                  % (self._python, self._host, self._listen_sock.port)

        for (num, slave) in enumerate(self._all_slaves):
            if slave.ready_to_start():
                unique_id = self._get_unique_id(num)
                self._starting_slaves[unique_id] = slave
                output = 'slave%d.output' % num
                slave._start(command, unique_id, output)

        for array in self._slave_arrays:
            array._start(command)
        self._slave_arrays = []

    def _get_unique_id(self, num):
        id = "%d:" % num
        for i in range(0, 8):
            id += chr(random.randint(0, 25) + ord('A'))
        return id

    def _send_tasks_to_slaves(self, context):
        self._start_all_slaves()
        # Prefer slaves that already have the task context
        available_slaves = [a for a in self._all_slaves
                            if a.ready_for_task(context)] + \
                           [a for a in self._all_slaves
                            if a.ready_for_task(None)]
        for slave in available_slaves:
            if len(context._tasks) == 0:
                break
            else:
                self._send_task_to_slave(self, slave, context)

    def _send_task_to_slave(self, slave, context):
        if len(context._tasks) == 0:
            return
        t = context._tasks[0]
        try:
            slave._start_task(t, context)
            context._tasks.pop(0)
        except socket.error, detail:
            slave._kill()

    def _get_finished_tasks(self, context):
        while True:
            events = self._get_network_events(context)
            if len(events) == 0:
                self._kill_timed_out_slaves(context)
            for event in events:
                task = self._process_event(event, context)
                if task:
                    yield task

    def _process_event(self, event, context):
        if event == self._listen_sock:
            # New slave just connected
            (conn, addr) = self._listen_sock.accept()
            new_slave = self._accept_slave(conn, context)
        elif event.running_task(context):
            try:
                task = event._get_finished_task()
                if task:
                    self._send_task_to_slave(event, context)
                    return task
                else: # the slave sent back a heartbeat
                    self._kill_timed_out_slaves(context)
            except _NetworkError, detail:
                task = event._kill()
                print "Slave %s failed (%s): rescheduling task %s" \
                      % (str(event), str(detail), str(task))
                context._tasks.append(task)
                self._send_tasks_to_slaves(context)
        else:
            pass # Slave not running a task

    def _kill_timed_out_slaves(self, context):
        timed_out = [a for a in self._all_slaves if a.running_task() and \
                     a.get_contact_timed_out(self.heartbeat_timeout)]
        for slave in timed_out:
            print("Did not hear from slave %s in %d seconds; rescheduling "
                  "task %s" % (str(slave), str(task)))
            task = slave._kill()
            context._tasks.append(task)
        if len(timed_out) > 0:
            self._send_tasks_to_slaves(context)

    def _accept_slave(self, sock, context):
        sock.setblocking(True)
        identifier = sock.recv(1024)
        if identifier and identifier in self._starting_slaves:
            slave = self._starting_slaves.pop(identifier)
            slave._accept_connection(sock)
            print "Identified slave %s " % str(slave)
            self._send_task_to_slave(slave, context)
            return slave
        else:
            print "Ignoring request from unknown slave"

    def _get_network_events(self, context):
        fileno = self._listen_sock.fileno()
        slavemap = { fileno: self._listen_sock }
        p = select.poll()
        p.register(fileno, select.POLLIN)
        running = [a for a in self._all_slaves if a.running_task(context)]
        if len(running) == 0:
            if len(context._tasks) == 0:
                raise _NoMoreTasksError()
            elif len(self._starting_slaves) == 0:
                raise NoMoreSlavesError("Ran out of slaves to run tasks")
            # Otherwise, wait for starting slaves to connect back and get tasks

        for slave in running:
            fileno = slave._socket.fileno()
            slavemap[fileno] = slave
            p.register(fileno, select.POLLIN)
        ready = p.poll(self.heartbeat_timeout * 1000)

        if len(ready) == 0:
            return []
        else:
            return [slavemap[fd[0]] for fd in ready]
