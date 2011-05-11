##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Darima Lamazhapova and Wolfgang Rieping
##        
##          Copyright (C) Michael Habeck and Wolfgang Rieping
## 
##          All rights reserved.
##
## NO WARRANTY. This library is provided'as is' without warranty of any
## kind, expressed or implied, including, but not limited to the implied
## warranties of merchantability and fitness for a particular purpose or
## a warranty of non-infringement.
##
## Distribution of substantively modified versions of this module is
## prohibited without the explicit permission of the copyright holders.
##

import os
import sys
import socket
import threading 
import time
import Queue
import Pyro.core, Pyro.errors, Pyro.util
import atexit
import string

from IMP.isd.utils import WatchDog
from IMP.isd.AbstractGrid import AbstractGrid, Server, Result
from IMP.isd import PyroUtils

Pyro.config.PYRO_PRINT_REMOTE_TRACEBACK = 1
Pyro.config.PYRO_DETAILED_TRACEBACK = 1
Pyro.config.PYRO_MOBILE_CODE = 1

def get_sge_host_list():
    """parse the sge variable $PE_HOSTLIST"""
    hostlist = []
    for line in open(os.environ('PE_HOSTLIST')):
        tokens=line.split()
        rep = int(tokens[1])
        for i in xrange(rep):
            hostlist.append(tokens[0])
    return hostlist

