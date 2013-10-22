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


if hasattr(select, 'poll'):
    def _poll_events(listen_sock, slaves, timeout):
        fileno = listen_sock.fileno()
        slavemap = { fileno: listen_sock }

        p = select.poll()
        p.register(fileno, select.POLLIN)
        for slave in slaves:
            fileno = slave._socket.fileno()
            slavemap[fileno] = slave
            p.register(fileno, select.POLLIN)
        ready = p.poll(timeout * 1000)
        return [slavemap[fd[0]] for fd in ready]

else:
    # Use select on systems that don't have poll()
    def _poll_events(listen_sock, slaves, timeout):
        fileno = listen_sock.fileno()
        slavemap = { fileno: listen_sock }
        waitin = [fileno]

        for slave in slaves:
            fileno = slave._socket.fileno()
            slavemap[fileno] = slave
            waitin.append(fileno)
        (ready,rout,rerr) = select.select(waitin, [], [], timeout)
        return [slavemap[fd] for fd in ready]
