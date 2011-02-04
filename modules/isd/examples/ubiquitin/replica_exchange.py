#!/usr/bin/env python

import sys,os,errno
import atexit
import random

# Comment either this line or the following to attain different communication schemes:
# to start pyro
from IMP.isd.PyroGrid import PyroGrid as Grid
#from IMP.isd.FileBasedGrid import FileBasedGrid as Grid
from IMP.isd.hosts import create_host_list
import IMP.atom
import IMP.container
import IMP.isd
from IMP.isd.Replica import ReplicaTracker

import shared_functions as sf

IMP.set_log_level(IMP.NONE)

###simulation settings
#where to output files
outfolder=os.path.join(os.getcwd(), 'results')
#temp dir
tmpdir = os.getenv('IMP_TMP_DIR')
#number of replicas / hosts
nreps = 8
#lambda scaling distribution
kB= (1.381 * 6.02214) / 4184.0
lambda_1 = 1.0
lambda_N = 0.8
lambdas=[lambda_N*(lambda_1/lambda_N)**((float(nreps)-k)/(nreps-1)) 
        for k in xrange(1,nreps+1)]
#thermostat coupling constant (berendsen, in fs)
tau=[500.0]*nreps
#stat_rate is the rate at which to print out traj statistics
stat_rate=[10]*nreps
#list of files relative to the current dir to copy over to all nodes
initpdb = "generated2.pdb"
charmmtop = "top.lib"
charmmpar =  "par.lib"
restraints = "NOE_HN-full_7A_sparse100.tbl"
filelist=[initpdb,charmmtop,charmmpar,restraints] #add whatever you want
#prefix of output files 
nums=[[os.path.join(outfolder,'r%02d' % (i+1))] for i in xrange(nreps)]
#number of gibbs sampling steps in  the first temperature relaxation
n_gibbs1 = 10
#number of gibbs sampling steps in  the second temperature relaxation
n_gibbs2 = 10
#number of gibbs sampling steps in  the production temperature relaxation
n_gibbs3 = 10
#number of md steps
n_md = 1000
#number of mc steps (not < 50 because adaptive)
n_mc = 100
#where to run sims
hostlist = ['localhost']*nreps
#replica exchange scheme
rex_scheme='convective'
#replica exchange exchange method
rex_xchg='gromacs'
#whether to use TuneRex to tune temperatures
tune_temps = True
tune_data = {'rate' : 100, #temp optimization rate, in rex steps.
        'method':'ar', #either "ar" acceptance ratio optimization, or "flux" optimization
        'alpha':0.05} # type I error on the estimates
templog = os.path.join(outfolder, 'temps.txt')
rexlog = os.path.join(outfolder,'replicanums.txt')

#misc

imppy = '/mandolin1/home/cgreen/imp/tools/imppy.sh'
src_path = '/mandolin1/home/cgreen/imp/modules/isd/pyext/src'
#imppy = '/bongo1/home/yannick/imp_local/build-fast/tools/imppy.sh'
#src_path = '/bongo1/home/yannick/imp_local/build-fast/build/lib/IMP/isd'
showX11 = False
grid_debug = False
grid_verbose = False
X11_delay = 1.0
window_size = '80x25'
#pyroGrid
shared_temp_path = True
terminate_during_publish = False
nshost = 'localhost'

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
	
    #pyro grid
    grid = Grid(hosts, src_path, showX11, X11_delay, grid_debug,
            grid_verbose, shared_temp_path, nshost, terminate_during_publish)
	    
    #uncomment the following lines and comments the two previous ones to use file based grid	
    #file based grid
    #grid = Grid(hosts, src_path, showX11, X11_delay, grid_debug,
    #        grid_verbose)
    #grid.shared_temp_path = shared_temp_path	    
    if showX11:
        grid.window_size = window_size
    grid.copy_files('./', filelist)
    #grid.copy_files(src_path,["shared_functions.py"])
    
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
    #proxy = grid.acquire_service(sfo_id)
    #print proxy.hello().get()
    #grid.release_service(proxy)

    print "broadcast test"
    print grid.gather(grid.broadcast(sfo_id, 'hello'))

    #call init on all nodes
    print "initializing model"
    mkdir_p(tmpdir)
    mkdir_p(outfolder)
    requests = grid.broadcast(sfo_id, 'init_model', tmpdir, initpdb, restraints)
    #wait til init is done
    results = grid.gather(requests)

    #turn off verbose noise (works because IMP.NONE is picklable, being an int.
    grid.gather(grid.broadcast(sfo_id, 'set_checklevel', IMP.NONE))
    grid.gather(grid.broadcast(sfo_id, 'set_loglevel', IMP.NONE))

    # evaluate the score of the whole system (without derivatives, False flag)
    print "initial energy"
    grid.gather(grid.broadcast(sfo_id, 'm', 'evaluate', False))
    
    #berendsen 300K tau=0.5ps
    #perform two independent MC moves for sigma and gamma
    print "initializing simulation and statistics"
    grid.gather(grid.scatter(sfo_id, 'init_simulation', 
        zip(lambdas, tau)))
    grid.gather(grid.scatter(sfo_id, 'init_stats', nums))
    replica = ReplicaTracker(nreps, lambdas, grid, sfo_id,
            rexlog=rexlog,
            scheme=rex_scheme, xchg=rex_xchg, 
            tune_temps=tune_temps, tune_data=tune_data, templog=templog)

    print "start gibbs sampling loop: first relaxation"
    for i in range(n_gibbs1):
        print "\rgibbs step %d" % i,
        sys.stdout.flush()
        #print " md"
        grid.gather(grid.broadcast(sfo_id, 'do_md', n_md))
        #print " mc"
        grid.gather(grid.broadcast(sfo_id, 'do_mc_and_update_stepsize', n_mc))
        #print " swaps"
	replica.tune_data['dumb_scale']=0.5
        replica.replica_exchange()
        #print " stats"
        replica.write_rex_stats()

    print "start gibbs sampling loop: second relaxation"
    for i in range(n_gibbs2):
        print "\rgibbs step %d" % i,
        sys.stdout.flush()
        #print " md"
        grid.gather(grid.broadcast(sfo_id, 'do_md', n_md))
        #print " mc"
        grid.gather(grid.broadcast(sfo_id, 'do_mc_and_update_stepsize', n_mc))
        #print " swaps"
	replica.tune_data['dumb_scale']=0.2
        replica.replica_exchange()
        #print " stats"
        replica.write_rex_stats()

    print "start gibbs sampling loop: production"
    for i in range(n_gibbs3):
        print "\rgibbs step %d" % i,
        sys.stdout.flush()
        #print " md"
        grid.gather(grid.broadcast(sfo_id, 'do_md', n_md))
        #print " mc"
        grid.gather(grid.broadcast(sfo_id, 'do_mc_and_update_stepsize', n_mc))
        #print " swaps"
	replica.tune_temps=False
        replica.replica_exchange()
        #print " stats"
        replica.write_rex_stats()

    print "terminating grid"
    grid.terminate()
    print "done."
    

if __name__ == '__main__':

    main()

    
