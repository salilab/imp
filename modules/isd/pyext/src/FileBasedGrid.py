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

import os
import time
import random
import cPickle
import shutil
import socket

from AbstractGrid import AbstractGrid, Server, Result
from ro import *
from OrderedDict import OrderedDict
from utils import touch, my_glob, WatchDog, Pipe

## TODO: move auxiliary functions to a separate module

def file_exists(name, timeout=None):

    name = os.path.expanduser(name)

    try:
        f = open(name)
        f.close()
        return True

    except IOError:
        return False

def valid_timestamp(name, max_delay):

    try:
        creation_time = os.stat(name).st_ctime
    except:
        return True
    
    sys_time = time.time()

    if creation_time-sys_time > 1.:
        msg = 'Inconsistent clocks: filename=%s, timestamp=%.2f, system time=%.2f'
        raise StandardError, msg % (name, creation_time, sys_time)

    return abs(creation_time-sys_time) < max_delay

class FileBasedCommunicator:

    remote_shell = 'ssh'
    polling_interval = 0.1
    lock_max_delay = 10.

    def __init__(self, temp_path, tid=None, debug=False, nfs_care=False):


        self.comm_path = os.path.join(temp_path, 'comm')
        self.temp_path = temp_path

        if not os.path.exists(self.comm_path):
            os.system('mkdir ' + self.comm_path)

        if tid is None:

            self.tid_counter = random.randint(0, 12345678)
            
            tid = self.tid_counter
            
            self.create_slot(tid)

        self.tid = tid

        self.message_id = 0

        self.data = OrderedDict()

        self.debug = debug
        
        self.nfs_care = nfs_care

        self.__stop = False

    def halt(self):
        self.__stop = True

    def create_slot(self, tid):

        slot = '%s/%d' % (self.comm_path, tid)

        if not os.path.exists(slot):
            os.system('mkdir ' + slot)

    def spawn(self, host_name, command, args, init_data, pipe = '',
              X_forward = False):

        self.tid_counter += 1

        self.create_slot(self.tid_counter)

        ssh_cmd = self.remote_shell

        if X_forward:
            ssh_cmd += ' -X'

        init_data['temp_path'] = self.temp_path
        init_data['debug'] = self.debug
        init_data['parent_tid'] = self.tid
        init_data['tid'] = self.tid_counter
        init_data['nfs_care'] = self.nfs_care

        args =  ' '.join([str(x) for x in args])
        
        filename = os.path.join(self.temp_path, 'init_data.%d' % init_data['tid'])

        f = open(filename, 'w')
        cPickle.dump(init_data, f)
        f.close()
        
        sh_cmd = "%s %s '%s %s %s %s' &" % (ssh_cmd, host_name, command, args,
                                          filename, pipe)
        
        os.system(sh_cmd)

        return self.tid_counter

    def load(self, filename):
        "load contents of message file filename by unpickling it"

        debug_shown = False

        while file_exists(filename + '.lock'):

            if self.nfs_care and self.lock_max_delay is not None:

                if file_exists(filename) and not \
                   valid_timestamp(filename + '.lock', self.lock_max_delay):

                    sys_time = time.time()

                    try:
                        file_time = os.stat(filename + '.lock').st_ctime
                    except:
                        continue

                    if self.debug:
                        msg = 'cPickle.load: lock file %s.lock has old timestamp: %d, system time is %d.'
                        print msg % (filename, file_time, sys_time)

                    break

            if self.debug and not debug_shown:
                print 'Waiting for message to be unlocked ...'
                debug_shown = True

            time.sleep(self.polling_interval)

        try:

            f = open(filename)
            _data = cPickle.load(f)
            f.close()

        except IOError, err_msg:

            while 1:

                print 'Error accessing %s (%s). Error message was "%s". Trying again...' % \
                      (filename, time.ctime(), err_msg)

                time.sleep(60.)

                try:
                    
                    f = open(filename)
                    _data = cPickle.load(f)
                    f.close()

                    break

                except IOError:
                     continue

        return _data

    def dump(self, op, filename):
      

        try:

            f = open(filename + '.lock', 'w')
            cPickle.dump(op, f)
            f.flush()
            f.close()

        except IOError, err_msg:

            while 1:

                print 'Error accessing %s (%s). Error message was "%s". Trying again...' % \
                      (filename, time.ctime(), err_msg)

                time.sleep(60.)

                try:
                    
                    f = open(filename + '.lock', 'w')
                    cPickle.dump(op, f, 1)
                    f.close()

                    break

                except IOError:
                    continue

        os.link('%s.lock' % filename, '%s' % filename)
        os.unlink('%s.lock' % filename)

    def dump_old(self, op, filename):

        try:

            f = open(filename + '.lock', 'w')
            cPickle.dump(op, f)
            f.flush()
            f.close()

        except IOError, err_msg:

            while 1:
                print 'Error accessing %s (%s). Error message was "%s". Trying again...' % \
                      (filename, time.ctime(), err_msg)

                time.sleep(60.)

                try:
                    
                    f = open(filename + '.lock', 'w')
                    cPickle.dump(op, f, 1)
                    f.close()

                    break

                except IOError:
                    continue

        shutil.copyfile('%s.lock' % filename, '%s' % filename)
        os.system('cat %s > /dev/null' % filename)
        os.unlink('%s.lock' % filename)

    def register(self, _id, tid, msg, data):

        key = (tid, msg)

        if key in self.data:
            self.data[key].append(data)
        else:
            self.data[key] = [data]

    def pop_message(self, tid, msg):

        key = (tid, msg)
        data = self.data

        if key in data and data[key]:
                
            value = data[key].pop(0)
            
            if not data[key]:
                del data[key]

            if self.debug:
                print 'pop_message', tid, msg

            return tid, msg, value

        else:
            return None

    def poll(self):
        """check if new message arrived, 
        extract data and register to self.data construct,
        remove message file
        """

        filenames = self.comm_path + '/%d/msg.*.msg' % self.tid

        touch = self.nfs_care

        files = my_glob(filenames, touch)

        if len(files) and self.debug:
            print 'poll: new message arrived:'

        data = []

        for f in files:

            sender, msg, _id = f.split('.')[-4:-1]

            _data = self.load(f)

            data.append((int(_id), int(sender), int(msg), _data))

            if self.debug:
                print 'poll: sender=%s, msg=%s, id=%s, type=%s'%\
                      (sender, msg, _id,_data.__class__.__name__)

            os.unlink(f)

        data.sort(lambda a, b: cmp(a[0], b[0]))

        [self.register(*d) for d in data]

    def find_message(self, sender, msg):
        """get message msg from sender sender, -1 stands for any.
        returns (tid, msg, data) or None
        """

        data = self.data
        
        if sender == -1:

            if msg == -1:

                if data:

                    (tid, _msg), _data = data.get_first_item()

                    return self.pop_message(tid, _msg)

            else:

                for (tid, _msg), _data in data.items():

                    if _msg == msg:
                        return self.pop_message(tid, _msg)

        else:

            if msg == -1:

                for (tid, _msg), _data in data.items():

                    if tid == sender:
                        return self.pop_message(tid, _msg)

            else:
                return self.pop_message(sender, msg)

        return None
            
    def recv(self, sender, msg):
        """get new message from sender, wait if necessary
        returns (tid, msg, data)
        """

        if self.debug:
            print 'recv (tid=%d): waiting for sender=%d, msg=%d' % \
                  (self.tid, sender, msg)

        result = None

        while not self.__stop:

            self.poll()

            result = self.find_message(sender, msg)

            if result is not None:
                break

            time.sleep(self.polling_interval)

        if self.debug:
            print 'received.'

        return result

    def send(self, receiver, msg, value):

        recv_path = self.comm_path + '/%d' % receiver

        while not os.path.exists(recv_path):

            print 'send: path %s does not exist. waiting ...' % recv_path
            
            time.sleep(self.polling_interval)

        time.sleep(self.polling_interval)
        
        filename = recv_path + '/msg.%d.%d.%d.msg' % (self.tid, msg, self.message_id)

        self.message_id += 1

        self.dump(value, filename)

        if self.debug:
            print 'sent: receiver=%d, msg=%d, msg_id=%d' % (receiver, msg, self.message_id - 1)

