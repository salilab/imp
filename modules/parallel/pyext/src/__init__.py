import socket
import time
import sys
import os
import re
import random
import socket
import xdrlib
try:
    import cPickle as pickle
except ImportError:
    import pickle
import IMP
from IMP.parallel import workerstate
from IMP.parallel.subproc import _run_background, _Popen4
from IMP.parallel.util import _ListenSocket, _ErrorWrapper
from IMP.parallel.util import _TaskWrapper, _HeartBeat, _ContextWrapper
from IMP.parallel.util import _SetPathAction

# Save sys.path at import time, so that workers can import using the same
# path that works for the manager imports
_import_time_path = sys.path[:]

class Error(Exception):
    """Base class for all errors specific to the parallel module"""
    pass


class _NoMoreTasksError(Error):
    pass


class NoMoreWorkersError(Error):
    """Error raised if all workers failed, so tasks cannot be run"""
    pass


@IMP.deprecated_object("2.14", "Use NoMoreWorkersError instead")
class NoMoreSlavesError(NoMoreWorkersError):
    pass


class NetworkError(Error):
    """Error raised if a problem occurs with the network"""
    pass


class RemoteError(Error):
    """Error raised if a worker has an unhandled exception"""
    def __init__(self, exc, traceback, worker):
        self.exc = exc
        self.traceback = traceback
        self.worker = worker

    def __str__(self):
        errstr = str(self.exc.__class__).replace("exceptions.", "")
        return "%s: %s from %s\nRemote traceback:\n%s" \
               % (errstr, str(self.exc), str(self.worker), self.traceback)

class _Communicator(object):
    """Simple support for sending Python pickled objects over the network"""

    def __init__(self):
        self._socket = None
        self._ibuffer = b''

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
                except socket.error as detail:
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


class Worker(_Communicator):
    """Representation of a single worker.
       Each worker uses a single thread of execution (i.e. a single CPU core)
       to run tasks sequentially.
       Worker is an abstract class; instead of using this class directly, use
       a subclass such as LocalWorker or SGEQsubWorkerArray."""

    def __init__(self):
        _Communicator.__init__(self)
        self._state = workerstate.init
        self._context = None
        self._task = None
        self.update_contact_time()

    def _start(self, command, unique_id, output):
        """Start the worker running on the remote host; override in
           subclasses"""
        self._state = workerstate.started

    def _accept_connection(self, sock):
        self._socket = sock
        self._state = workerstate.connected
        self.update_contact_time()

    def _set_python_search_path(self, path):
        self._send(_SetPathAction(path))

    def update_contact_time(self):
        self.last_contact_time = time.time()

    def get_contact_timed_out(self, timeout):
        return (time.time() - self.last_contact_time) > timeout

    def _start_task(self, task, context):
        if not self._ready_for_task(context) and not self._ready_for_task(None):
            raise TypeError("%s not ready for task" % str(self))
        if self._context != context:
            self._context = context
            self._send(_ContextWrapper(context._startup))
        self._state = workerstate.running_task
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
        self._state = workerstate.connected
        return task

    def _kill(self):
        task = self._task
        self._task = None
        self._context = None
        self._state = workerstate.dead
        return task

    def _ready_to_start(self):
        return self._state == workerstate.init

    def _ready_for_task(self, context):
        return self._state == workerstate.connected \
               and self._context == context

    def _running_task(self, context):
        return self._state == workerstate.running_task \
               and self._context == context


@IMP.deprecated_object("2.14", "Use Worker instead")
class Slave(Worker):
    pass


class WorkerArray(object):
    """Representation of an array of workers.
       This is similar to Worker, except that it represents a collection of
       workers that are controlled together, such as a batch submission system
       array job on a compute cluster.
       Worker is an abstract class; instead of using this class directly, use
       a subclass such as SGEQsubWorkerArray."""

    def _get_workers(self):
        """Return a list of Worker objects contained within this array"""
        pass

    def _start(self):
        """Do any necessary startup after all contained Workers have started"""
        pass

@IMP.deprecated_object("2.14", "Use WorkerArray instead")
class SlaveArray(WorkerArray):
    pass


class LocalWorker(Worker):
    """A worker running on the same machine as the manager."""

    def _start(self, command, unique_id, output):
        Worker._start(self, command, unique_id, output)
        cmdline = "%s %s" % (command, unique_id)
        _run_background(cmdline, output)

    def __repr__(self):
        return "<LocalWorker>"


