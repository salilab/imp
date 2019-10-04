import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.benchmark
import time
import sys
import os
try:
    from time import process_time  # needs python 3.3 or later
except ImportError:
    from time import clock as process_time

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.tools as tools
import IMP.pmi.samplers as samplers
import IMP.pmi.output as output

IMP.setup_from_argv(sys.argv, "Loop reconstruction benchmark.")
IMP.set_log_level(IMP.SILENT)
# this benchmark should take 5 sec to initialize and 3 sec per MC loop, for a total of 35 sec.

# Redirect chatty PMI output so we can see benchmark output
old_stdout = sys.stdout

class DummyFile(object):
    def flush(self):
        pass
    def write(self, txt):
        pass
sys.stdout = DummyFile()

# input parameter

pdbfile = IMP.pmi.get_data_path("benchmark_starting_structure.pdb")
fastafile = IMP.pmi.get_data_path("benchmark_sequence.fasta")
sequences = IMP.pmi.topology.Sequences(fastafile)

# create the representation
log_objects = []
optimizable_objects = []

sw = tools.Stopwatch()
log_objects.append(sw)

m = IMP.Model()
s = IMP.pmi.topology.System(m)
st = s.create_state()

cA = st.create_molecule("chainA", sequence=sequences[0])
atomic = cA.add_structure(pdbfile, chain_id='A')
cA.add_representation(atomic, resolutions=[1, 10], color=0.)
cA.add_representation(cA.get_non_atomic_residues(),
                      resolutions=[1], color=0.)

cB = st.create_molecule("chainB", sequence=sequences[0])
atomic = cB.add_structure(pdbfile, chain_id='B')
cB.add_representation(atomic, resolutions=[1, 10], color=0.5)
cB.add_representation(cB.get_non_atomic_residues(),
                      resolutions=[1], color=0.)
root_hier = s.build()

dof = IMP.pmi.dof.DegreesOfFreedom(m)
dof.create_rigid_body(cA)
dof.create_rigid_body(cB)

cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(root_hier)
cr.add_to_model()
log_objects.append(cr)

listofexcludedpairs = []

lof = [cA[:12], cB[:12],
       cA[293:339], cB[293:339],
       cA[685:701], cB[685:701],
       cA[453:464], cB[453:464],
       cA[471:486], cB[471:486],
       cA[813:859], cB[813:859]]

# add bonds and angles
for l in lof:

    rbr = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(objects=l)
    rbr.add_to_model()
    listofexcludedpairs += rbr.get_excluded_pairs()
    log_objects.append(rbr)

    rar = IMP.pmi.restraints.stereochemistry.ResidueAngleRestraint(objects=l)
    rar.add_to_model()
    listofexcludedpairs += rar.get_excluded_pairs()
    log_objects.append(rar)

# add excluded volume

ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
    included_objects=root_hier, resolution=10.0)
ev.add_excluded_particle_pairs(listofexcludedpairs)
ev.add_to_model()
log_objects.append(ev)

mc = samplers.MonteCarlo(m, dof.get_movers(), 1.0)
log_objects.append(mc)


start_time = process_time()
# In debug mode things are way too slow to actually run MC
if IMP.get_check_level() < IMP.USAGE_AND_INTERNAL:
    o = output.Output()
    rmf = o.init_rmf("conformations.rmf3", [root_hier])
    o.init_stat2("modeling.stat", log_objects)
    o.write_rmf("conformations.rmf3")
    o.init_pdb("conformations.pdb", root_hier)

    for i in range(0, 10):
#       print("Running job, frame number ", i)

        mc.optimize(10)
        o.write_rmf("conformations.rmf3")
        o.write_pdbs()
        o.write_stats2()
    o.close_rmf("conformations.rmf3")

sys.stdout = old_stdout
IMP.benchmark.report("pmi loop", process_time() - start_time, 3*10+5)

if IMP.get_check_level() < IMP.USAGE_AND_INTERNAL:
    for output in ["conformations.pdb", "conformations.rmf3", "modeling.stat"]:
        os.unlink(output)
