#!/usr/bin/env python

import sys,os,errno
import atexit
import random

from IMP.isd.FileBasedGrid import FileBasedGrid
from IMP.isd.hosts import create_host_list
import IMP.atom
import IMP.container
import IMP.isd
import IMP.isd.shared_functions as sf
from IMP.isd.Replica import ReplicaTracker

IMP.set_log_level(IMP.NONE)

###simulation settings
#where to output files
outfolder=os.path.join(os.getcwd(), 'results')
#temp dir
tmpdir = os.path.join(os.getcwd(),'tmp')
#number of replicas / hosts
nreps = 8
#lambda scaling distribution
lambda_1 = 1.0
lambda_N = 0.7
lambdas=[lambda_N*(lambda_1/lambda_N)**((float(nreps)-k)/(nreps-1)) 
        for k in xrange(1,nreps+1)]
#thermostat coupling constant (berendsen, in fs)
tau=[500.0]*nreps
#list of files relative to the current dir to copy over to all nodes
initpdb = "generated2.pdb"
charmmtop = "top.lib"
charmmpar =  "par.lib"
restraints = "NOE_HN-full_7A_sparse100.tbl"
filelist=[initpdb,charmmtop,charmmpar,restraints] #add whatever you want
#prefix of output files 
nums=[[os.path.join(outfolder,'p%02d' % (i+1))] for i in xrange(nreps)]
#number of gibbs sampling steps
n_gibbs = 3
#number of md steps
n_md = 10
#number of mc steps
n_mc = 10
#where to run sims
hostlist = ['localhost']*nreps

#misc
imppy = '/bongo1/home/yannick/imp_local/build-release/tools/imppy.sh'
src_path = '/bongo1/home/yannick/imp_local/build-release/build/lib/IMP/isd'
showX11 = False 
grid_debug = False
grid_verbose = False
X11_delay = 1.0
window_size = '80x25'

def mkdir_p(path):
    "mkdir -p, taken from stackoverflow" 
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST:
            pass
        else: raise

def launch_grid():
    hosts = create_host_list(hostlist,tmpdir)
    for host in hosts:
        #ugly hack
        host.init_cmd = imppy + ' !'
    grid = FileBasedGrid(hosts, src_path, showX11, X11_delay, grid_debug,
            grid_verbose)
    grid.shared_temp_path = True
    if showX11:
        grid.window_size = window_size
    grid.copy_files('./', filelist)
    #grid.copy_files(src_path,["shared_functions.py"])
    #register termination
    atexit.register(grid.terminate)
    #start grid on all nodes
    grid.start()
    return grid

def main():
    
    # launch grid
    print "launching grid"
    grid = launch_grid()

    #publish the shared object
    print "publishing sfo"
    sfo = sf.sfo()
    sfo_id = grid.publish(sfo)

    print "communication test"
    #get a worker to do a task
    proxy = grid.acquire_service(sfo_id)
    print proxy.hello().get()
    grid.release_service(proxy)

    #call init on all nodes
    print "initializing model"
    mkdir_p(tmpdir)
    mkdir_p(outfolder)
    requests = grid.broadcast(sfo_id, 'init_model', tmpdir, 
            initpdb, restraints)
    #wait til init is done
    results = grid.gather(requests)

    #turn off noise (works because IMP.NONE is picklable, being an int.
    grid.gather(grid.broadcast(sfo_id, 'set_checklevel', IMP.NONE))
    grid.gather(grid.broadcast(sfo_id, 'set_loglevel', IMP.NONE))

    # evaluate the score of the whole system (without derivatives)
    print "initial energy"
    proxy = grid.acquire_service(sfo_id)
    print proxy.m('evaluate',False).get()
    print "writing initial structure to file initial.pdb"
    proxy.write_pdb(os.path.join(outfolder,'initial.pdb')).get()
    grid.release_service(proxy)
    
    #berendsen 300K tau=0.5ps
    #perform two independent MC moves for sigma and gamma
    print "initializing simulation and statistics"
    grid.gather(grid.scatter(sfo_id, 'init_simulation', zip(lambdas, tau)))
    grid.gather(grid.scatter(sfo_id, 'init_stats', nums))
    replica = ReplicaTracker(nreps, lambdas, grid, sfo_id,
            logfile=os.path.join(outfolder,'replicanums.txt'))

    print "start gibbs sampling loop"
    for i in range(n_gibbs):
        print "gibbs step %d" % i
        print " md"
        grid.gather(grid.broadcast(sfo_id, 'do_md', n_md))
        print " mc"
        grid.gather(grid.broadcast(sfo_id, 'do_mc_and_update_stepsize', n_mc))
        print " swaps"
        replica.replica_exchange()
        print " stats"
        grid.gather(grid.broadcast(sfo_id, 'write_stats', i, n_mc))
        replica.write_rex_stats()

    print "terminating grid"
    grid.terminate()
    print "done."
    

if __name__ == '__main__':

    main()

    
