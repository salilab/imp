"""@namespace IMP.parallel.util Utilities for the IMP.parallel module."""

import socket
import select
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
                if tries > 10:
                    raise
            else:
                break
        return port


class _ContextWrapper:

    def __init__(self, obj):
        self.obj = obj


class _TaskWrapper:

    def __init__(self, obj):
        self.obj = obj


class _ErrorWrapper:

    def __init__(self, obj, traceback):
        self.obj = obj
        self.traceback = traceback


class _HeartBeat:
    pass


class _WorkerAction:
    pass


class _SetPathAction(_WorkerAction):

    def __init__(self, path):
        self.path = path

    def execute(self):
        sys.path.insert(0, self.path)


if hasattr(select, 'poll'):
    def _poll_events(listen_sock, workers, timeout):
        fileno = listen_sock.fileno()
        workermap = {fileno: listen_sock}

        p = select.poll()
        p.register(fileno, select.POLLIN)
        for worker in workers:
            fileno = worker._socket.fileno()
            workermap[fileno] = worker
            p.register(fileno, select.POLLIN)
        ready = p.poll(timeout * 1000)
        return [workermap[fd[0]] for fd in ready]

else:
    # Use select on systems that don't have poll()
    def _poll_events(listen_sock, workers, timeout):
        fileno = listen_sock.fileno()
        workermap = {fileno: listen_sock}
        waitin = [fileno]

        for worker in workers:
            fileno = worker._socket.fileno()
            workermap[fileno] = worker
            waitin.append(fileno)
        (ready, rout, rerr) = select.select(waitin, [], [], timeout)
        return [workermap[fd] for fd in ready]
