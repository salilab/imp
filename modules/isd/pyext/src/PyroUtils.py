##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Darima Lamazhapova and Wolfgang Rieping
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
import sys
from socket import gethostname
from string import split, join
from threading import Lock, Thread
from time import time, sleep

import Pyro.naming
import Pyro.core
from Pyro.errors import PyroError, NamingError, ConnectionDeniedError
from Pyro.naming import NameServerProxy

default_debug = False

default_group = ':%s' % os.environ['USER']
Pyro.config.PYRO_NS_DEFAULTGROUP = default_group

if 'PYROGRID_TIMEOUT' in os.environ:
    default_timeout = eval(os.environ['PYROGRID_TIMEOUT'])
else:
    default_timeout = 600.0

if 'PYROGRID_WAIT' in os.environ:
    default_wait = eval(os.environ['PYROGRID_WAIT'])
else:
    default_wait = 5.0

def absolute_uri(uri, ns = None):

    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host=ns)

    return ns.fullName(uri)

def group_uri_list(group_uri, ns = None):
    """
    Returns the list of all URIs given the group_uri

    """
    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host=ns)

    level_0 = group_uri

    try:
        lst = ns.list(level_0) ## level_0 is a group

        level_1 = [ join( (level_0, lst[i][0]), '.' ) for i in range(0, len(lst)) ]
        level_1_expanded = []

        for level_1_i in level_1:
            level_1_expanded += group_uri_list(level_1_i, ns)

    except NamingError:

        try:
            ns.resolve(level_0)
            level_1_expanded = [ns.fullName(level_0)]  ## level_0 is not a group, it is full URI

        except NamingError:
            level_1_expanded = [] ## level_0 is not registered with ns

    return level_1_expanded

def _new_id(xlst):
    """
    returns a number which is not in xlst

    """


    if len(xlst) == 0: x = 0    ## 'obj_name.0'
    else:                       ## 'obj_name.x'
        x = len(xlst)
        xlst.sort()
        for i in range(len(xlst)):
            if xlst[i] != i:
                x = i
                break
    return x

def create_unique_uri(obj_name, n = 1, ns = None):
    """
    obj_name = e.g. URI will be 'obj_name.6'
    uri_list = group_uri_list(obj_name)

    Create unique name 'obj_name.x' (the name which is not in uri_list)

    """
    if default_debug:
        print 'PyroUtils.create_unique_uri: %s URIs for %s...' % (n, obj_name)

    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host=ns)

    old_uri_list = group_uri_list(obj_name, ns)

    xlst = [] ## list of 'x' components of 'smth.obj_name.x' of URI in  uri_list
    for uri in old_uri_list: xlst.append( eval(uri.split('.')[-1]) )

    new_uri_list = []

    for i in range(n):

        x = _new_id(xlst)
        new_uri_list.append( ns.fullName(join( (obj_name, str(x)), '.' )) )
        xlst.append(x)

    return new_uri_list

def create_groups(obj_uri, ns = None):
    """
    Creates groups for the obj_uri

    Example: for 'factory.horse--bio.1'
             creates groups factory and factory.horse--bio

    """

    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host=ns)

    try: ns.createGroup(default_group)
    except NamingError: pass

    groups = obj_uri.split('.')[:-1]

    for i in range(1, len(groups)):
        groups[i] = join( groups[i-1:i+1], '.')

    for i in range(0, len(groups)):
        try:
            ns.createGroup(groups[i])
        except NamingError:
            pass

def is_stopped(obj_uri, timeout = default_timeout, ns = None):
    """
    Checks if the Pyro remote object with URI obj_uri
    has been stopped (gracefully).

    """
    if default_debug:
        print 'PyroUtils.is_stopped: for %s...' % obj_uri

    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host = ns)


    stopped = False
    t0 = time()

    while (not stopped) and time()-t0 < timeout:
        try:
            ns.resolve(obj_uri)
        except Pyro.errors.NamingError:
            stopped = True
            pass

    if stopped:
        return True
    else:
        print 'PyroUtils.is_stopped: Object with URI %s could not ' +\
              'be shut down during timeout' % obj_uri
        return False