@IMP.deprecated_object("2.14", "Use LocalWorker instead")
class LocalSlave(LocalWorker):
    pass


class _SGEQsubWorker(Worker):
    def __init__(self, array):
        Worker.__init__(self)
        self._jobid = None
        self._array = array

    def _start(self, command, unique_id, output):
        Worker._start(self, command, unique_id, output)
        self._array._worker_started(unique_id, output, self)

    def __repr__(self):
        jobid = self._jobid
        if jobid is None:
            jobid = '(unknown)'
        return "<SGE qsub worker, ID %s>" % jobid


class SGEQsubWorkerArray(WorkerArray):
    """An array of workers on a Sun Grid Engine system, started with 'qsub'.
       To use this class, the manager process must be running on a machine that
       can submit Sun Grid Engine (SGE) jobs using the 'qsub' command (this
       is termed a 'submit host' by SGE). The class starts an SGE job array
       (every worker has the same SGE job ID, but a different task ID).
    """


    standard_options = '-j y -cwd -r n -o sge-errors'

    def __init__(self, numworker, options):
        """Constructor.
           @param numworker The number of workers, which corresponds to the
                           number of tasks in the SGE job.
           @param options  A string of SGE options that are passed on the 'qsub'
                           command line. This is added to standard_options.
        """
        self._numworker = numworker
        self._options = options
        self._starting_workers = []
        self._jobid = None

    def _get_workers(self):
        """Return a list of Worker objects contained within this array"""
        return [_SGEQsubWorker(self) for x in range(self._numworker)]

    def _worker_started(self, command, output, worker):
        self._starting_workers.append((command, output, worker))

    def _start(self, command):
        qsub = "qsub -S /bin/sh %s %s -t 1-%d" % \
               (self._options, self.standard_options,
                len(self._starting_workers))
        print(qsub)
        a = _Popen4(qsub)
        (inp, out) = (a.stdin, a.stdout)
        worker_uid = " ".join([repr(s[0]) for s in self._starting_workers])
        worker_out = " ".join([repr(s[1]) for s in self._starting_workers])
        inp.write("#!/bin/sh\n")
        inp.write("uid=( '' %s )\n" % worker_uid)
        inp.write("out=( '' %s )\n" % worker_out)
        inp.write("myuid=${uid[$SGE_TASK_ID]}\n")
        inp.write("myout=${out[$SGE_TASK_ID]}\n")
        inp.write("%s $myuid > $myout 2>&1\n" % command)
        inp.close()
        outlines = out.readlines()
        out.close()
        for line in outlines:
            print(line.rstrip('\r\n'))
        a.require_clean_exit()
        self._set_jobid(outlines)
        self._starting_workers = []

    def _set_jobid(self, outlines):
        """Try to figure out the job ID from the SGE qsub output"""
        if len(outlines) > 0:
            m = re.compile(r"\d+").search(outlines[0])
            if m:
                self._jobid = int(m.group())
                for (num, worker) in enumerate(self._starting_workers):
                    worker[2]._jobid = "%d.%d" % (self._jobid, num+1)


@IMP.deprecated_object("2.14", "Use SGEQsubWorkerArray instead")
class SGEQsubSlaveArray(SGEQsubWorkerArray):
    pass


class _SGEPEWorker(Worker):
    def __init__(self, host):
        Worker.__init__(self)
        self._host = host

    def _start(self, command, unique_id, output):
        Worker._start(self, command, unique_id, output)
        cmdline = "qrsh -inherit -V %s %s %s" % (self._host, command, unique_id)
        _run_background(cmdline, output)

    def __repr__(self):
        return "<SGE PE worker on %s>" % self._host


class SGEPEWorkerArray(WorkerArray):
    """An array of workers in a Sun Grid Engine system parallel environment.
       In order to use this class, the manager must be run via Sun Grid Engine's
       'qsub' command and submitted to a parallel environment using the qsub
       -pe option. This class will start workers on every node in the parallel
       environment (including the node running the manager). Each worker is
       started using the 'qrsh' command with the '-inherit' option."""

    def _get_workers(self):
        workers = []

        pe = os.environ['PE_HOSTFILE']
        fh = open(pe, "r")
        while True:
            line = fh.readline()
            if line == '':
                break
            (node, num, queue) = line.split(None, 2)
            for i in range(int(num)):
                workers.append(_SGEPEWorker(node))
        # Replace first worker with a local worker, as this is ourself, and SGE
        # won't let us start this process with qrsh (as we are already
        # occupying the slot)
        if len(workers) > 0:
            workers[0] = LocalWorker()
        return workers


