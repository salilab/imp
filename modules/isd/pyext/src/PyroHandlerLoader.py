##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Darima Lamazhapova and Wolfgang Rieping
##
##          Copyright (C) Michael Habeck, Wolfgang Rieping
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

__doc__ = """This script must be launched on the host where a calculation
is sought to be made; It forks a thread that runs the pyro handler.

Arguments (in order):
    niceness (int)
    nameserver's hostname (string, or None to broadcast for it)
    uri of this instance's PyroHandler (string)
    timeout (int)
    debug token (bool)
    display X11 window (bool)
    temp path (string)
"""

if __name__ == '__main__':

    import os, sys, socket
    from threading import Thread
    from IMP.isd.PyroGrid import PyroHandler
    from IMP.isd.PyroUtils import launch_instance #, get_proxy


    # niceness = 19
    # nshost = 'horse--bio'
    # obj_name = 'PyroHandler.0'
    # debug = True

    #Profile=False
    #if Profile:
    #    import yappi
    #    yappi.start()

    niceness = eval(sys.argv[1])
    try:
        nshost = eval(sys.argv[2])
    except:
        nshost = sys.argv[2]

    uri = sys.argv[3]
    timeout = eval(sys.argv[4])
    debug = eval(sys.argv[5])
    display = eval(sys.argv[6])
    temp_path = sys.argv[7]

    os.environ['ISD_ROOT'] = temp_path

    if not temp_path in sys.path:
        sys.path.insert(0, temp_path)

    os.nice(niceness)

    print 'PyroHandlerLoader: %s (%s)' % (uri, socket.gethostname())

    #following command also forks a WatchDog thread via PyroHandler.__init__
    handler_instance = PyroHandler(timeout, nshost, debug)

    print 'PyroHandlerLoader: starting launch_instance in a thread'

    #the delegate approach is not used since PyroHandler is inherited from Pyro.core.ObjBase
    t = Thread(target = launch_instance, \
               args = (handler_instance, uri, False, \
                       nshost, debug, not display))
    t.start()

    print 'PyroHandlerLoader: thread has been launched'

    ## get_proxy: initializes Pyro client on server side

    ## handler = get_proxy(uri, nshost)
    ## print 'handler = \nPyroUtils.get_proxy\n(\'%s\',\n \'%s\')\n' % (uri, nshost)


    #if Profile:
    #    import time
    #    fl=open("/Bis/home/yannick/simulations/test_isd_ww/profiled_"+str(int(time.time())),'w')
    #    for stat in yappi.get_stats(): fl.write(stat+"\n")
    #    fl.close()
    #    yappi.stop()
