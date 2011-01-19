#!/usr/bin/env python

import sys,os
import atexit
from IMP.isd.FileBasedGrid import FileBasedGrid
from IMP.isd.hosts import create_host_list

import IMP.atom
import IMP.container
import IMP.isd
import IMP.isd.shared_functions as sf

IMP.set_log_level(IMP.NONE)

imppy = '/bongo1/home/yannick/impisd/build-fast/tools/imppy.sh'

def launch_grid():
    hostlist = ['localhost']
    tmpdir = '/tmp'
    src_path = '/bongo1/home/yannick/impisd/src/isd/pyext/src'
    showX11 = True 
    X11_delay = 1.0
    grid_debug = True 
    grid_verbose = False
    window_size = '80x25'
    hosts = create_host_list(hostlist,tmpdir)
    for host in hosts:
        #ugly hack
        host.init_cmd = imppy + ' !'
    grid = FileBasedGrid(hosts, src_path, showX11, X11_delay, grid_debug,
            grid_verbose)
    grid.shared_temp_path = True
    if showX11:
        grid.window_size = window_size
    grid.copy_files('./',["generated2.pdb","top.lib",
        "par.lib","NOE_HN-full_7A_sparse100.tbl"])
    #grid.copy_files(src_path,["shared_functions.py"])
    #register termination
    atexit.register(grid.terminate)
    #start grid on all nodes
    grid.start()
    return grid

def propagate_call(grid, sfo_id, funcname, *args, **kw):
    results = []
    for server in grid.servers[sfo_id]:
        func=getattr(server.proxy, funcname)
        results.append(func(*args, **kw))
    return results

def get_result(results):
    retval=[]
    for server in results:
        retval.append(server.get())
    return retval

if __name__ == '__main__':

    # launch grid
    grid = launch_grid()

    #publish the shared object
    sfo = sf.sfo()
    sfo_id = grid.publish(sfo)

    proxy = grid.acquire_service(sfo_id)
    print proxy.hello().get()
    grid.release_service(proxy)

    #call init on all nodes
    requests = propagate_call(grid, sfo_id, 'init_simulation')
    #wait til init is done
    results = get_result(requests)

    # Finally, evaluate the score of the whole system (without derivatives)
    proxy = grid.acquire_service(sfo_id)
    print proxy.m('evaluate',False).get()
    proxy.write_pdb('out.pdb').get()
    grid.release_service(proxy)
    
    results=[]
    for server in grid.servers[sfo_id]:
        results.append(server.proxy.setup_md())
    #wait til init is done
    sfo.init_simulation()
    for server in results:
        server.get()
    proxy.setup_md(300.0,500.0) #berendsen 300K tau=0.5ps

    #perform two independent MC moves for sigma and gamma
    (mc_sigma,nm_sigma) = setup_mc(sigma)
    (mc_gamma,nm_gamma) = setup_mc(gamma)

    stat=simstats()
    n_gibbs = 100
    n_md = 500
    n_mc = 50
    for i in range(n_gibbs):
        print "md step %d" % i
        print " md"
        do_md(n_md)
        print " mc"
        do_mc_and_update_stepsize(n_mc)
        print " stats"
        stat.write_stats(i,n_mc)



    #grid.terminate()
    #grid.release_service(proxy)
    

