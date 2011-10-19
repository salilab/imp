##
## The Inferential Structure Determination (ISD) software library
##
## Authors:  Darima Lamazhapova and Wolfgang Rieping
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

import os, sys, string
from threading import Thread, Event, Lock
from Queue import Queue
from fnmatch import fnmatch
import py_compile




from hosts import abspath, remote_mkdirs
from utils import atexit_register, atexit_unregister,Pipe,average



class AbstractGrid(Thread):
    """
    Grid main functionality: manage services

    It contains list of servers, which provide services.
    When instance is published in the grid, corresponding servers object
    are created on each host and saved as list in the dictionary, with
    a key corresponding to their service_id.

    Servers has part specific for communication method
    (at the creation stage), thus have to be defined in derived classes.

    USAGE:

        g = DerivedGrid(..)
        g.copy_files(isd_src_path, ['*.pyc', '*.so'])

        some_obj_instance = some_obj_class(..)

        service_id = g.publish(some_obj_instance)

        -> a) assigns some_service_id to this instance
           b) creates servers that manage the execution of this instance
              in remote hosts: servers[some_service_id] = [server1, server2, ...]

        proxy = g.acquire_service(service_id) ## returns proxy object

        proxy.set_parameters(p)  # these functions will be performed sequentally
        proxy.f()                # and service won't be freed
        proxy.g()                # unless .release_service() is called

        g.release_service(proxy)

    """

    def __init__(self, hosts, src_path, display, X11_delay, debug, verbose, shared_temp_path):

        """
        not shared_temp_path: temp_path == None
        look for individual temp_paths in host list

        """

        Thread.__init__(self)

        self.verbose = verbose
        self.debug = debug
        self.debug_out = sys.stderr

        self.display = display
        self.X11_delay = X11_delay
        self.window_size = '30x10'

        ## list of Host objects

        self.hosts = hosts

        self.shared_temp_path = shared_temp_path

        self.src_files = {}

        #copy general files needed for all grids
        self.initialise(src_path, ['AbstractGrid','utils','hosts','logfile'])

        ## key = service_id
        ## self.servers[key] = list of servers providing this service
        ## self.queues[key]  = a queue with available servers for the service

        self.servers = {}
        self.queues = {}

        atexit_register(self.terminate)

    def dp(self, str):

        print >>self.debug_out, str
        self.debug_out.flush()

    def set_loader(self, src_path, loader_filename):
        "sets the name of the compiled python file of the loader"

        if os.path.exists(os.path.join(src_path, '%s.pyo' % loader_filename)):
            self.loader_filename = '%s.pyo' % loader_filename
        else:
            self.loader_filename = '%s.pyc' % loader_filename

    def set_debug(self, debug, verbose = None):

        self.debug = debug
        if not verbose == None: self.verbose = verbose

    def publish(self, instance):
        """
        Publish instance on all the hosts of the grid
        (specific for the communication method)

        Returns: service_id

        Internally:

        It creates list of servers performing the job
        specified by this service_id

        """
        raise NotImplementedError

    def create_service_id(self, instance):

        return string.join( (instance.__class__.__name__, str(abs(id(instance)))), '.' )

    def add_server(self, server): # from AbstractISDGrid
        """
        (for internal use)

        Registers the server within a grid

        1) register itself (grid) in the server
        2) proxy must have _selfrelease attribute to be able
           to release service (i.e. put the connected to it server
           in the .queues[service_id]), once the last function
           call before invoking .release_service() has finished
           calculations

        self.queues: a dict[DerivedServer.service_id]=Queue(-1) containing the DerivedServers
        self.servers:  dict[DerivedServer.service_id]=[list of DerivedServer instances]
        """

        server.grid = self
        server.proxy.__dict__['_selfrelease'] = False

        service_id = server.service_id

        if not service_id in self.queues:
            self.queues[service_id] = Queue(-1)
            self.servers[service_id] = []

        self.queues[service_id].put(server)
        self.servers[service_id].append(server)

        if self.debug:
            self.dp('AbstractGrid.add_server: %s' % server)

    def find_server(self, service_id):
        """
        (for internal use)

        Selects a server that is capable of executing
        the requested service. If they are all busy,
        waits until there will be one available.

        """

        if not service_id in self.queues:
            raise StandardError, 'Service type "%s" not known.' % service_id

        queue = self.queues[service_id]

        ## if there is no servers in the queue, waits until it is in the queue
        server = queue.get()

        return server

    def acquire_service(self, service_id):
        """
        For a given service, it finds available server and
        returns corresponding proxy worker.

        """

        server = self.find_server(service_id)
        server.jobs += 1
        server.acquired = True

        if self.debug and self.verbose:
            msg = 'Grid.acquire_service: service "%s" on host "%s" acquired [%s]'
            self.dp( msg % (service_id, server.host, server.proxy) )

        return server.proxy

    def release_service(self, proxy):
        """
        Used to inform server/proxy that we are done using the service,
        and it should be released, when the last funciton call of the
        proxy is finished.

        However, it means that someone (the proxy or the grid)
        will have to call _release_service() itself and also
        restore ._selfrelease to the previos state (FALSE)

        """

        proxy._selfrelease = True

    def _release_service(self, proxy):

        ## we put server back in the queue only if it is still
        ## in the list of grid servers (it may already be deleted from
        ## self.servers, if it is planned to be terminated/deleted)

        proxy._server.acquired = False

        if proxy._server in self.servers[proxy._server.service_id]:
            self.queues[proxy._server.service_id].put(proxy._server)

        if self.debug and self.verbose:
            msg = 'Grid.release_service: service "%s" on host "%s" released [%s]'
            self.dp( msg % (proxy._server.service_id, proxy._server.host, proxy) )

    def add_time(self, proxy, time):

        proxy._server.time.put(time)

    def copy_files(self, rootdir, filelist, hosts = None,
            exclude = ['cns/','.svn/']):
        """
        (for internal use)

        Copies source files to a common folder so that
        they were accessible to the grid on remote hosts

        g = DerivedGrid(...)

        src_path = os.path.join(os.environ['ISD_ROOT'],'/src/py')

        from compileall import compile_dir
        compile_dir(src_path, maxlevels=0, force=True, quiet=True)

        g.copy_files(src_path, ['*.pyc','*.so'])

        The method will replicate the directory structure of
        items in the filelist, e.g. will create data folder
        in the destination for

        g.copy_files(src_path, ['data/*.txt'])

        also, it will copy all subfolders matching the required name, i.e. it
        will copy all files called foo.txt from all existing subfolders it can
        find

        g.copy_files(src_path, ['Isd/foo.txt'])
        will copy files like 'Isd/foo.txt', 'Isd/a/foo.txt', 'Isd/a/b/c/foo.txt' etc.

        the exclude keyword excludes paths containing the given strings.

        """

        if hosts == None: hosts = self.hosts
        elif type(hosts).__name__ == 'Host': hosts = [hosts]

        if self.debug:

            print 'AbstractGrid.copy_files: filelist = %s' % filelist

            if self.shared_temp_path:
                print 'AbstractGrid.copy_files: to %s' % self.hosts[0].temp_path
            else:
                for host in hosts:
                    print 'AbstractGrid.copy_files: to %s on %s' % (host.temp_path, host.name)

        subroots = {}

        #create necessary folders and store files to be copied in a
        #dictionnary: subroots['path/to']=[list, of, files]
        cwd = os.getcwd()
        os.chdir(rootdir)
        try:

            for f in filelist:

                folder, pattern = os.path.split(f)
                if folder=='': folder = './'
                for root,useless,filematch in os.walk(folder):

                    for file in filematch:
                        fullfile = os.path.join(rootdir, root, file)
                        if fnmatch(file, pattern) and (
                                not False in [fullfile.find(ex) < 0 for ex in exclude] ):
                            if not root in subroots:
                                self.create_subroot(root)
                                subroots[root]=[]
                            subroots[root].append(os.path.join(rootdir, root, file))

            for subroot, subfilelist in subroots.items():

                _from = string.join(subfilelist)

                if self.shared_temp_path:

                    _to = os.path.join(self.hosts[0].temp_path, subroot)
                    os.system('cp %s %s' % (_from, _to))

                else:

                    for host in hosts:

                        _to = os.path.join(host.temp_path, subroot)

                        os.system('scp -r %s %s:%s > /dev/null' % (_from, host.name, _to))

                        if self.debug:
                            print 'Host %s: done. (%s)' % (host.name, _from)
            self.copied_files = subroots
            try:
                self.others = {'rootdir':rootdir,'root':root, 'files':files}
            except:
                pass

        finally:
            os.chdir(cwd)

    def create_subroot(self, subroot):

        if self.shared_temp_path:

            tempdir = os.path.join( self.hosts[0].temp_path, subroot)

            if not os.path.exists(tempdir):
                os.makedirs(tempdir)

        else:

            for host in self.hosts:

                tempdir = abspath(os.path.join(host.temp_path, subroot))

                if not tempdir in self.temp_paths[host.name]:

                    remote_mkdirs(host, tempdir, False)

                    while not tempdir in self.temp_paths[host.name]:

                        self.temp_paths[host.name].append(tempdir)
                        tempdir = os.path.split(tempdir)[0]

    def initialise(self, src_path, src_files):
        """
        Initialise the grid:
        Create temp paths if needed, either shared or remotely on each host
        Copy source files to this folder
        """

        ## create temporary directories if needed

        if self.shared_temp_path:
            if not os.path.exists(self.hosts[0].temp_path):
                os.makedirs(self.hosts[0].temp_path)

        else:
            if not hasattr(self, 'temp_paths'): self.temp_paths = {}

            for host in self.hosts:
                if not host.name in self.temp_paths:

                    remote_mkdirs(host, host.temp_path, self.debug)
                    self.temp_paths[host.name] = [host.temp_path,] ## saves folders created remotely

        ## copy source files

        compiled_files = []

        for f in src_files:
            py_compile.compile(os.path.join(src_path, '%s.py' % f))

            if os.path.exists(os.path.join(src_path, '%s.pyo' % f)):
                compiled_files += ['%s.pyo' % f]
            else:
                compiled_files += ['%s.pyc' % f]

        self.copy_files(src_path, compiled_files)

    def ishalted(self):
        raise NotImplementedError

    def terminate(self, service_id = None):

        ## if the grid is already terminated (e.g. by hands)
        ## then function is not called on the exit

        if self.ishalted(): return

        atexit_unregister( self.terminate )

        if service_id == None: service_ids = self.servers.keys()
        else: service_ids = [service_id]

        for service_id in service_ids:

            servers = self.servers[service_id]

            while len(servers) > 0:

                if self.debug:
                    print 'AbstractGrid.terminate: terminating %s' % servers[-1]

                ## 1. prevents server from being acquired
                server = servers.pop()

                ## 2. terminates server (and corresponding proxy) if needed
                server.terminate()

        if self.debug: ## PLEASE: keep debug statement. PLEASE!
            print 'AbstractGrid: terminated.'

