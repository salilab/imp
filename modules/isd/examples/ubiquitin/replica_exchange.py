#!/usr/bin/env python

import sys,os,errno
import atexit
import random

# Comment either this line or the following to attain different communication schemes:
# to start pyro
try:
    from IMP.isd.PyroGrid import PyroGrid as Grid
#   from IMP.isd.FileBasedGrid import FileBasedGrid as Grid
except ImportError:
    print >> sys.stderr, "This example needs the Python Pyro module"
    sys.exit(0)
from IMP.isd.hosts import create_host_list
import IMP.atom
import IMP.container
import IMP.isd
from IMP.isd.Replica import ReplicaTracker

import shared_functions as sf

IMP.base.set_log_level(IMP.base.NONE)

###simulation settings
#where to output files
outfolder=os.path.join(os.getcwd(), 'results')
#temp dir
tmpdir=os.path.join(os.getcwd(), 'tmp')
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
#(in units of gibbs sampling steps)
stat_rate=[1]*nreps
#list of files relative to the current dir to copy over to all nodes
initpdb = "generated2.pdb"
charmmtop = "top.lib"
charmmpar =  "par.lib"
restraints = "NOE_HN-full_7A_sparse100.tbl"
sequence='sequence.dat'
#export the files in a local tmp directory
filelist=[initpdb,charmmtop,charmmpar,sequence,
          restraints,'shared_functions.py'] #add whatever you want
#prefix of output files
nums=[os.path.join(outfolder,'r%02d' % (i+1)) for i in xrange(nreps)]
#thermalization (mc parameters stay fixed)
n_therm = 100 #number of loops, where temperatures are scaled to target value
n_hmc_therm = 10 #number of steps per loop
#number of gibbs sampling steps in  the first temperature relaxation
n_gibbs1 = 100
#number of gibbs sampling steps in  the second temperature relaxation
n_gibbs2 = 100
#number of gibbs sampling steps in  the production temperature relaxation
n_gibbs3 = 10000
#number of md steps
n_md = 1000
#number of mc steps (not < 50 because adaptive)
n_mc = 100
#where to run sims
hostlist = ['localhost']*nreps
#qsub or ssh
grid_method = 'ssh'
qsub_config="-inherit $HOSTNAME" #"-now no -cwd -q -j y -N 'slave' -S /bin/bash"
#replica exchange scheme
rex_scheme='standard'
#replica exchange exchange method
rex_xchg='random'
#whether to use TuneRex to tune temperatures
tune_temps = True
tune_data = {'rate' : 100, #temp optimization rate, in rex steps.
        'method':'ar', #either "ar" acceptance ratio optimization, or "flux" optimization
        'alpha':0.05} # type I error on the estimates
templog = os.path.join(outfolder, 'temps.txt')
rexlog = os.path.join(outfolder,'replicanums.txt')

#misc

imppy = os.path.abspath(
        os.path.join(os.getenv('IMP_ISD_DATA'),'../../tools/imppy.sh'))
src_path = os.path.abspath(
        os.path.join(os.getenv('IMP_ISD_DATA'),'../lib/IMP/isd'))
showX11 = False
grid_debug = False
grid_verbose = False
X11_delay = 1.0
window_size = '80x25'
#pyroGrid
shared_temp_path = True
terminate_during_publish = False
nshost = None

def mkdir_p(path):
    "mkdir -p, taken from stackoverflow"
    try:
        os.makedirs(path)
    except OSError, exc:
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
            grid_verbose, shared_temp_path, nshost, terminate_during_publish,
            method=grid_method, qsub_config=qsub_config)

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
    requests = grid.broadcast(sfo_id, 'init_model', tmpdir, sequence, initpdb, restraints)
    #wait til init is done
    results = grid.gather(requests)

    #turn off verbose noise (works because IMP.base.NONE is picklable, being an int.
    grid.gather(grid.broadcast(sfo_id, 'set_checklevel', IMP.base.NONE))
    grid.gather(grid.broadcast(sfo_id, 'set_loglevel', IMP.base.NONE))

    # evaluate the score of the whole system (without derivatives, False flag)
    print "initial energy"
    grid.gather(grid.broadcast(sfo_id, 'm', 'evaluate', False))

    #berendsen 300K tau=0.5ps
    #perform two independent MC moves for sigma and gamma
    print "initializing simulation and statistics"
    grid.gather(grid.scatter(sfo_id, 'init_simulation',
        zip(lambdas, tau)))
    grid.gather(grid.scatter(sfo_id, 'init_stats', zip(nums, stat_rate)))
    replica = ReplicaTracker(nreps, lambdas, grid, sfo_id,
            rexlog=rexlog,
            scheme=rex_scheme, xchg=rex_xchg,
            tune_temps=tune_temps, tune_data=tune_data, templog=templog)

    print "thermalization"
    for i in range(n_therm):
        print "\rgibbs step %d" % i,
        sys.stdout.flush()
        grid.gather(grid.scatter(sfo_id, 'set_inv_temp',
            [ n_therm/float(i+1) * l for l in lambdas ]))
        grid.gather(grid.broadcast(sfo_id, 'do_md', n_hmc_therm))
        grid.gather(grid.broadcast(sfo_id, 'write_stats'))

    print "start gibbs sampling loop: first relaxation"
    for i in range(n_gibbs1):
        print "\rgibbs step %d" % i,
        sys.stdout.flush()
        #print " md"
        grid.gather(grid.broadcast(sfo_id, 'do_md', n_md))
        #print " mc"
        grid.gather(grid.broadcast(sfo_id, 'do_mc', n_mc))
        grid.gather(grid.broadcast(sfo_id, 'write_stats'))
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
        grid.gather(grid.broadcast(sfo_id, 'do_mc', n_mc))
        grid.gather(grid.broadcast(sfo_id, 'write_stats'))
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
        grid.gather(grid.broadcast(sfo_id, 'do_mc', n_mc))
        grid.gather(grid.broadcast(sfo_id, 'write_stats'))
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