def launch_instance(obj_instance, obj_uri, delegate, \
                    nshost = None, debug = True, shutdown = False):
    """
    Launches obj_instance as Pyro daemon on the current host.

    obj_instance = the class of the Pyro implementation object
    obj_uri      = URI for the object to register in the Name Server
                   <- create_uri(obj_name, nshost)
    delegate     = whether to use the delegate approach or the regular
                   objBase subclassing approach

    Function also assigns:

    obj_instance._pyro_stop = False
    obj_instance._pyro_suppl['uri'] = obj_uri
    ...

    """

    if debug:
        print 'PyroUtils.launch_instance: %s, URI %s, PID %s on %s' \
              % (obj_instance, obj_uri, os.getpid(), gethostname())

    ## initialize the server and set the default namespace group

    Pyro.config.PYRO_MOBILE_CODE = 1        #TODO: this should spare us
                                        #the copying of all files at the initialization
                                        #of PyroGrid()
    Pyro.core.initServer()

    ## locate the NS

    if nshost is None or nshost == '':
        ns = Pyro.naming.NameServerLocator().getNS()
    else:
        ns = Pyro.naming.NameServerLocator().getNS(host=nshost)

    try:
        ns.resolve(obj_uri)
    except NamingError: pass
    else:
        print 'Object with URI %s already exists, unregister first!' % obj_uri
        raise

    create_groups(obj_uri, ns)

    ## start daemon

    if debug:
        print 'launching pyro daemon and nameserver'
    daemon = Pyro.core.Daemon()
    daemon.useNameServer(ns)

    ## connect a new object implementation (first unregister previous one)

    obj_instance._pyro_stop = False  ## used to stop execution
    obj_instance._pyro_suppl = {}
    obj_instance._pyro_suppl['uri'] = obj_uri         ## to monitor the status of the
    obj_instance._pyro_suppl['nshost'] = nshost       ## proxy (stopped or running)
    obj_instance._pyro_suppl['host'] = gethostname()  ## (for debugging)

    obj_instance._pyro_suppl['callable'] = {}
    for attr in dir(obj_instance):
        obj_instance._pyro_suppl['callable'][attr] = callable(getattr(obj_instance,attr))

    if isinstance(obj_instance, type(Thread())):
        obj_instance.start()

    #print 'PyroUtils.launch_instance: Connecting to a daemon...'

    if delegate:
        #print 'PyroUtils.launch_instance: Delegating...'
        obj = Pyro.core.ObjBase()
        obj.delegateTo(obj_instance)
        daemon.connectPersistent(obj, obj_uri)
    else:
        ## obj_instance in this case must be inherited from Pyro.core.ObjBase
        daemon.connectPersistent(obj_instance, obj_uri)

    print 'PyroUtils.launch_instance: Remote object "%s" is ready' % obj_uri

    try:
        #listen for requests as long as condition returns True
        #by default, each loop lasts 3 seconds
        daemon.requestLoop(condition = lambda:
                       not obj_instance._pyro_stop)
    except KeyboardInterrupt:
        pass

    print 'PyroUtils.launch_instance: Shutting down %s gracefully' % obj_uri

    #should not need this since the daemon does this anyway
    if delegate:
        daemon.disconnect(obj)
    else:
        daemon.disconnect(obj_instance)

    if shutdown:
        ## This will cleanly stop all threads that might
        ## still be running.
        daemon.shutdown()
        print 'Exiting'
    else:
        print 'Finished, keeping interperter alive and Pyro daemon running'

def get_proxy(obj_uri, ns = None, attrproxy = True, timeout = default_timeout):

    if default_debug:
        print 'PyroUtils.get_proxy: "%s" using nameserver "%s"...' \
              % (obj_uri, ns)

    Pyro.config.PYRO_MOBILE_CODE = 1

    Pyro.core.initClient()

    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host=ns)

    t0 = time()
    resolved = False

    while not resolved:
        try:
            pyro_uri = ns.resolve(obj_uri)
            resolved = True

        except Pyro.errors.NamingError, x:

            if time()-t0 < timeout:
                sleep(default_wait)

            else:
                print 'PyroUtils.get_proxy: could not locate object "%s" using "%s"' \
                      % (obj_uri, ns)
                raise

    if default_debug:
        print 'PyroUtils.get_proxy: returning the proxy "%s"' % pyro_uri

    if attrproxy:
        return pyro_uri.getAttrProxy()
    else:
        return pyro_uri.getProxy()

def get_nameserver(nshost):

    if nshost is None or nshost == '':
        return Pyro.naming.NameServerLocator().getNS()
    else:
        return Pyro.naming.NameServerLocator().getNS(host = nshost)

def unregister(obj_uri, ns = None):

    if default_debug:
        print 'PyroUtils.unregister: Unregistering %s...' % obj_uri

    if not isinstance(ns, NameServerProxy):
        if ns is None or ns == '':
            ns = Pyro.naming.NameServerLocator().getNS()
        else:
            ns = Pyro.naming.NameServerLocator().getNS(host = ns)

    ns.unregister(obj_uri)