class Server:
    """
    Contains all the information that is required
    to use a remote object (alone and within a grid)

    Also it does all the job to launch the instance on remote side
    and wrap it within a Grid specific Proxy object

    1) self.service_id
    2) self.proxy

    3) self.host

    """

    def __init__(self, proxy, service_id, host, debug):

        self.debug = debug
        self.debug_out = sys.stdout

        self.proxy = proxy
        self.service_id = service_id
        self.host = host

        ## register itself (server) in the proxy

        self.proxy.__dict__['_server'] = self

        ## variables for load balancing

        self.time = Pipe(50)
        self.time.put(0.)
        self.jobs = 0          ## (log) number of jobs done by the Server
        self.acquired = False  ## (log)

    def dp(self, str):

        print >>self.debug_out, str
        self.debug_out.flush()

    def terminate(self):
        """
        It terminates self.proxy if needed

        """
        raise NotImplementedError

    def __str__(self):

        s = '%s(busy=%s, <time>=%.2f, jobs=%d, host="%s", proxy="%s"' \
             % (self.__class__.__name__, self.acquired, average(self.time), self.jobs, \
                self.host.name, self.proxy)

        return s

    __repr__ = __str__

class TimeoutError(Exception):
    pass

class Result(object):
    """
    A Result object is returned by a Proxy object, when some remote
    function invoked.

    When results of the remote function calculations are ready,
    the .event is set and the result of calculations is stored
    in the .value field. These has to be done externally,
    for example in the Proxy object.

    USAGE:

    result = Result()

    ## assigning result value

    result.value = result of the function call
    result.event.set()

    ## collecting the result

    result.get(timeout = 60)

    """

    def __init__(self, proxy):

        self.event = Event()

        ## register proxy in the Result object it has produced

        self.proxy = proxy    ## used in FBGrid, in PyroGrid for debugging only

    def get(self, timeout = None):

        self.event.wait(timeout)

        if timeout is not None:

            if not self.event.isSet():

                raise TimeoutError

        return self.value

    def __str__(self):

        sx = []
        if hasattr(self, 'proxy'): sx += ['proxy="%s"' % self.proxy]
        if hasattr(self, 'value'): sx += ['value=%s' % self.value]

        s = '%s(%s)' % ( self.__class__.__name__, string.join(sx, ',') )

        return s

    def __del__(self):
        pass

    __repr__ = __str__

