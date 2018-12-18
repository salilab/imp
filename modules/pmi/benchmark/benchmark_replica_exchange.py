import IMP
import IMP.benchmark
import IMP.pmi.samplers
import IMP.pmi.representation
import IMP.pmi.restraints.basic
import IMP.pmi.macros
import IMP.pmi.output
import time
import sys
import shutil

IMP.setup_from_argv(sys.argv, "Replica exchange benchmark.")
IMP.set_log_level(IMP.SILENT)
old_stdout = sys.stdout
class DummyFile(object):
    def write(self, txt):
        pass
sys.stdout = DummyFile()

"""setting up the representation
PMI 1.0 representation. Creates two particles and
an harmonic distance restraints between them"""
m=IMP.Model()
with IMP.allow_deprecated():
    r=IMP.pmi.representation.Representation(m)
r.create_component("A")
r.add_component_beads("A",[(1,1),(2,2)])
ps=IMP.atom.get_leaves(r.prot)
dr=IMP.pmi.restraints.basic.DistanceRestraint(r,(1,1,"A"),(2,2,"A"),10,10)
dr.add_to_model()

start_time = time.clock()

rex=IMP.pmi.macros.ReplicaExchange0(m,
                r,
                monte_carlo_sample_objects=[r],
                output_objects=[r,dr],
                monte_carlo_temperature=1.0,
                replica_exchange_minimum_temperature=1.0,
                replica_exchange_maximum_temperature=2.5,
                number_of_best_scoring_models=10,
                monte_carlo_steps=10,
                number_of_frames=10000,
                write_initial_rmf=True,
                initial_rmf_name_suffix="initial",
                stat_file_name_suffix="stat",
                best_pdb_name_suffix="model",
                do_clean_first=True,
                do_create_directories=True,
                global_output_directory="./benchmark_replica_exchange_macro_output",
                rmf_dir="rmfs/",
                best_pdb_dir="pdbs/",
                replica_stat_file_suffix="stat_replica",
                em_object_for_rmf=None,
                replica_exchange_object=None)

# check whether the directory is existing, in case remove it
try:
    shutil.rmtree('./benchmark_replica_exchange_macro_output')
except OSError:
    pass

rex.execute_macro()

# check that each replica index is below the total number of replicas
my_index= rex.replica_exchange_object.get_my_index()
nreplicas=rex.replica_exchange_object.get_number_of_replicas()
temperatures=rex.replica_exchange_object.get_my_parameter("temp")

sys.stdout = old_stdout
IMP.benchmark.report("replica "+str(my_index), time.clock() - start_time, 8.5)