@IMP.deprecated_object("2.14", "Use SGEPEWorkerArray instead")
class SGEPESlaveArray(SGEPEWorkerArray):
    pass


class Context(object):
    """A collection of tasks that run in the same environment.
       Context objects are typically created by calling Manager::get_context().
    """
    def __init__(self, manager, startup=None):
        """Constructor."""
        self._manager = manager
        self._startup = startup
        self._tasks = []

    def add_task(self, task):
        """Add a task to this context.
           Tasks are any Python callable object that can be pickled (e.g. a
           function or a class that implements the \_\_call\_\_ method). When
           the task is run on the worker its arguments are the return value
           from this context's startup function."""
        self._tasks.append(task)

    def get_results_unordered(self):
        """Run all of the tasks on available workers, and return results.
           If there are more tasks than workers, subsequent tasks are
           started only once a running task completes: each worker only runs
           a single task at a time. As each task completes, the return value(s)
           from the task callable are returned from this method, as a
           Python generator. Note that the results are returned in the order
           that tasks complete, which may not be the same as the order they
           were submitted in.

           @exception NoMoreWorkersError there are no workers available
                      to run the tasks (or they all failed during execution).
           @exception RemoteError a worker encountered an unhandled exception.
           @exception NetworkError the manager lost (or was unable to
                      establish) communication with any worker.
        """
        return self._manager._get_results_unordered(self)