class PyroGrid(AbstractGrid):

    def __init__(self, hosts, src_path, display, X11_delay, 
                 debug, verbose, 
                 shared_temp_path, nshost, terminate_during_publish, 
                 method='ssh'):
        
        AbstractGrid.__init__(self, hosts, src_path, display, X11_delay,\
                              debug, verbose, shared_temp_path)

        #copy remaining files specific for the pyrogrid
        self.initialise(src_path, ['PyroGrid','PyroUtils',
                                   'PyroHandlerLoader'])

        #set the pyro loader as the loader
        self.set_loader(src_path, 'PyroHandlerLoader')
        
        #method is either ssh or qsub
        if method != 'ssh' and method != 'qsub':
            raise ValueError, "unknown method: %s. Try ssh or qsub" % method
        self.method = method

        #number of hosts
        self.n_hosts = len(hosts)

        #name of the host having the pyro-ns nameserver
        self.nshost = nshost         
        
        self.timeout = 60.0 ## (for WatchDog, in minutes) 

        self.handlers = {}  ## maps instance (URI) to a handler (proxy)

        self.__terminate_during_publish = terminate_during_publish
        self.__published = {}
        
        self.comm_acquired = False

        self.__stopped = False
        
    def _launch_handler(self, handler_uri, host):
        """start up handlers in parallel. Handlers have URI 'handler_uri' and
        are launched on 'host'.
        """

        handler_script = os.path.join(host.temp_path,
                                      self.loader_filename)

        #arguments to be passed to the handler script launched on each client.
        argv = "%s %s %s %s %s %s %s %s" \
               % (handler_script, host.niceness, self.nshost, handler_uri, \
                  self.timeout, self.debug, self.display, host.temp_path)        
        
        #if x11 forwarding is requested
        if self.display:
            try:
                title = socket.getfqdn(host.name).split('.')[0]
            except:
                title = host.name
            cmd =  "xterm -T %s -geometry %s -hold -e \"%s -i %s\"" \
                  % (title, self.window_size, host.python, argv) 
        
        else: #no x11 forwarding
            cmd = "%s %s" % (host.python, argv)

        #if requested, add required init commands prior to 
        #launching anything else on the target host.
        if host.init_cmd != '':
            if host.init_cmd.rstrip().endswith(';'):
                cmd = host.init_cmd + cmd
            elif host.init_cmd.rstrip().endswith('!'):
                cmd = host.init_cmd.rstrip()[:-1] + cmd
            else:
                cmd = host.init_cmd + ';' + cmd 

        #now wrap everything for ssh
        if self.method == 'ssh':
            cmd  = "ssh %s '%s' " % (host.name, cmd)
        else:
            cmd = "qrsh -inherit %s '%s' " % (host.name, cmd)

        #if debug then be verbose. In any case do this in background.
        if self.debug: 
            cmd = cmd + " > /dev/null &" 
            print 'PYRO echo: %s' % cmd
        else:
            cmd = cmd + "&"

        #finally issue command to the shell.
        os.system(cmd)

    def _connect_handlers(self, handler_uris, handlers, handlers_lock, hosts_map):

        #handlers: a FIFO queue, 
        #handlers_lock: a Condition lock
        #handler (see below): the PyroHandler proxy

        ## connect to handlers sequentially

        _ns = PyroUtils.get_nameserver(self.nshost)

        for i in range(self.n_hosts):
            try:
                handler = PyroUtils.get_proxy(handler_uris[i], ns = _ns)
            except:
                print 'PyroGrid._connect_handlers: failed to connect to "%s"' % handler_uris[i]
                raise
                                        
            #tell the pyro proxy to close the socket for now.
            handler._release()                                    
            #tell the handler on which host he's running.
            handler.host = hosts_map[handler_uris[i]]

            if self.debug:
                print 'PyroGrid._connect_handlers: handler "%s" on "%s" is ready' \
                      % (handler_uris[i], handler._pyro_suppl['host'])

            #acquire the lock
            handlers_lock.acquire()
            #put the handler proxy object into the queue
            handlers.put(handler)

            if self.debug:
                print 'PyroGrid._connect_handlers: notifies "%s" is in the queue' % handler_uris[i]
            #update queue status and release lock
            handlers_lock.notify()
            handlers_lock.release()

    def _launch_instances(self, instance, instance_uris, \
                          handlers, handlers_lock, \
                          launched_instances, launched_instances_lock, \
                          self_handlers_lock):

        ## launch instances in parallel (handler.publish launch instance in a thread)

        #handlers: PyroHandler proxies in a Queue(). It's different from 
        # self.handlers, which is a dict['URI']=handler proxy.
        #handlers_lock: the condition lock associated to filling this queue.

        ns_clear_interval = 5

        for i in range(self.n_hosts):
            
            #get the lock, and obtain the next queue element, or wait for it to be filled.
            handlers_lock.acquire()            
            if handlers.empty(): handlers_lock.wait()
            handler = handlers.get()
            handlers_lock.release()            
            
            instance_uri = instance_uris[i]

            if self.debug:
                print 'PyroGrid._launch_instances: "%s" is being published remotely...' % instance_uri
            
            #TODO: remove the release(), since it was done previously
            handler._release()
            handler.publish(instance, instance_uri, delegate = True)

            #The self.handlers dict is a shared resource hence the acquire / release mechanism.
            self_handlers_lock.acquire()
            self.handlers[instance_uri] = handler
            self_handlers_lock.release()

            launched_instances_lock.acquire()
            launched_instances.put(instance_uri)

            if self.debug:
                print 'PyroGrid._launch_instances: notifies "%s" is being published' % instance_uri

            launched_instances_lock.notify()
            launched_instances_lock.release()

            #if i and not i % ns_clear_interval:
            #    print 'Pausing for 1 min'
            #    sleep(60.)
                
    def _connect_instances(self, service_id, \
                           launched_instances, launched_instances_lock, \
                           self_handlers_lock, servers_lock):

        ## connect to instances sequentially

        _ns = PyroUtils.get_nameserver(self.nshost)
        
        for i in range(self.n_hosts):

            launched_instances_lock.acquire()            
            if launched_instances.empty(): launched_instances_lock.wait()                
            instance_uri = launched_instances.get()
            launched_instances_lock.release()

            try:
                instance_proxy = PyroUtils.get_proxy(instance_uri, ns = _ns)
            except:
                print 'PyroGrid._connect_instances: failed to connect to "%s"' % instance_uri
                raise

            if self.debug:
                print 'PyroGrid._connect_instances: instance "%s" on "%s" is ready' \
                      % (instance_uri, instance_proxy._pyro_suppl['host'])

            self._append_instance(service_id, instance_proxy, \
                                  self_handlers_lock, servers_lock)

        self.__published[service_id] = True
            
    def _append_instance(self, service_id, instance_proxy, \
                         self_handlers_lock, servers_lock):

        ## extracts parameters needed for PyroProxy below
        
        instance_proxy._release()
        instance_uri = instance_proxy._pyro_suppl['uri']

        self_handlers_lock.acquire()
        handler = self.handlers[instance_uri]        
        handler._release()
        host = handler.host        
        self_handlers_lock.release()

        ## wrap raw Pyro proxy in PyroProxy (smart remote object)
        ## and then in PyroServer (for grid interface)

        proxy = PyroProxy(instance_proxy, debug = self.debug, verbose = self.verbose) 
        server = PyroServer(proxy, service_id, host, self.debug)

        ## register the handler in the proxy (to monitor whether the grid is alive)

        self_handlers_lock.acquire()
        proxy.__dict__['_handler'] = handler
        self_handlers_lock.release()

        ## register the server in the grid

        #add the grid to server.grid and fill in self.queues and self.servers 
        #self.queues: a dict[service_id of instance 
        #to be published]=Queue(-1) containing the DerivedServers
        #self.servers:  dict[service_id of instance 
        #to be published]=[list of PyroServer instances]
        servers_lock.acquire()
        self.add_server(server)
        
        if self.debug:
            print 'PyroGrid._append_instance: notifies %s is ready' % server

        servers_lock.notify()
        servers_lock.release()        
        
    def start_handlers(self, service_id):
        """
        Starts PyroHandler on each host (non-blocking) and 
        returns a Condition lock object and 
                a FIFO queue that is being filled with their 
                Pyro proxies by a separate thread
                   
        """        
        #create a list of unique URIs for each host
        handler_uris = PyroUtils.create_unique_uri(\
            string.join(('PyroHandler', service_id),'.'),\
            n = self.n_hosts,  ns = self.nshost)
        
        #create a FIFO queue of infinite capacity
        handlers = Queue.Queue(-1)
        #create the condition lock, see effbot.org/zone/thread-synchronization.htm
        handlers_lock = threading.Condition()

        hosts_map = {}
        
        for i in range(self.n_hosts):
            #ssh to each host and fork a PyroHandler thread.
            self._launch_handler(handler_uris[i], self.hosts[i])
            hosts_map[handler_uris[i]] = self.hosts[i]

            ## sleep here is to avoid errors like
            ## "Warning: No xauth data; using fake 
            ##  authentication data for X11 forwarding."           
            
            #TODO: speedup things by checking if it is necessary
            time.sleep(self.X11_delay) 

        #fill the queue asynchronously with the handler proxy objects that were launched.
        t = threading.Thread(target = self._connect_handlers, \
                   args = (handler_uris, handlers, handlers_lock, hosts_map)) 
        t.start()

        return handlers, handlers_lock
        
    def start_instances(self, service_id, instance, handlers, handlers_lock):

        #create n unique URIs for the instance, out of its service_id.
        instance_uris = PyroUtils.create_unique_uri(\
            service_id, n = self.n_hosts, ns = self.nshost)

        #queue and lock for managing the launched instances of the 'instance' object
        #via the PyroHandler
        launched_instances = Queue.Queue(-1)
        launched_instances_lock = threading.Condition()

        instances = Queue.Queue(-1)
        #lock for the self.handlers dict
        self_handlers_lock = threading.Condition()

        servers_lock = threading.Condition()

        #Call PyroHandler.publish(instance) on each host
        t0 = threading.Thread(target = self._launch_instances, \
                    args = (instance, instance_uris,  handlers, handlers_lock,\
                            launched_instances, launched_instances_lock, \
                            self_handlers_lock))
        t0.start()

        #Create a PyroServer instance for each host, and put
        #them into
        #self.queues: a dict[service_id of instance 
        #to be published]=Queue(-1) containing the DerivedServers
        #self.servers:  dict[service_id of instance 
        #to be published]=[list of PyroServer instances]
        t1 = threading.Thread(target = self._connect_instances, \
                    args = (service_id, \
                            launched_instances, launched_instances_lock, \
                            self_handlers_lock, servers_lock))
        t1.start()

        return servers_lock

    def publish(self, instance):
        """
        Publish instance on all the hosts of the grid
        (specific for the communication method)

        Returns: service_id

        Internally:

        It creates list of servers performing the job
        specified by this service_id
        
        """
     
        #get unique identifier for this instance
        service_id = self.create_service_id(instance)

        self.__published[service_id] = False

        #start PyroHandlers and give them an URI which contains service_id)
        handlers, handlers_lock = self.start_handlers(service_id)
        
        #start the PyroServer instances which control the underlying 'instance' objects
        #published on each host by the PyroHandler
        servers_lock = self.start_instances(service_id, instance,
                                            handlers, handlers_lock)

        if self.debug:
            print 'PyroGrid.publish: waiting for service %s to become available' % service_id

        print 'Distributing jobs on the grid...'
                 
        servers_lock.acquire()
        if not service_id in self.servers: servers_lock.wait()
        servers_lock.release()

        #now that there is at least one server in the queue that can accept a job, launch the sims.
        print 'Start calculations on the grid... '
        
        return service_id

    def check_published(self):

        def publishing():
            
            return False in self.__published.values() or \
                   False in [ len(self.hosts) == len(self.servers[service_id])
                              for service_id in self.__published.keys() ]                                                
        
        if publishing():
            
            print 'WARNING: Attempting shutdown while publishing!'
            
            if not self.__terminate_during_publish:
                print '         Wait until publishing is complete...' 
                while publishing():
                    time.sleep(0.5)
            else:
                print '         Do not wait until publishing is complete...'
                print '         Please, stop remote processes manually!'

    def ishalted(self):
        return self.__stopped
                    
    def terminate(self):

        ## AbstractGrid.terminate:
        ##
        ## 1. prevents server from being acquired [AbstractGrid.terminate]
        ## 2. terminates server and its proxy:
        ##       AbstractGrid.terminate -> PyroServer.terminate ->
        ##       -> PyroProxy.terminate,
        ##       where PyroProxy.terminate does
        ##
        ## PyroGrid.terminate:
        ##
        ## 3. terminates handler [ PyroHandler.terminate ]
        
        self.check_published()

        AbstractGrid.terminate(self)

        for instance_uri, handler in self.handlers.items():

            try:
                handler._release()
                handler.terminate()
                self.handlers.pop(instance_uri)
                
            except Pyro.errors.PyroError,e:
                print  ''.join(Pyro.util.getPyroTraceback(e))

        self.__stopped = True
        
        if self.debug: ## please do not remove.
            print 'PyroGrid: terminated'

    def broadcast(self, sfo_id, funcname, *args, **kw):
        if self.comm_acquired == False:
            self.comm_proxies = [ self.acquire_service(sfo_id) for i in \
                    xrange(self.n_hosts)]
            self.comm_acquired = True
        return [getattr(prox, funcname)(*args, **kw) for prox in \
                self.comm_proxies]

    def scatter(self, sfo_id, funcname, arglist, kwlist=None):
        if self.comm_acquired == False:
            self.comm_proxies = [ self.acquire_service(sfo_id) for i in \
                    xrange(self.n_hosts)]
            self.comm_acquired = True
        results = []
        if kwlist is None:
            kwlist=[{} for i in xrange(len(arglist))]
        if not hasattr(arglist[0],'__iter__'):
            arglist = [[i] for i in arglist]
        for prox,args,kw in zip(self.comm_proxies,arglist,kwlist):
            func=getattr(prox, funcname)
            if type(args) is dict:
                results.append(func(args,**kw))
            else:
                results.append(func(*args, **kw))
        return results

    def gather(self, results):
        retval=[]
        for server in results:
            retval.append(server.get())
        return retval

    def release_all(self):
        for prox in self.comm_proxies:
            self.release_service(prox)
        self.comm_acquired = False