class AbstractService(object):
    """
    Wrapper around Grid services, facilitating non parameter-specific
    usage of the remote objects.

    AbstractService resides on the local side and is parameter-specific.
    It provides the interface for remote services using grid.

    self._set_methods - are used to set up parameters specific to the
                        local service to the remote objects

    self._get_methods - are used to get updated parameters from remote side

    self._parameters  - parameters that have to be set remotely
                        1. kept on local side
                        2. sets to the remote object before each call

    Example:

    heatbath = GridService(grid, service_id)
    heatbath.parameters = {'T': 30, etc}
    heatbath.set_methods = {'T': 'set_temperature' }
    heatbath.get_methods = {'T': 'get_temperature' }
    heatbath.generate_sample(x,y,z)

    """

    def __init__(self, grid, service_id, debug = False,
                 return_remote_attributes = False):

        object.__init__(self)

        if not service_id in grid.queues:
            raise StandardError, 'Service type "%s" not known in the grid' % service_id

        self.service_id = service_id
        self.debug = debug

        self._set_methods = None
        self._get_methods = None
        self._parameters  = {}

        self.__proxy_lock = Lock()       ## protects PyroProxy from overwriting

        self.__parameters_last = {}      ## results with parameters from previous call
        self.__parameters_lock = Lock()

        self.__return_remote_attributes = return_remote_attributes

        self.grid = grid

    def get_parameter(self, attr_name):
        """
        Makes sure that the parameters requested (locally)
        were updated before being returned

        Example: (in derived class)

        >> def get_temperature(self):
        >>    return self.get_parameter('T')

        """
        if self._get_methods is not None:
            self.__get_parameters()

        return self._parameters[attr_name]

    def __get_parameters(self):

        self.__parameters_lock.acquire()

        for attr_name in self.__parameters_last:
            self._parameters[attr_name] = self.__parameters_last[attr_name].get()

        self.__parameters_last = {}

    def __proxy_acquire(self):

        if self.debug:
            print 'AbstractService.__proxy_acquire(): %s' % (self.service_id)

        self.__proxy_lock.acquire()

        self.proxy = self.grid.acquire_service(self.service_id)

    def __proxy_release(self):

        if self.debug:
            print 'AbstractService.__proxy_release(): ' % (self.proxy)

        self.grid.release_service(self.proxy)
        self.proxy = None

        self.__proxy_lock.release()

    def __proxy_release_now(self):

        self.grid._release_service(self.proxy)
        self.proxy = None

        self.__proxy_lock.release()

    def __getattr__(self, name):

        # use these mechanism only when __init__() is completed
        # all the new attribute calls and declarations will go through
        # to remote methods

        if not 'grid' in self.__dict__:
            return getattr(object, name)

        self.__proxy_acquire()

        if self.debug:
            print 'AbstractService.__getattr__(%s): acquired %s [%s]' % (name, self.proxy, id(self))

        try:
            attr = getattr(self.proxy, name)

        except:
            self.__proxy_release_now()
            raise

        if callable(attr):

            if self.debug:
                print 'AbstractService.__getattr__(%s): is callable' % name

            return lambda *args: self.__call_method(name, *args)

        else:
            if self.debug:
                print 'AbstractService.__getattr__(%s): is not callable' % name

            if self.__return_remote_attributes:

                attr = getattr(self.proxy, name)

                self.__proxy_release_now()

                return attr

            else:
                raise 'AbstractService must not return remote attributes ' +\
                      '(because remote workers are parameter not specific)'

    def __call_method(self, name, *args):

        if self.debug:
            print 'AbstractService.__call_method: ', self.proxy

        ## renew parameters from the results of the last call

        if self._get_methods is not None:

            if self.debug:
                print 'AbstractService.__call_method(%s): renew local parameters [%s]' % (name, id(self))

            self.__get_parameters()

            self.__parameters_lock.acquire() ## now parameters may change

        ## set up remote parameters

        for attr_name, set_method in self._set_methods.items():
            getattr(self.proxy, set_method)( self._parameters[attr_name] )

        ## actual call (immediately returns a Result object)

        if self.debug:
            print 'AbstractService.__call_method(%s): actual call %s [%s]' % (name, self.proxy, id(self))

        try:
            result = getattr(self.proxy, name)(*args)
            result.proxy = self.proxy

        except:
            self.__proxy_release()
            raise

        ## requests for changed parameters and releases service
        ## to the grid (when remote job is finished)

        if self._get_methods is not None:

            if self.debug:
                print 'AbstractService.__call_method(%s): parameter call %s [%s]' % (name, self.proxy, id(self))

            t = Thread(target = self.__call_parameters, args = ())
            t.start()

        else:
            self.__proxy_release()

        if self.debug:
            print 'AbstractService.__call_method(%s): returning result %s [%s]' % (name, self.proxy, id(self))

        return result

    def __call_parameters(self):

        ## waits until the remote job is finished and
        ## then returns remote parameters in a Result
        ## object (self.__parameters_last[attr_name])

        for attr_name, get_method in self._get_methods.items():
            self.__parameters_last[attr_name] = getattr(self.proxy, get_method)()

        self.__parameters_lock.release()

        if self.debug:
            print 'AbstractService.__call_parameters: releasing %s [%s]' % (self.proxy, id(self))

        ## releases service to the grid

        self.__proxy_release()