class Manager(object):
    """Manages workers and contexts.
    """

    connect_timeout = 7200

    # Note: must be higher than that in worker_handler._HeartBeatThread
    heartbeat_timeout = 7200

    def __init__(self, python=None, host=None, output='worker%d.output'):
        """Constructor.
           @param python If not None, the command to run to start a Python
                         interpreter that can import the IMP module. Otherwise,
                         the same interpreter that the manager is currently
                         using is used. This is passed to the shell, so a full
                         command line (including multiple words separated by
                         spaces) can be used if necessary.
           @param host   The hostname that workers use to connect back to the
                         manager. If not specified, the manager machine's
                         primary IP address is used. On multi-homed machines,
                         such as compute cluster headnodes, this may need to be
                         changed to allow all workers to reach the manager
                         (typically the name of the machine's internal network
                         address is needed). If only running local workers,
                         'localhost' can be used to prohibit connections
                         across the network.
           @param output A format string used to name worker output files. It is
                         given the numeric worker id, so for example the default
                         value 'worker\%d.output' will yield output files called
                         worker0.output, worker1.output, etc.
        """
        if python is None:
            self._python = sys.executable
        else:
            self._python = python
        self._host = host
        self._output = output
        self._all_workers = []
        self._starting_workers = {}
        self._worker_arrays = []
        if host:
            self._host = host
        else:
            # Get primary IP address of this machine
            self._host = socket.gethostbyname_ex(socket.gethostname())[-1][0]
        self._listen_sock = _ListenSocket(self._host, self.connect_timeout)

    def add_worker(self, worker):
        """Add a Worker object."""
        if hasattr(worker, '_get_workers'):
            self._worker_arrays.append(worker)
        else:
            self._all_workers.append(worker)

    def get_context(self, startup=None):
        """Create and return a new Context in which tasks can be run.
           @param startup If not None, a callable (Python function or class
                          that implements the \_\_call\_\_ method) that sets up
                          the worker to run tasks. This method is only called
                          once per worker. The return values from this method
                          are passed to the task object when it runs on
                          the worker.
           @return A new Context object.
        """
        return Context(self, startup)

    def _get_results_unordered(self, context):
        """Run all of a context's tasks, and yield results"""
        self._send_tasks_to_workers(context)
        try:
            while True:
                for task in self._get_finished_tasks(context):
                    tasks_queued = len(context._tasks)
                    yield task._results
                    # If the user added more tasks while processing these
                    # results, make sure they get sent off to the workers
                    if len(context._tasks) > tasks_queued:
                        self._send_tasks_to_workers(context)
        except _NoMoreTasksError:
            return

    def _start_all_workers(self):
        for array in self._worker_arrays:
            self._all_workers.extend(array._get_workers())

        command = ("%s -c \"import IMP.parallel.worker_handler as s; s.main()\""
                   " %s %d") % (self._python, self._host,
                                self._listen_sock.port)

        for (num, worker) in enumerate(self._all_workers):
            if worker._ready_to_start():
                unique_id = self._get_unique_id(num)
                self._starting_workers[unique_id] = worker
                worker._start(command, unique_id, self._output % num)

        for array in self._worker_arrays:
            array._start(command)
        self._worker_arrays = []

    def _get_unique_id(self, num):
        id = "%d:" % num
        for i in range(0, 8):
            id += chr(random.randint(0, 25) + ord('A'))
        return id

    def _send_tasks_to_workers(self, context):
        self._start_all_workers()
        # Prefer workers that already have the task context
        available_workers = [a for a in self._all_workers
                            if a._ready_for_task(context)] + \
                           [a for a in self._all_workers
                            if a._ready_for_task(None)]
        for worker in available_workers:
            if len(context._tasks) == 0:
                break
            else:
                self._send_task_to_worker(worker, context)

    def _send_task_to_worker(self, worker, context):
        if len(context._tasks) == 0:
            return
        t = context._tasks[0]
        try:
            worker._start_task(t, context)
            context._tasks.pop(0)
        except socket.error as detail:
            worker._kill()

    def _get_finished_tasks(self, context):
        while True:
            events = self._get_network_events(context)
            if len(events) == 0:
                self._kill_all_running_workers(context)
            for event in events:
                task = self._process_event(event, context)
                if task:
                    yield task

    def _process_event(self, event, context):
        if event == self._listen_sock:
            # New worker just connected
            (conn, addr) = self._listen_sock.accept()
            new_worker = self._accept_worker(conn, context)
        elif event._running_task(context):
            try:
                task = event._get_finished_task()
                if task:
                    self._send_task_to_worker(event, context)
                    return task
                else: # the worker sent back a heartbeat
                    self._kill_timed_out_workers(context)
            except NetworkError as detail:
                task = event._kill()
                print("Worker %s failed (%s): rescheduling task %s" \
                      % (str(event), str(detail), str(task)))
                context._tasks.append(task)
                self._send_tasks_to_workers(context)
        else:
            pass # Worker not running a task

    def _kill_timed_out_workers(self, context):
        timed_out = [a for a in self._all_workers if a._running_task(context) \
                     and a.get_contact_timed_out(self.heartbeat_timeout)]
        for worker in timed_out:
            task = worker._kill()
            print("Did not hear from worker %s in %d seconds; rescheduling "
                  "task %s" % (str(worker), self.heartbeat_timeout, str(task)))
            context._tasks.append(task)
        if len(timed_out) > 0:
            self._send_tasks_to_workers(context)

    def _kill_all_running_workers(self, context):
        running = [a for a in self._all_workers if a._running_task(context)]
        for worker in running:
            task = worker._kill()
            context._tasks.append(task)
        raise NetworkError("Did not hear from any running worker in "
                           "%d seconds" % self.heartbeat_timeout)

    def _accept_worker(self, sock, context):
        sock.setblocking(True)
        identifier = sock.recv(1024)
        if identifier:
            identifier = identifier.decode('ascii')
        if identifier and identifier in self._starting_workers:
            worker = self._starting_workers.pop(identifier)
            worker._accept_connection(sock)
            print("Identified worker %s " % str(worker))
            self._init_worker(worker)
            self._send_task_to_worker(worker, context)
            return worker
        else:
            print("Ignoring request from unknown worker")

    def _init_worker(self, worker):
        if _import_time_path[0] != '':
            worker._set_python_search_path(_import_time_path[0])
        if sys.path[0] != '' and sys.path[0] != _import_time_path[0]:
            worker._set_python_search_path(sys.path[0])

    def _get_network_events(self, context):
        running = [a for a in self._all_workers if a._running_task(context)]
        if len(running) == 0:
            if len(context._tasks) == 0:
                raise _NoMoreTasksError()
            elif len(self._starting_workers) == 0:
                raise NoMoreWorkersError("Ran out of workers to run tasks")
            # Otherwise, wait for starting workers to connect back and get tasks

        return util._poll_events(self._listen_sock, running,
                                 self.heartbeat_timeout)