class PyroHandler(Pyro.core.ObjBase):
    """    
    Runs on remote side, non-specific object. 

    It is a tool to launch instances of any picklable object
    on the remote host (where it resides)

    If PyroHandler is idle longer than .timeout time, it kills itself
    on the remote side (together with all registered instances)
    
    Returns: raw Pyro proxy
    
    """

    def __init__(self, timeout, nshost, debug):

        Pyro.core.ObjBase.__init__(self)

        self.debug = debug
        self.nshost = nshost
        self.instance_uri = None
        self.host = None

        #fork a WatchDog thread, which sleeps all the time and quits if it 
        #hasn't been pinged after timeout minutes, by calling self.watchdog.set(xxx)
        #where xxx is a float (the current time).
        self.watchdog = WatchDog(timeout, debug, logfile='watchdog.debug')
        self.watchdog.start()        

    def set(self, time):
        self.watchdog.set(time)
 
    def publish(self, instance, instance_uri, delegate):
        """
        delegate = False for descendants of Pyro.core.ObjBase

        Publishes a copy of the instance on the host where PyroHandler
        is running.
        
        """        

        if self.debug:
            print 'PyroHandler.publish: publishing instance %s with URI %s' % (instance, instance_uri)

        self.t = threading.Thread(target = PyroUtils.launch_instance, \
                        args = (instance, instance_uri, delegate,
                                self.nshost, self.debug, False))
        self.t.start()

        self.instance_uri = instance_uri

        ## to enable cleaning up upon remote shutdown

        atexit.register(self.terminate)

    def terminate(self):

        ## called from PyroProxy.terminate or on sys.exit (WatchDog call)

        if self.debug:
            print 'PyroHandler.terminate: %s terminating... ' % self

        ##--- call terminate only 
        ## if the handler is already terminated (e.g. by hands)
        ## then function is not called on the exit

        topop = [atexit._exithandlers[i][0] == self.terminate
                 for i in range(len(atexit._exithandlers))]
        if True in topop:
            atexit._exithandlers.pop( topop.index(True) )

        ##--- unregister from the nameserver            
        ## it will waits PyroUtils.default_timeout to let it
        ## finish self-unregistration
            
        if not self.instance_uri == None:
            PyroUtils.unregister(self.instance_uri, ns = self.nshost)

        ##--- shuts down remote process
        ## will leave interpreter alive if debug = True
            
        ## PyroHandler itself is an object instance managed
        ## by Pyro.core.Daemon, hence will be stopped by setting
        ## PyroHandler._pyro_stop to True
            
        self._pyro_stop = True
        
        ## PyroUtils.is_stopped(self._pyro_suppl['uri'], ns = self.nshost)
        ##
        ## but also it may cause problems when running without xterm  (unconfirmed)
        ## because it shuts down the python process (daemon.shutdown()) causing
        ## connection lost error
    
    def __str__(self):

        if hasattr(self, '_pyro_suppl'):
            s = '%s(uri="%s", host="%s")' % (self.__class__.__name__, \
                self._pyro_suppl['uri'], self._pyro_suppl['host'])
        else:
            s = self.__class__.__name__
            
        return s
            
