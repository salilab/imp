##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Michael Habeck and Wolfgang Rieping
##        
##          Copyright (C) Michael Habeck and Wolfgang Rieping
## 
##          All rights reserved.
##
## NO WARRANTY. This library is provided 'as is' without warranty of any
## kind, expressed or implied, including, but not limited to the implied
## warranties of merchantability and fitness for a particular purpose or
## a warranty of non-infringement.
##
## Distribution of substantively modified versions of this module is
## prohibited without the explicit permission of the copyright holders.
##


import sys
import time
from threading import Event
from thread import start_new_thread
from numpy.oldnumeric import sum


from AbstractGrid import TimeoutError

MSG_INIT_DONE = 1
MSG_CALL_METHOD = 8
MSG_LOOP = 11
MSG_INIT = 12
MSG_TERMINATE = 14
MSG_CLIENT_CRASHED = 2
MSG_CALL_METHOD_RESULT = 15
MSG_DEL_KEY = 17

MSG_INIT_FAILED = 3
MSG_SHOW_OUTPUT = 4
MSG_STOP = 5
MSG_RESULT = 6
MSG_QUIT = 7
MSG_EXCEPTION = 9
MSG_HASATTR = 10
MSG_SETATTR = 13

MSG_SET_ATTR = 13
MSG_GET_ATTR = 10
MSG_GET_TIME = 18

class MyResult(object):

    def __init__(self):

        object.__init__(self)

        self.__dict__['event'] = Event()
        self.__dict__['value'] = None

    def wait(self, timeout = None):
        self.event.wait(timeout)

        if timeout is not None:
            
            if not self.isSet():
                raise TimeoutError
        
        return self.value

    def __call__(self):
        return self.wait()

    def isSet(self):
        return self.event.isSet()

    def set(self):
        return self.event.set()

    def clear(self):
        return self.event.clear()
    
def _wrapper(f, result, *args, **kw):

    result.value = f(*args, **kw)
    result.set()

def threaded(f, *args, **kw):

    result = MyResult()
    
    start_new_thread(_wrapper, (f, result) + args, kw)
    
    return result

class RemoteObjectHandler:

    def __init__(self, kill_on_error = 0, signal_file = '', debug = False):

        self.debug = debug

        self.kill_on_error = kill_on_error
        self.signal_file = signal_file

        self.bindings = {MSG_QUIT: self.quit,
                         MSG_GET_ATTR: self.getattr,
                         MSG_SET_ATTR: self.setattr,
                         MSG_CALL_METHOD: self.call_method,                         
                         MSG_TERMINATE: self.terminate,
                         MSG_GET_TIME: self.get_time}

        self.object = None ## defined in derived classes
        
        self.times = []
        self.t_expire = 30. * 60

        self.delay = 0.0
        self.active = 0

    def send(self, msg, value = None):
        raise NotImplementedError

    def recv(self, msg):
        raise NotImplementedError
        
    def setattr(self, name, val):
        
        setattr(self.object, name, val)
        self.send(MSG_SETATTR, None)

    def getattr(self, name):
        
        val = getattr(self.object, name)
        self.send(MSG_GET_ATTR, val)

    def call_method(self, key, name, args, kw):

        #if self.debug: print 'call_method: %s' % name

        method = getattr(self.object, name)

        t0 = time.time()

        result = method(*args, **kw)

        t = time.time()

        self.append_time(t, t - t0)

        t = self.get_time()

        self.send(MSG_CALL_METHOD_RESULT, (key, result, t))

    def append_time(self, time_stamp, delta_t):

        delta_t = abs(delta_t)

        if delta_t > 100 * 24. * 60. * 60.:
            return

        self.times.append({'time_stamp': time_stamp, 'delta': delta_t})

        self.times = [d for d in self.times if \
                      abs(time_stamp - d['time_stamp']) < self.t_expire]

    def get_time(self):

        times = [t['delta'] for t in self.times]

        if times:
            t = sum(times) / len(times)
        else:
            t = None

        return t

    def set_object(self, o):        
        self.object = o

    def initialize(self):
        raise NotImplementedError

    def start(self):
        raise NotImplementedError

    def terminate(self):
        raise NotImplementedError

    def quit(self):

        if self.daemon:
            return
        else:
            self.terminate()


class RemoteObject(object):

    def __init__(self, op, handler_tid, manager):

        object.__init__(self)
        #print 'calling as %s'  % self.__class__.__name__

        attr_name = '_%s__handler_tid' % self.__class__.__name__
        #print 'debug2:',attr_name
        object.__setattr__(self, attr_name, handler_tid)
        self.__dict__['__handler_tid'] = handler_tid

        attr_name = '_%s__manager' % self.__class__.__name__
        object.__setattr__(self, attr_name, manager)
        self.__dict__['__manager'] = manager

        self.__dict__['_op'] = op
        #self.__dict__['key'] = 0

    def _get_url(self):
        #return self.__handler_tid
        return self.__dict__[ '_%s__handler_tid' % self.__class__.__name__]

    def _call_method(self, name, *args, **kw):
        raise NotImplementedError        

    def _get_attr(self, value):

        #print "debug: ",value,self.__handler_tid
        self.__manager.send(self.__handler_tid, MSG_GET_TIME, value)

        return self.__manager.recv(self.__handler_tid, MSG_GET_ATTR)

    def _get_time(self):

        self.__manager.send(self.__handler_tid, MSG_GET_TIME, None)

        return self.__manager.recv(self.__handler_tid, MSG_GET_TIME)

    def __getattr__(self, name):

        #print 'DEBUG RemoteObject.__getattr__: name = %s (%s)' \
        #     % (name, id(self))        

        if hasattr(self._op, name):

            val = getattr(self._op, name)

            if callable(val):
                return lambda *args, **kw: self._call_method(name, *args, **kw)

            else:
                return self._get_attr(name)

        else:
            raise AttributeError, 'Attribute %s does not exist' % name

    def __setattr__(self, name, val):

        #print 'DEBUG RemoteObject.__setattr__: name = %s, val = %s (%s)' \
        #      % (name, val, id(self))

        if hasattr(self, name):
            object.__setattr__(self, name, val)
        else:
            self.__manager.send(self.__handler_tid, MSG_SET_ATTR, (name, val))

if __name__ == '__main__':
    ro = RemoteObjectHandler()
