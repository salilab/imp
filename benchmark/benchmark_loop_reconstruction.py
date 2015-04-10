import IMP
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.benchmark
import time
import sys
import os

import IMP.pmi.restraints.stereochemistry as stereochemistry
import IMP.pmi.restraints.crosslinking as crosslinking
import IMP.pmi.representation as representation
import IMP.pmi.tools as tools
import IMP.pmi.samplers as samplers
import IMP.pmi.output as output

IMP.base.set_log_level(IMP.base.SILENT)
# this benchmark should take 5 sec to initialize and 3 sec per MC loop, for a total of 35 sec.

# Redirect chatty PMI output so we can see benchmark output
old_stdout = sys.stdout

class DummyFile(object):
    def write(self, txt):
        pass
sys.stdout = DummyFile()

# input parameter

pdbfile = IMP.pmi.get_data_path("benchmark_starting_structure.pdb")
fastafile = IMP.pmi.get_data_path("benchmark_sequence.fasta")
fastids = tools.get_ids_from_fasta_file(fastafile)
missing_bead_size = 1


#         Component  pdbfile    chainid  rgb color     fastafile     sequence id
# in fastafile
data = [("chainA", pdbfile, "A", 0.00000000, (fastafile, 0)),
        ("chainB", pdbfile, "B", 0.50000000, (fastafile, 0))]


# create the representation
log_objects = []
optimizable_objects = []

sw = tools.Stopwatch()
log_objects.append(sw)

m = IMP.Model()
r = representation.Representation(m)

hierarchies = {}

for d in data:
    component_name = d[0]
    pdb_file = d[1]
    chain_id = d[2]
    color_id = d[3]
    fasta_file = d[4][0]
    fasta_file_id = d[4][1]
    # avoid to add a component with the same name
    r.create_component(component_name,
                       color=color_id)

    r.add_component_sequence(component_name,
                             fasta_file,
                             id=fastids[fasta_file_id])

    hierarchies = r.autobuild_model(component_name,
                                    pdb_file,
                                    chain_id,
                                    resolutions=[1, 10],
                                    missingbeadsize=missing_bead_size)

    r.show_component_table(component_name)

rbAB = r.set_rigid_bodies(["chainA", "chainB"])

r.set_floppy_bodies()
r.fix_rigid_bodies([rbAB])
r.setup_bonds()


log_objects.append(r)

listofexcludedpairs = []

lof = [(1, 12, "chainA"), (1, 12, "chainB"),
       (294, 339, "chainA"), (294, 339, "chainB"),
       (686, 701, "chainA"), (686, 701, "chainB"),
       (454, 464, "chainA"), (454, 464, "chainB"),
       (472, 486, "chainA"), (472, 486, "chainB"),
       (814, 859, "chainA"), (814, 859, "chainB")]


# add bonds and angles
for l in lof:

    rbr = IMP.pmi.restraints.stereochemistry.ResidueBondRestraint(r, l)
    rbr.add_to_model()
    listofexcludedpairs += rbr.get_excluded_pairs()
    log_objects.append(rbr)

    rar = IMP.pmi.restraints.stereochemistry.ResidueAngleRestraint(r, l)
    rar.add_to_model()
    listofexcludedpairs += rar.get_excluded_pairs()
    log_objects.append(rar)

# add excluded volume

ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
    r,
    resolution=10.0)
ev.add_excluded_particle_pairs(listofexcludedpairs)
ev.add_to_model()
log_objects.append(ev)

mc = samplers.MonteCarlo(m, [r], 1.0)
log_objects.append(mc)


start_time = time.clock()
# In debug mode things are way too slow to actually run MC
if IMP.base.get_check_level() < IMP.base.USAGE_AND_INTERNAL:
    o = output.Output()
    rmf = o.init_rmf("conformations.rmf3", [r.prot])
    o.init_stat2("modeling.stat", log_objects)
    o.write_rmf("conformations.rmf3")
    o.init_pdb("conformations.pdb", r.prot)

    for i in range(0, 10):
#       print("Running job, frame number ", i)

        mc.optimize(10)
        o.write_rmf("conformations.rmf3")
        o.write_pdbs()
        o.write_stats2()
    o.close_rmf("conformations.rmf3")

sys.stdout = old_stdout
IMP.benchmark.report("pmi loop", time.clock() - start_time, 3*10+5)

if IMP.base.get_check_level() < IMP.base.USAGE_AND_INTERNAL:
    for output in ["conformations.pdb", "conformations.rmf3", "modeling.stat"]:
        os.unlink(output)