class PyroServer(Server):

    def terminate(self):

        if self.debug:
            print 'PyroServer.terminate: is terminating %s...' % self.proxy

        self.proxy.terminate()
        
 
class PyroProxy(threading.Thread):
    """
    This is high level wrapper of Pyro proxy objects,
    returns immediately with an empty Result object    

    Used to be able to be used alone (without a grid) since it could take care
    of clashing invocations.

    REMARK:

        Calls always has to be in the following order:
    
        proxy = g.acquire_service(service_id) ## returns proxy object
        
        proxy.set_parameters(p)  # these functions will be performed sequentally
        proxy.f()                # and service won't be freed
        proxy.g()                # unless .release_service() is called
    
        g.release_service(proxy)

        However if g.acquire_service call is followed immediately by
        g.release_service, without calling proxy.f() or other proxy functions,
        service will never be actually freed. Because service is freed from
        inside PyroProxy in the run() cycle.
             
    """
    
    def __init__(self, pyro_proxy, debug = False, verbose = False):
        """
        pyro_proxy = raw Pyro proxy object
        
        """
        threading.Thread.__init__(self)
        Pyro.core.initClient()

        self.__verbose = verbose
        self.__debug = debug
                
        self.__called = threading.Event()
        self.__call_method_name   = None
        self.__call_method_args   = None
        self.__call_method_result = None

        pyro_proxy._release()
        self.__uri = pyro_proxy._pyro_suppl['uri']
        self.__host = pyro_proxy._pyro_suppl['host']
        self.__nshost = pyro_proxy._pyro_suppl['nshost']
        self.__callable = pyro_proxy._pyro_suppl['callable']

        self.__stop = False
        self.__stopped = False

        self.__debug_out = sys.stdout
        if self.__verbose:
            self.__debug_out = open('./DEBUG_%s.log' % self.__uri, 'w')

        ## every attribute declared (assigned) after self.__pyro_proxy will
        ## be declared (assigned) in the remote object

        self.__pyro_lock = threading.Lock()
        self.__pyro_proxy = pyro_proxy

        if self.__debug and self.__verbose:
            self.__dp('%s is created' % self)
                
        self.start()        

    def __dp(self, str):

        if self.__verbose: s0 = '%s ' % self.__uri
        else: s0 = ''
        
        if self.__pyro_lock.locked(): s1 = '  locked'
        else: s1 = 'unlocked'

        if self.__called.isSet(): s2 = 'called'
        else: s2 = '  idle'
                
        print >>self.__debug_out, '[%s %s, %s] %s' \
              % (s0, s1, s2, str)

        if self.__verbose:
            self.__debug_out.flush()
        
    def __setattr__(self, name, val):

        if '__pyro_proxy' in self.__dict__.keys() and \
               not name in self.__dict__.keys():

            self.__pyro_lock.acquire()
            self.__pyro_proxy._release()
            setattr(self.__pyro_proxy, name, val)
            self.__pyro_lock.release()
            
        else:
            
            threading.Thread.__setattr__(self, name, val)
        
    def __getattr__(self, name):

        ## Called when an attribute lookup has not found the attribute
        ## in the usual places (i.e. it is not an instance attribute
        ## nor is it found in the class tree for self)

        msg = 'PyroProxy.__getattr__(%s): ' % name

        if name in self.__callable:
            
            if self.__callable[name]:

                ## function '_call_method' will be invoked instead of function 'name'
                ## and arguments of function 'name' will go into '*args'

                if self.__debug and self.__verbose:
                    self.__dp( msg + 'is callable')
                
                return lambda *args: self.__call_method(name, *args)

            else:
                self.__pyro_lock.acquire()
                self.__pyro_proxy._release()                
                attr = getattr(self.__pyro_proxy, name)
                self.__pyro_lock.release()

                return attr            
        else:
            raise ValueError, 'Proxy object does not have attribute "%s"' % name
        
    def __call_method(self, name, *args):

        msg = 'PyroProxy.__call_method(%s): ' % name

        if self.__debug and self.__verbose:
            self.__dp( msg + 'acquiring the lock "%s", args = <%s>' \
                     % (name, args) )

        self.__pyro_lock.acquire()
        
        ## NOTE: we do not have to _release here because
        ##       it is done in remote_call (also it is faster if we do not
        ##       reconnect when call is from within of the same thread)
        ##
        ## self.__pyro_proxy._release()
            
        if self.__debug and self.__verbose:
            self.__dp( msg + 'acquired the lock "%s", args = <%s>' % (name, args) )

        self.__call_method_name = name
        self.__call_method_args = args 
        self.__call_method_result = Result(self) 

        if self.__debug and self.__verbose:
            self.__dp( msg + '"%s", args = <%s>' \
                     % (self.__call_method_name, self.__call_method_args) )

        self.__called.set()  ## "called"

        if self.__debug and self.__verbose:
            self.__dp( msg + 'now called (triggers START )')

        return self.__call_method_result
        
    def run(self):

        while not self.__stop:

            msg = 'PyroProxy.run(%s): ' % self.__call_method_name

            if self.__debug and self.__verbose:
                self.__dp( msg + 'waiting for calls...' )

            self.__called.wait()  ## wait until "called"
            if self.__stop: break

            msg = 'PyroProxy.run(%s): ' % self.__call_method_name
            
            if self.__debug and self.__verbose:
                self.__dp( msg + 'now called (START)' )

            try:
                if hasattr(self, '_handler'):
                    remote_call( self.__debug, self.__verbose, self._handler,
                            'set', time.time() )
                
                self.__call_method_result.value = remote_call( self.__debug, self.__verbose,\
                    self.__pyro_proxy, self.__call_method_name, *self.__call_method_args )

            except Exception,e:

                if not self.__stop:
                    self.__dp( msg + ''.join(Pyro.util.getPyroTraceback(e)) )
                    print msg + 'exception occurred remotely! [%s]' % self
                    print msg + ''.join(Pyro.util.getPyroTraceback(e))
                    raise

            self.__call_method_result.event.set()

            ## when the result is ready
            
            if self.__debug and self.__verbose:
                self.__dp( msg + 'becoming idle...' )

            self.__called.clear()  ## "idle"

            self.__call_method_name = None

            if self.__debug and self.__verbose:
                self.__dp( msg + 'now idle (FINISH)' )
                
           ## calculations are finished -> release the service in the grid

            if hasattr(self, '_server') and hasattr(self._server, 'grid') \
                   and self._selfrelease:

                if self.__debug and self.__verbose:
                    self.__dp( msg + 'releasing server %s for the grid' % self._server )
                
                self._selfrelease = False                

                self._server.grid._release_service(self)

            ## now, when everything is assigned, can be free

            self.__pyro_lock.release()

        if not self.__debug_out is sys.stdout:
            self.__debug_out.close()

        self.__stopped = True
        
        if self.__debug:
            print msg + ' %s terminated' % self

    def ishalted(self):
        return self.__stopped
    
    def terminate(self, terminate_proxy = 0):

        if self.__debug and self.__verbose: 
            print 'PyroProxy.terminate: %s terminating...' % self

        self.__stop = True
        self.__called.set()
            
        ## if this proxy is used within PyroGrid,
        ## PyroHandler.terminate will clean up by:
        ##
        ##   1. unregister Pyro URI from nameserver
        ##   2. close remote python process
        ##
        ## therefore terminate_proxy should be 0

        if terminate_proxy:
        
            self.__pyro_lock.acquire()

            ## 1. unregister Pyro URI from nameserver and handler
            ## 2. leaves remote python process intact
            
            self.__pyro_proxy._release()
            self.__pyro_proxy._pyro_stop = True

            if not PyroUtils.is_stopped(self.__uri, ns = self.__nshost):
                PyroUtils.unregister(self.__uri, ns = self.__nshost)

            if hasattr(self, '_handler'): 
                self._handler._release()
                self._handler.instance_uri = None

            while not self.__stopped:
                time.sleep(0.1)
        
    def __str__(self):

        #s = '%s(uri="%s", host="%s")' % (self.__class__.__name__, self.__uri, self.__host)
        s = '%s("%s")' % (self.__class__.__name__, self.__uri) 
        
        return s
            
    __repr__ = __str__

def remote_call(debug, verbose, proxy, method, *args):
    """
    for long calls when connection can fail
    during the call
    
    """

    i = 1
    while i <= 5:

        try:
            proxy._release()
            result = getattr(proxy, method)(*args)            
            return result
        
        except Pyro.errors.ConnectionClosedError,e:

            if verbose:
                e = ''.join(Pyro.util.getPyroTraceback(e))
                
            if debug:
                print '%s\nPyroUtils.remote_call(%s): Connection to %s is lost, trying reconnect...' \
                  % (e, method, proxy)
                
            proxy.adapter.rebindURI(tries = 50)
            i += 1