class FileBasedServer(Server):

    def terminate(self):

        if self.debug:
            self.dp( 'FileBasedServer.terminate: terminating proxy %s' % self.proxy)

        self.grid.send(self.url, MSG_TERMINATE, None)

class FileBasedRemoteObjectHandler(RemoteObjectHandler):

    def __init__(self, kill_on_error=0, signal_file='', temp_path='',
                 parent_tid=None, tid=None, debug=False, nfs_care=False):

        
        RemoteObjectHandler.__init__(self, kill_on_error, signal_file, debug)

        ## create watchdog to check every 60 mins whether child processes
        ## are still alive.

        self.watchdog = WatchDog(60, debug=debug)
        self.watchdog.start()
        
        self.create_communicator(temp_path, tid, debug, nfs_care)

        self.parent_tid = parent_tid

        self.message_id = 0

        self.initialize()
        
    def create_communicator(self, temp_path, tid, debug, nfs_care):
        
        self.communicator = FileBasedCommunicator(temp_path, tid, debug,
                                                  nfs_care)

    def send(self, msg, value = None):
        self.communicator.send(self.parent_tid, msg, value)

    def recv(self, msg):
        return self.communicator.recv(self.parent_tid, msg)
    
    def initialize(self):
        """wait for initialization request and initialize accordingly"""

        print 'Initializing...'

        tid, msg, vals = self.recv(MSG_INIT)

        if 'object' in vals:
            self.set_object(vals['object'])

        if 'daemon' in vals:
            self.daemon = vals['daemon']

        if 'expiration_time' in vals:
            self.t_expire = vals['expiration_time']

        self.send(MSG_INIT_DONE)

        print 'Done.'

    def start(self):
        """main request management loop. Head node sends commands which this
        thread will execute"""

        print 'Ready.'

        self._terminate = False

        self.watchdog.set(time.time())

        while 1 and not self._terminate:

            tid, msg, data = self.recv(-1)

            if msg == MSG_STOP:
                return

            if type(data) is not tuple:
                data = (data,)

            method = self.bindings[msg]

            if self.kill_on_error:

                try:
                    method(*data)
                    self.watchdog.set(time.time())

                except:
                    self.send(MSG_CLIENT_CRASHED)
                    sys.exit(0)

            else:

                method(*data)
                self.watchdog.set(time.time())

        if not self.debug:
            print 'Grid has been halted.'
            sys.exit(0)

        else:
            print 'Debugging mode, keeping Python interpreter alive.'


    def terminate(self, x=None):
        
        self._terminate = True

class FileBasedRemoteObject(RemoteObject):
    """the filebased proxy instance"""

    def _call_method(self, name, *args, **kw):

        #result = self.__manager.create_result_object(self.__handler_tid)

        result = self.__manager.create_result_object(self)

        result.info = {'name': name, 'args': args, 'kw': kw}
        
        value = (result.key, name, args, kw)
            
        self.__manager.send(self.__handler_tid, MSG_CALL_METHOD, value)

        return result  

class FileBasedGrid(AbstractGrid):    

    def __init__(self, hosts, src_path, display, X11_delay, debug, verbose, nfs_care=False):

        AbstractGrid.__init__(self, hosts, src_path, display, X11_delay, debug, verbose, shared_temp_path=True)

        #copy all files in current dir to remote host.
        self.initialise(src_path, ['filebased_loader','ro',
                                   'FileBasedGrid','OrderedDict'])


        self.set_loader(src_path, 'filebased_loader')
        
        self.create_communicator(nfs_care) 
        
        self.results = {}        
        self.key = 0
        
        self.__stop = False
        self.__stopped = False

        if self.debug:
            print 'FileBasedGrid created: tid = ', self.communicator.tid

    def set_debug(self, debug):
    
        AbstractGrid.set_debug(self, debug)
        self.communicator.debug = debug

    def create_communicator(self, nfs_care):
        
        self.communicator = FileBasedCommunicator(self.hosts[0].temp_path,
                                debug = self.debug, nfs_care = nfs_care)

    def publish(self, instance):
        """
        for each host of self.hosts
            -launch a FileBasedRemoteObjectHandler through the filebased loader.
            - pickle the instance object and send it to the handler.
            setting an attribute of the proxy results in a message being sent to
            the concerned host.
            - create a FileBasedServed for the proxy
            - add it to the queue self.queues[sid] 
                and to the list self.servers[sid]
                and set server.grid and server.proxy
        returns the service id associated to this instance.

        note from the authors
        In FileBasedGrid there is one to one correspondence between
        a Server and a Handler...
        
        """

        service_id = self.create_service_id(instance)
    
        for host in self.hosts:

            proxy = self.create_proxy(instance, host, self.display, daemon = 1)
            
            server = FileBasedServer(proxy, service_id, host, self.debug)

            server.url = proxy._get_url()

            self.add_server(server)

            if self.display and self.X11_delay is not None:
                time.sleep(self.X11_delay)
                            
        return service_id

    def create_proxy(self, instance, host, display = 0, daemon = 0):
        """
        (copied from Grid, called from AbstractISDGrid.create_server)
        
        """

        handler_tid = self.create_handler(instance, host, display, daemon)

        proxy = FileBasedRemoteObject(instance, handler_tid, manager = self)

        if self.debug:
            print 'Connected: tid=%d' % handler_tid
            
        return proxy

    def create_handler(self, instance, host, display, daemon):

        d = {'object': instance,
             'daemon': daemon}

        handler_tid = self.spawn_handler(host, display)

        if self.debug:
            print 'Initialising service on host "%s"' % host.name

        self.send(handler_tid, MSG_INIT, d)

        if self.debug:
            print 'MSG_INIT sent.'

        return handler_tid

    def spawn_handler(self, host, display):

        handler_script = os.path.join(self.hosts[0].temp_path,
                                      self.loader_filename)

        init_data = {'niceness': host.niceness,
                     'display': display}

        argv = [handler_script]

        #add required init commands prior to launching anything else on the target host.
        if host.init_cmd != '':
            if host.init_cmd.rstrip().endswith(';'):
                command = host.init_cmd 
            else:
                command = host.init_cmd + ';' 
        else:
            command = ''

        if display:

            if type(display) is type(0):

                master_name = socket.gethostname()

                if host.name == master_name:
                    display = ':0.0'
                else:
                    display = master_name + ':0.0'
            
            command += 'xterm'

            argv = ['-title', host.name, '-geometry', self.window_size,
                    '-hold', '-e',
                    host.python, '-i'] + argv

            pipe = ''
            
        else:
            command += host.python
            pipe = '> /dev/null'

        if self.debug:
            print 'Spawning service on host "%s"' % host.name

        X_forward = display

        tid = self.communicator.spawn(host.name, command, argv,
                                      init_data, pipe, X_forward)

        if self.debug:
            print 'Service spawned: tid = ', tid

        return tid                                                                                   

    def recv(self, tid, msg):
        return self.communicator.recv(tid, msg)

    def send(self, tid, msg, value = None):    
        self.communicator.send(tid, msg, value)    

    def create_result_object(self, proxy):
        """
        Results has to be temporarily stored in the Grid
        until their values are calculated
    
        """
    
        key = self.key
        self.key += 1

        if key in self.results:
            print 'Result object for key %d already exists.' % key

        #result = Result(tid, key, self)

        result = Result(proxy)
        
        result.key = key

        self.results[key] = result

        return result

    def run(self):

        self.removed = Pipe(100000)

        while not self.__stop:

            if os.path.exists(os.path.join(self.hosts[0].temp_path, 'quit')):

                [self.send(server.url, MSG_TERMINATE) for \
                 server in self.servers[service_id]]

                #self.halt()
                self.terminate() 

                break

            recv = self.recv(-1, -1)

            if recv is None:
                continue

            tid, msg, data = recv

            if self.debug:
                template = 'received: tid=%d, msg=%d, type(data)=%s'

            if msg == MSG_INIT_DONE:

                for service_id, server_list in self.servers.items():

                    server = [s for s in server_list if s.url == tid]

                    if server:

                        if len(server) > 1:
                            raise 'Inconsistency'

                        if self.debug:
                            print server[0].host.name, 'ready.'

                continue

            elif msg == MSG_CALL_METHOD_RESULT:

                try:
                    key, value, elapsed = data
                except:
                    print data, len(data)

                if key in self.results:

                    result = self.results[key]

                    result.value = value
                    result.event.set()

                    self.add_time(result.proxy, elapsed)

                    if result.proxy._selfrelease:

                        if self.debug:
                            self.dp( 'FileBasedGrid.run: releasing server %s for the grid' \
                                     % result.proxy._server )

                        result.proxy._selfrelease = False
                        self._release_service(result.proxy)

                    del self.results[key]
                    self.removed.put(key)

                else:

                    print 'Result object, key %d, not known.' % key

                    if key in self.removed.pipe:
                        print 'Key has already been deleted.'

            elif msg == MSG_CLIENT_CRASHED:

                ## find service to which the crashed client belongs

                for service_id, server_list in self.servers.items():

                    crashed = [server for server in server_list \
                               if server.url == tid]

                    if crashed:
                        break

                else:
                    raise 'Inconsistency: TID %d not known.' % tid

                msg = 'Client on host "%s" inaccessible. Attempting shutdown...'
                print msg % crashed[0].host.name

                self.terminate(service_id)

            else:
                print 'Unknown message', tid, msg

        self.__stopped = self.__stop

    def ishalted(self):
        return self.__stopped

    def terminate(self, service_id = None):

         AbstractGrid.terminate(self, service_id)
         
         self.communicator.halt()
         self.__stop = True

         if self.debug: ## please keep debug statement!
             print 'FileBasedGrid: terminated.'

    def __del__(self):
        self.terminate()
       

