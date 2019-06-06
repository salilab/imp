"""@namespace IMP.pmi.macros
Protocols for sampling structures and analyzing them.
"""

from __future__ import print_function, division
import IMP
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.samplers
import IMP.pmi.output
import IMP.pmi.analysis
import IMP.pmi.io
import IMP.rmf
import IMP.isd
import IMP.pmi.dof
import RMF
import os
import glob
from operator import itemgetter
from collections import defaultdict
import numpy as np
import string
import itertools
import warnings
import math
import warnings

class _RMFRestraints(object):
    """All restraints that are written out to the RMF file"""
    def __init__(self, model, user_restraints):
        self._rmf_rs = IMP.pmi.tools.get_restraint_set(model, rmf=True)
        self._user_restraints = user_restraints if user_restraints else []

    def __len__(self):
        return (len(self._user_restraints)
                + self._rmf_rs.get_number_of_restraints())

    def __bool__(self):
        return len(self) > 0
    __nonzero__ = __bool__ # Python 2 compatibility

    def __getitem__(self, i):
        class FakePMIWrapper(object):
            def __init__(self, r):
                self.r = IMP.RestraintSet.get_from(r)
            get_restraint = lambda self: self.r
        lenuser = len(self._user_restraints)
        if 0 <= i < lenuser:
            return self._user_restraints[i]
        elif 0 <= i - lenuser < self._rmf_rs.get_number_of_restraints():
            r = self._rmf_rs.get_restraint(i - lenuser)
            return FakePMIWrapper(r)
        else:
            raise IndexError("Out of range")


class ReplicaExchange0(object):
    """A macro to help setup and run replica exchange.
    Supports Monte Carlo and molecular dynamics.
    Produces trajectory RMF files, best PDB structures,
    and output stat files.
    """
    def __init__(self, model,
                 representation=None,
                 root_hier=None,
                 sample_objects=None, # DEPRECATED
                 monte_carlo_sample_objects=None,
                 molecular_dynamics_sample_objects=None,
                 output_objects=[],
                 rmf_output_objects=None,
                 crosslink_restraints=None,
                 monte_carlo_temperature=1.0,
                 simulated_annealing=False,
                 simulated_annealing_minimum_temperature=1.0,
                 simulated_annealing_maximum_temperature=2.5,
                 simulated_annealing_minimum_temperature_nframes=100,
                 simulated_annealing_maximum_temperature_nframes=100,
                 replica_exchange_minimum_temperature=1.0,
                 replica_exchange_maximum_temperature=2.5,
                 replica_exchange_swap=True,
                 num_sample_rounds=1,
                 number_of_best_scoring_models=500,
                 monte_carlo_steps=10,
                 self_adaptive=False,
                 molecular_dynamics_steps=10,
                 molecular_dynamics_max_time_step=1.0,
                 number_of_frames=1000,
                 save_coordinates_mode="lowest_temperature",
                 nframes_write_coordinates=1,
                 write_initial_rmf=True,
                 initial_rmf_name_suffix="initial",
                 stat_file_name_suffix="stat",
                 best_pdb_name_suffix="model",
                 do_clean_first=True,
                 do_create_directories=True,
                 global_output_directory="./",
                 rmf_dir="rmfs/",
                 best_pdb_dir="pdbs/",
                 replica_stat_file_suffix="stat_replica",
                 em_object_for_rmf=None,
                 atomistic=False,
                 replica_exchange_object=None,
                 test_mode=False):
        """Constructor.
           @param model                    The IMP model
           @param representation PMI.representation.Representation object
                  (or list of them, for multi-state modeling)
           @param root_hier Instead of passing Representation, pass the System hierarchy
           @param monte_carlo_sample_objects Objects for MC sampling; a list of
                  structural components (generally the representation) that will
                  be moved and restraints with parameters that need to
                  be sampled.
                  For PMI2: just pass flat list of movers
           @param molecular_dynamics_sample_objects Objects for MD sampling
                  For PMI2: just pass flat list of particles
           @param output_objects A list of structural objects and restraints
                  that will be included in output (ie, statistics "stat" files). Any object
                  that provides a get_output() method can be used here. If None is passed
                  the macro will not write stat files.
           @param rmf_output_objects A list of structural objects and restraints
                  that will be included in rmf. Any object
                  that provides a get_output() method can be used here.
           @param crosslink_restraints List of cross-link restraints that will
                  be included in output RMF files (for visualization).
           @param monte_carlo_temperature  MC temp (may need to be optimized
                  based on post-sampling analysis)
           @param simulated_annealing If True, perform simulated annealing
           @param simulated_annealing_minimum_temperature Should generally be
                  the same as monte_carlo_temperature.
           @param simulated_annealing_minimum_temperature_nframes Number of
                  frames to compute at minimum temperature.
           @param simulated_annealing_maximum_temperature_nframes Number of
                  frames to compute at
                  temps > simulated_annealing_maximum_temperature.
           @param replica_exchange_minimum_temperature Low temp for REX; should
                  generally be the same as monte_carlo_temperature.
           @param replica_exchange_maximum_temperature High temp for REX
           @param replica_exchange_swap Boolean, enable disable temperature
                  swap (Default=True)
           @param num_sample_rounds        Number of rounds of MC/MD per cycle
           @param number_of_best_scoring_models Number of top-scoring PDB models
                  to keep around for analysis
           @param monte_carlo_steps        Number of MC steps per round
           @param self_adaptive        self adaptive scheme for monte carlo movers
           @param molecular_dynamics_steps  Number of MD steps per round
           @param molecular_dynamics_max_time_step Max time step for MD
           @param number_of_frames         Number of REX frames to run
           @param save_coordinates_mode  string: how to save coordinates.
                  "lowest_temperature" (default) only the lowest temperatures is saved
                  "25th_score" all replicas whose score is below the 25th percentile
                  "50th_score" all replicas whose score is below the 50th percentile
                  "75th_score" all replicas whose score is below the 75th percentile
           @param nframes_write_coordinates How often to write the coordinates
                  of a frame
           @param write_initial_rmf        Write the initial configuration
           @param global_output_directory Folder that will be created to house
                  output.
        @param test_mode Set to True to avoid writing any files, just test one frame.
        """
        self.model = model
        self.vars = {}
        self.pmi2 = False

        ### add check hierarchy is multistate
        self.output_objects = output_objects
        self.rmf_output_objects=rmf_output_objects
        self.representation = representation
        if representation:
            if type(representation) == list:
                self.is_multi_state = True
                self.root_hiers = [r.prot for r in representation]
                self.vars["number_of_states"] = len(representation)
            else:
                self.is_multi_state = False
                self.root_hier = representation.prot
                self.vars["number_of_states"] = 1
        elif root_hier and type(root_hier) == IMP.atom.Hierarchy and root_hier.get_name()=='System':
            self.pmi2 = True
            if self.output_objects is not None:
                self.output_objects.append(IMP.pmi.io.TotalScoreOutput(self.model))
            if self.rmf_output_objects is not None:
                self.rmf_output_objects.append(IMP.pmi.io.TotalScoreOutput(self.model))
            self.root_hier = root_hier
            states = IMP.atom.get_by_type(root_hier,IMP.atom.STATE_TYPE)
            self.vars["number_of_states"] = len(states)
            if len(states)>1:
                self.root_hiers = states
                self.is_multi_state = True
            else:
                self.root_hier = root_hier
                self.is_multi_state = False
        else:
            raise Exception("Must provide representation or System hierarchy (root_hier)")

        self._rmf_restraints = _RMFRestraints(model, crosslink_restraints)
        self.em_object_for_rmf = em_object_for_rmf
        self.monte_carlo_sample_objects = monte_carlo_sample_objects
        self.vars["self_adaptive"]=self_adaptive
        if sample_objects is not None:
            self.monte_carlo_sample_objects+=sample_objects
        self.molecular_dynamics_sample_objects=molecular_dynamics_sample_objects
        self.replica_exchange_object = replica_exchange_object
        self.molecular_dynamics_max_time_step = molecular_dynamics_max_time_step
        self.vars["monte_carlo_temperature"] = monte_carlo_temperature
        self.vars[
            "replica_exchange_minimum_temperature"] = replica_exchange_minimum_temperature
        self.vars[
            "replica_exchange_maximum_temperature"] = replica_exchange_maximum_temperature
        self.vars["replica_exchange_swap"] = replica_exchange_swap
        self.vars["simulated_annealing"]=\
                                   simulated_annealing
        self.vars["simulated_annealing_minimum_temperature"]=\
                                   simulated_annealing_minimum_temperature
        self.vars["simulated_annealing_maximum_temperature"]=\
                                   simulated_annealing_maximum_temperature
        self.vars["simulated_annealing_minimum_temperature_nframes"]=\
                                   simulated_annealing_minimum_temperature_nframes
        self.vars["simulated_annealing_maximum_temperature_nframes"]=\
                                   simulated_annealing_maximum_temperature_nframes

        self.vars["num_sample_rounds"] = num_sample_rounds
        self.vars[
            "number_of_best_scoring_models"] = number_of_best_scoring_models
        self.vars["monte_carlo_steps"] = monte_carlo_steps
        self.vars["molecular_dynamics_steps"]=molecular_dynamics_steps
        self.vars["number_of_frames"] = number_of_frames
        if not save_coordinates_mode  in ["lowest_temperature","25th_score","50th_score","75th_score"]:
            raise Exception("save_coordinates_mode has unrecognized value")
        else:
            self.vars["save_coordinates_mode"] = save_coordinates_mode
        self.vars["nframes_write_coordinates"] = nframes_write_coordinates
        self.vars["write_initial_rmf"] = write_initial_rmf
        self.vars["initial_rmf_name_suffix"] = initial_rmf_name_suffix
        self.vars["best_pdb_name_suffix"] = best_pdb_name_suffix
        self.vars["stat_file_name_suffix"] = stat_file_name_suffix
        self.vars["do_clean_first"] = do_clean_first
        self.vars["do_create_directories"] = do_create_directories
        self.vars["global_output_directory"] = global_output_directory
        self.vars["rmf_dir"] = rmf_dir
        self.vars["best_pdb_dir"] = best_pdb_dir
        self.vars["atomistic"] = atomistic
        self.vars["replica_stat_file_suffix"] = replica_stat_file_suffix
        self.vars["geometries"] = None
        self.test_mode = test_mode

    def add_geometries(self, geometries):
        if self.vars["geometries"] is None:
            self.vars["geometries"] = list(geometries)
        else:
            self.vars["geometries"].extend(geometries)

    def show_info(self):
        print("ReplicaExchange0: it generates initial.*.rmf3, stat.*.out, rmfs/*.rmf3 for each replica ")
        print("--- it stores the best scoring pdb models in pdbs/")
        print("--- the stat.*.out and rmfs/*.rmf3 are saved only at the lowest temperature")
        print("--- variables:")
        keys = list(self.vars.keys())
        keys.sort()
        for v in keys:
            print("------", v.ljust(30), self.vars[v])

    def get_replica_exchange_object(self):
        return self.replica_exchange_object

    def _add_provenance(self, sampler_md, sampler_mc):
        """Record details about the sampling in the IMP Hierarchies"""
        if not self.is_multi_state or self.pmi2:
            output_hierarchies = [self.root_hier]
        else:
            output_hierarchies = self.root_hiers

        iterations = 0
        if sampler_md:
            method = "Molecular Dynamics"
            iterations += self.vars["molecular_dynamics_steps"]
        if sampler_mc:
            method = "Hybrid MD/MC" if sampler_md else "Monte Carlo"
            iterations += self.vars["monte_carlo_steps"]
        # If no sampling is actually done, no provenance to write
        if iterations == 0 or self.vars["number_of_frames"] == 0:
            return
        iterations *= self.vars["num_sample_rounds"]

        for h in output_hierarchies:
            pi = self.model.add_particle("sampling")
            p = IMP.core.SampleProvenance.setup_particle(
                    self.model, pi, method, self.vars["number_of_frames"],
                    iterations)
            p.set_number_of_replicas(
                    self.replica_exchange_object.get_number_of_replicas())
            IMP.pmi.tools._add_pmi_provenance(h)
            IMP.core.add_provenance(self.model, h, p)

    def execute_macro(self):
        temp_index_factor = 100000.0
        samplers=[]
        sampler_mc=None
        sampler_md=None
        if self.monte_carlo_sample_objects is not None:
            print("Setting up MonteCarlo")
            sampler_mc = IMP.pmi.samplers.MonteCarlo(self.model,
                                                     self.monte_carlo_sample_objects,
                                                     self.vars["monte_carlo_temperature"])
            if self.vars["simulated_annealing"]:
                tmin=self.vars["simulated_annealing_minimum_temperature"]
                tmax=self.vars["simulated_annealing_maximum_temperature"]
                nfmin=self.vars["simulated_annealing_minimum_temperature_nframes"]
                nfmax=self.vars["simulated_annealing_maximum_temperature_nframes"]
                sampler_mc.set_simulated_annealing(tmin,tmax,nfmin,nfmax)
            if self.vars["self_adaptive"]:
                sampler_mc.set_self_adaptive(isselfadaptive=self.vars["self_adaptive"])
            if self.output_objects is not None:
                self.output_objects.append(sampler_mc)
            if self.rmf_output_objects is not None:
                self.rmf_output_objects.append(sampler_mc)
            samplers.append(sampler_mc)


        if self.molecular_dynamics_sample_objects is not None:
            print("Setting up MolecularDynamics")
            sampler_md = IMP.pmi.samplers.MolecularDynamics(self.model,
                                                            self.molecular_dynamics_sample_objects,
                                                            self.vars["monte_carlo_temperature"],
                                                            maximum_time_step=self.molecular_dynamics_max_time_step)
            if self.vars["simulated_annealing"]:
                tmin=self.vars["simulated_annealing_minimum_temperature"]
                tmax=self.vars["simulated_annealing_maximum_temperature"]
                nfmin=self.vars["simulated_annealing_minimum_temperature_nframes"]
                nfmax=self.vars["simulated_annealing_maximum_temperature_nframes"]
                sampler_md.set_simulated_annealing(tmin,tmax,nfmin,nfmax)
            if self.output_objects is not None:
                self.output_objects.append(sampler_md)
            if self.rmf_output_objects is not None:
                self.rmf_output_objects.append(sampler_md)
            samplers.append(sampler_md)
# -------------------------------------------------------------------------

        print("Setting up ReplicaExchange")
        rex = IMP.pmi.samplers.ReplicaExchange(self.model,
                                               self.vars[
                                                   "replica_exchange_minimum_temperature"],
                                               self.vars[
                                                   "replica_exchange_maximum_temperature"],
                                               samplers,
                                               replica_exchange_object=self.replica_exchange_object)
        self.replica_exchange_object = rex.rem

        myindex = rex.get_my_index()
        if self.output_objects is not None:
            self.output_objects.append(rex)
        if self.rmf_output_objects is not None:
            self.rmf_output_objects.append(rex)
        # must reset the minimum temperature due to the
        # different binary length of rem.get_my_parameter double and python
        # float
        min_temp_index = int(min(rex.get_temperatures()) * temp_index_factor)

# -------------------------------------------------------------------------

        globaldir = self.vars["global_output_directory"] + "/"
        rmf_dir = globaldir + self.vars["rmf_dir"]
        pdb_dir = globaldir + self.vars["best_pdb_dir"]

        if not self.test_mode:
            if self.vars["do_clean_first"]:
                pass

            if self.vars["do_create_directories"]:

                try:
                    os.makedirs(globaldir)
                except:
                    pass
                try:
                    os.makedirs(rmf_dir)
                except:
                    pass

                if not self.is_multi_state:
                    try:
                        os.makedirs(pdb_dir)
                    except:
                        pass
                else:
                    for n in range(self.vars["number_of_states"]):
                        try:
                            os.makedirs(pdb_dir + "/" + str(n))
                        except:
                            pass

# -------------------------------------------------------------------------

        sw = IMP.pmi.tools.Stopwatch()
        if self.output_objects is not None:
            self.output_objects.append(sw)
        if self.rmf_output_objects is not None:
            self.rmf_output_objects.append(sw)

        print("Setting up stat file")
        output = IMP.pmi.output.Output(atomistic=self.vars["atomistic"])
        low_temp_stat_file = globaldir + \
            self.vars["stat_file_name_suffix"] + "." + str(myindex) + ".out"

        # Ensure model is updated before saving init files
        if not self.test_mode:
            self.model.update()

        if not self.test_mode:
            if self.output_objects is not None:
                output.init_stat2(low_temp_stat_file,
                              self.output_objects,
                              extralabels=["rmf_file", "rmf_frame_index"])
        else:
            print("Stat file writing is disabled")

        if self.rmf_output_objects is not None:
            print("Stat info being written in the rmf file")

        print("Setting up replica stat file")
        replica_stat_file = globaldir + \
            self.vars["replica_stat_file_suffix"] + "." + str(myindex) + ".out"
        if not self.test_mode:
            output.init_stat2(replica_stat_file, [rex], extralabels=["score"])

        if not self.test_mode:
            print("Setting up best pdb files")
            if not self.is_multi_state:
                if self.vars["number_of_best_scoring_models"] > 0:
                    output.init_pdb_best_scoring(pdb_dir + "/" +
                                                 self.vars["best_pdb_name_suffix"],
                                                 self.root_hier,
                                                 self.vars[
                                                     "number_of_best_scoring_models"],
                                                 replica_exchange=True)
                    output.write_psf(pdb_dir + "/" +"model.psf",pdb_dir + "/" +
                                                 self.vars["best_pdb_name_suffix"]+".0.pdb")
            else:
                if self.vars["number_of_best_scoring_models"] > 0:
                    for n in range(self.vars["number_of_states"]):
                        output.init_pdb_best_scoring(pdb_dir + "/" + str(n) + "/" +
                                                   self.vars["best_pdb_name_suffix"],
                                                   self.root_hiers[n],
                                                   self.vars[
                                                       "number_of_best_scoring_models"],
                                                   replica_exchange=True)
                        output.write_psf(pdb_dir + "/" + str(n) + "/" +"model.psf",pdb_dir + "/" + str(n) + "/" +
                                                 self.vars["best_pdb_name_suffix"]+".0.pdb")
# ---------------------------------------------

        if self.em_object_for_rmf is not None:
            if not self.is_multi_state or self.pmi2:
                output_hierarchies = [
                    self.root_hier,
                    self.em_object_for_rmf.get_density_as_hierarchy(
                    )]
            else:
                output_hierarchies = self.root_hiers
                output_hierarchies.append(
                    self.em_object_for_rmf.get_density_as_hierarchy())
        else:
            if not self.is_multi_state or self.pmi2:
                output_hierarchies = [self.root_hier]
            else:
                output_hierarchies = self.root_hiers

#----------------------------------------------
        if not self.test_mode:
            print("Setting up and writing initial rmf coordinate file")
            init_suffix = globaldir + self.vars["initial_rmf_name_suffix"]
            output.init_rmf(init_suffix + "." + str(myindex) + ".rmf3",
                            output_hierarchies,listofobjects=self.rmf_output_objects)
            if self._rmf_restraints:
                output.add_restraints_to_rmf(
                    init_suffix + "." + str(myindex) + ".rmf3",
                    self._rmf_restraints)
            output.write_rmf(init_suffix + "." + str(myindex) + ".rmf3")
            output.close_rmf(init_suffix + "." + str(myindex) + ".rmf3")

#----------------------------------------------

        if not self.test_mode:
            mpivs=IMP.pmi.samplers.MPI_values(self.replica_exchange_object)

#----------------------------------------------

        self._add_provenance(sampler_md, sampler_mc)

        if not self.test_mode:
            print("Setting up production rmf files")
            rmfname = rmf_dir + "/" + str(myindex) + ".rmf3"
            output.init_rmf(rmfname, output_hierarchies, geometries=self.vars["geometries"],
                            listofobjects = self.rmf_output_objects)

            if self._rmf_restraints:
                output.add_restraints_to_rmf(rmfname, self._rmf_restraints)

        ntimes_at_low_temp = 0

        if myindex == 0:
            self.show_info()
        self.replica_exchange_object.set_was_used(True)
        nframes = self.vars["number_of_frames"]
        if self.test_mode:
            nframes = 1
        for i in range(nframes):
            if self.test_mode:
                score = 0.
            else:
                for nr in range(self.vars["num_sample_rounds"]):
                    if sampler_md is not None:
                        sampler_md.optimize(
                                  self.vars["molecular_dynamics_steps"])
                    if sampler_mc is not None:
                        sampler_mc.optimize(self.vars["monte_carlo_steps"])
                score = IMP.pmi.tools.get_restraint_set(
                                             self.model).evaluate(False)
                mpivs.set_value("score",score)
            output.set_output_entry("score", score)



            my_temp_index = int(rex.get_my_temp() * temp_index_factor)

            if self.vars["save_coordinates_mode"] == "lowest_temperature":
                save_frame=(min_temp_index == my_temp_index)
            elif self.vars["save_coordinates_mode"] == "25th_score":
                score_perc=mpivs.get_percentile("score")
                save_frame=(score_perc*100.0<=25.0)
            elif self.vars["save_coordinates_mode"] == "50th_score":
                score_perc=mpivs.get_percentile("score")
                save_frame=(score_perc*100.0<=50.0)
            elif self.vars["save_coordinates_mode"] == "75th_score":
                score_perc=mpivs.get_percentile("score")
                save_frame=(score_perc*100.0<=75.0)

            # Ensure model is updated before saving output files
            if save_frame or not self.test_mode:
                self.model.update()

            if save_frame:
                print("--- frame %s score %s " % (str(i), str(score)))

                if not self.test_mode:
                    if i % self.vars["nframes_write_coordinates"]==0:
                        print('--- writing coordinates')
                        if self.vars["number_of_best_scoring_models"] > 0:
                            output.write_pdb_best_scoring(score)
                        output.write_rmf(rmfname)
                        output.set_output_entry("rmf_file", rmfname)
                        output.set_output_entry("rmf_frame_index", ntimes_at_low_temp)
                    else:
                        output.set_output_entry("rmf_file", rmfname)
                        output.set_output_entry("rmf_frame_index", '-1')
                    if self.output_objects is not None:
                        output.write_stat2(low_temp_stat_file)
                ntimes_at_low_temp += 1

            if not self.test_mode:
                output.write_stat2(replica_stat_file)
            if self.vars["replica_exchange_swap"]:
                rex.swap_temp(i, score)
        for p, state in IMP.pmi.tools._all_protocol_outputs(
                            [self.representation],
                            self.root_hier if self.pmi2 else None):
            p.add_replica_exchange(state, self)

        if not self.test_mode:
            print("closing production rmf files")
            output.close_rmf(rmfname)



# ----------------------------------------------------------------------
class BuildSystem(object):
    """A macro to build a IMP::pmi::topology::System based on a TopologyReader object.
    Easily create multi-state systems by calling this macro
    repeatedly with different TopologyReader objects!
    A useful function is get_molecules() which returns the PMI Molecules grouped by state
    as a dictionary with key = (molecule name), value = IMP.pmi.topology.Molecule
    Quick multi-state system:
    @code{.python}
    model = IMP.Model()
    reader1 = IMP.pmi.topology.TopologyReader(tfile1)
    reader2 = IMP.pmi.topology.TopologyReader(tfile2)
    bs = IMP.pmi.macros.BuildSystem(model)
    bs.add_state(reader1)
    bs.add_state(reader2)
    bs.execute_macro() # build everything including degrees of freedom
    IMP.atom.show_molecular_hierarchy(bs.get_hierarchy())
    ### now you have a two state system, you add restraints etc
    @endcode
    @note The "domain name" entry of the topology reader is not used.
    All molecules are set up by the component name, but split into rigid bodies
    as requested.
    """
    def __init__(self,
                 model,
                 sequence_connectivity_scale=4.0,
                 force_create_gmm_files=False,
                 resolutions=[1,10]):
        """Constructor
        @param model An IMP Model
        @param sequence_connectivity_scale For scaling the connectivity restraint
        @param force_create_gmm_files If True, will sample and create GMMs
                  no matter what. If False, will only sample if the
                  files don't exist. If number of Gaussians is zero, won't
                  do anything.
        @param resolutions The resolutions to build for structured regions
        """
        self.model = model
        self.system = IMP.pmi.topology.System(self.model)
        self._readers = []    # the TopologyReaders (one per state)
        self._domain_res = [] # TempResidues for each domain key=unique name, value=(atomic_res,non_atomic_res).
        self._domains = []    # key = domain unique name, value = Component
        self.force_create_gmm_files = force_create_gmm_files
        self.resolutions = resolutions

    def add_state(self,
                  reader,
                  keep_chain_id=False, fasta_name_map=None):
        """Add a state using the topology info in a IMP::pmi::topology::TopologyReader object.
        When you are done adding states, call execute_macro()
        @param reader The TopologyReader object
        @param fasta_name_map dictionary for converting protein names found in the fasta file
        """
        state = self.system.create_state()
        self._readers.append(reader)
        these_domain_res = {}    #  key is unique name, value is (atomic res, nonatomicres)
        these_domains = {}       #  key is unique name, value is _Component
        chain_ids = string.ascii_uppercase+string.ascii_lowercase+'0123456789'
        numchain = 0

        ### setup representation
        # loop over molecules, copies, then domains
        for molname in reader.get_molecules():
            copies = reader.get_molecules()[molname].domains
            for nc,copyname in enumerate(copies):
                print("BuildSystem.add_state: setting up molecule %s copy number %s" % (molname,str(nc)))
                copy = copies[copyname]
                # option to not rename chains
                if keep_chain_id == True:
                    chain_id = copy[0].chain
                else:
                    chain_id = chain_ids[numchain]
                if nc==0:
                    is_nucleic=False
                    fasta_flag=copy[0].fasta_flag
                    if fasta_flag == "RNA" or fasta_flag == "DNA": is_nucleic=True
                    seq = IMP.pmi.topology.Sequences(copy[0].fasta_file, fasta_name_map)[copy[0].fasta_id]
                    print("BuildSystem.add_state: molecule %s sequence has %s residues" % (molname,len(seq)))
                    orig_mol = state.create_molecule(molname,
                                                     seq,
                                                     chain_id,is_nucleic)
                    mol = orig_mol
                    numchain+=1
                else:
                    print("BuildSystem.add_state: creating a copy for molecule %s" % molname)
                    mol = orig_mol.create_copy(chain_id)
                    numchain+=1

                for domainnumber,domain in enumerate(copy):
                    print("BuildSystem.add_state: ---- setting up domain %s of molecule %s" % (domainnumber,molname))
                    # we build everything in the residue range, even if it
                    #  extends beyond what's in the actual PDB file
                    these_domains[domain.get_unique_name()] = domain
                    if domain.residue_range==[] or domain.residue_range is None:
                        domain_res = mol.get_residues()
                    else:
                        start = domain.residue_range[0]+domain.pdb_offset
                        if domain.residue_range[1]=='END':
                            end = len(mol.sequence)
                        else:
                            end = domain.residue_range[1]+domain.pdb_offset
                        domain_res = mol.residue_range(start-1,end-1)
                        print("BuildSystem.add_state: -------- domain %s of molecule %s extends from residue %s to residue %s " % (domainnumber,molname,start,end))
                    if domain.pdb_file=="BEADS":
                        print("BuildSystem.add_state: -------- domain %s of molecule %s represented by BEADS " % (domainnumber,molname))
                        mol.add_representation(
                            domain_res,
                            resolutions=[domain.bead_size],
                            setup_particles_as_densities=(
                                domain.em_residues_per_gaussian!=0),
                            color = domain.color)
                        these_domain_res[domain.get_unique_name()] = (set(),domain_res)
                    elif domain.pdb_file=="IDEAL_HELIX":
                        print("BuildSystem.add_state: -------- domain %s of molecule %s represented by IDEAL_HELIX " % (domainnumber,molname))
                        mol.add_representation(
                            domain_res,
                            resolutions=self.resolutions,
                            ideal_helix=True,
                            density_residues_per_component=domain.em_residues_per_gaussian,
                            density_prefix=domain.density_prefix,
                            density_force_compute=self.force_create_gmm_files,
                            color = domain.color)
                        these_domain_res[domain.get_unique_name()] = (domain_res,set())
                    else:
                        print("BuildSystem.add_state: -------- domain %s of molecule %s represented by pdb file %s " % (domainnumber,molname,domain.pdb_file))
                        domain_atomic = mol.add_structure(domain.pdb_file,
                                                          domain.chain,
                                                          domain.residue_range,
                                                          domain.pdb_offset,
                                                          soft_check=True)
                        domain_non_atomic = domain_res - domain_atomic
                        if not domain.em_residues_per_gaussian:
                            mol.add_representation(domain_atomic,
                                                   resolutions=self.resolutions,
                                                   color = domain.color)
                            if len(domain_non_atomic)>0:
                                mol.add_representation(domain_non_atomic,
                                                       resolutions=[domain.bead_size],
                                                       color = domain.color)
                        else:
                            print("BuildSystem.add_state: -------- domain %s of molecule %s represented by gaussians " % (domainnumber,molname))
                            mol.add_representation(
                                domain_atomic,
                                resolutions=self.resolutions,
                                density_residues_per_component=domain.em_residues_per_gaussian,
                                density_prefix=domain.density_prefix,
                                density_force_compute=self.force_create_gmm_files,
                                color = domain.color)
                            if len(domain_non_atomic)>0:
                                mol.add_representation(domain_non_atomic,
                                                       resolutions=[domain.bead_size],
                                                       setup_particles_as_densities=True,
                                                       color = domain.color)
                        these_domain_res[domain.get_unique_name()] = (
                            domain_atomic,domain_non_atomic)
        self._domain_res.append(these_domain_res)
        self._domains.append(these_domains)
        print('BuildSystem.add_state: State',len(self.system.states),'added')

    def get_molecules(self):
        """Return list of all molecules grouped by state.
        For each state, it's a dictionary of Molecules where key is the molecule name
        """
        return [s.get_molecules() for s in self.system.get_states()]

    def get_molecule(self,molname,copy_index=0,state_index=0):
        return self.system.get_states()[state_index].get_molecules()[molname][copy_index]

    def execute_macro(self, max_rb_trans=4.0, max_rb_rot=0.04, max_bead_trans=4.0, max_srb_trans=4.0,max_srb_rot=0.04):
        """Builds representations and sets up degrees of freedom"""
        print("BuildSystem.execute_macro: building representations")
        self.root_hier = self.system.build()

        print("BuildSystem.execute_macro: setting up degrees of freedom")
        self.dof = IMP.pmi.dof.DegreesOfFreedom(self.model)
        for nstate,reader in enumerate(self._readers):
            rbs = reader.get_rigid_bodies()
            srbs = reader.get_super_rigid_bodies()
            csrbs = reader.get_chains_of_super_rigid_bodies()

            # add rigid bodies
            domains_in_rbs = set()
            for rblist in rbs:
                print("BuildSystem.execute_macro: -------- building rigid body %s" % (str(rblist)))
                all_res = IMP.pmi.tools.OrderedSet()
                bead_res = IMP.pmi.tools.OrderedSet()
                for dname in rblist:
                    domain = self._domains[nstate][dname]
                    print("BuildSystem.execute_macro: -------- adding %s" % (str(dname  )))
                    all_res|=self._domain_res[nstate][dname][0]
                    bead_res|=self._domain_res[nstate][dname][1]
                    domains_in_rbs.add(dname)
                all_res|=bead_res
                print("BuildSystem.execute_macro: -------- \
creating rigid body with max_trans %s \
max_rot %s non_rigid_max_trans %s" \
                          % (str(max_rb_trans),str(max_rb_rot),str(max_bead_trans)))
                self.dof.create_rigid_body(all_res,
                                           nonrigid_parts=bead_res,
                                           max_trans=max_rb_trans,
                                           max_rot=max_rb_rot,
                                           nonrigid_max_trans=max_bead_trans)

            # if you have any domains not in an RB, set them as flexible beads
            for dname, domain in self._domains[nstate].items():
                if dname not in domains_in_rbs:
                    if domain.pdb_file != "BEADS":
                        warnings.warn(
                              "Making %s flexible. This may distort the "
                              "structure; consider making it rigid" % dname,
                              IMP.pmi.StructureWarning)
                    self.dof.create_flexible_beads(
                            self._domain_res[nstate][dname][1],
                            max_trans=max_bead_trans)

            # add super rigid bodies
            for srblist in srbs:
                print("BuildSystem.execute_macro: -------- building super rigid body %s" % (str(srblist)))
                all_res = IMP.pmi.tools.OrderedSet()
                for dname in srblist:
                    print("BuildSystem.execute_macro: -------- adding %s" % (str(dname  )))
                    all_res|=self._domain_res[nstate][dname][0]
                    all_res|=self._domain_res[nstate][dname][1]

                print("BuildSystem.execute_macro: -------- \
creating super rigid body with max_trans %s max_rot %s " \
                % (str(max_srb_trans),str(max_srb_rot)))
                self.dof.create_super_rigid_body(all_res,max_trans=max_srb_trans,max_rot=max_srb_rot)

            # add chains of super rigid bodies
            for csrblist in csrbs:
                all_res = IMP.pmi.tools.OrderedSet()
                for dname in csrblist:
                    all_res|=self._domain_res[nstate][dname][0]
                    all_res|=self._domain_res[nstate][dname][1]
                all_res = list(all_res)
                all_res.sort(key=lambda r:r.get_index())
                #self.dof.create_super_rigid_body(all_res,chain_min_length=2,chain_max_length=3)
                self.dof.create_main_chain_mover(all_res)
        return self.root_hier,self.dof


@IMP.deprecated_object("2.8", "Use AnalysisReplicaExchange instead")
class AnalysisReplicaExchange0(object):
    """A macro for running all the basic operations of analysis.
    Includes clustering, precision analysis, and making ensemble density maps.
    A number of plots are also supported.
    """
    def __init__(self, model,
                 merge_directories=["./"],
                 stat_file_name_suffix="stat",
                 best_pdb_name_suffix="model",
                 do_clean_first=True,
                 do_create_directories=True,
                 global_output_directory="output/",
                 replica_stat_file_suffix="stat_replica",
                 global_analysis_result_directory="./analysis/",
                 test_mode=False):
        """Constructor.
           @param model                           The IMP model
           @param stat_file_name_suffix
           @param merge_directories The directories containing output files
           @param best_pdb_name_suffix
           @param do_clean_first
           @param do_create_directories
           @param global_output_directory          Where everything is
           @param replica_stat_file_suffix
           @param global_analysis_result_directory
           @param test_mode If True, nothing is changed on disk
        """

        try:
            from mpi4py import MPI
            self.comm = MPI.COMM_WORLD
            self.rank = self.comm.Get_rank()
            self.number_of_processes = self.comm.size
        except ImportError:
            self.rank = 0
            self.number_of_processes = 1

        self.test_mode = test_mode
        self._protocol_output = []
        self.cluster_obj = None
        self.model = model
        stat_dir = global_output_directory
        self.stat_files = []
        # it contains the position of the root directories
        for rd in merge_directories:
            stat_files = glob.glob(os.path.join(rd,stat_dir,"stat.*.out"))
            if len(stat_files)==0:
                warnings.warn("no stat files found in %s"
                              % os.path.join(rd, stat_dir),
                              IMP.pmi.MissingFileWarning)
            self.stat_files += stat_files

    def add_protocol_output(self, p):
        """Capture details of the modeling protocol.
           @param p an instance of IMP.pmi.output.ProtocolOutput or a subclass.
        """
        # Assume last state is the one we're interested in
        self._protocol_output.append((p, p._last_state))

    def get_modeling_trajectory(self,
                                score_key="SimplifiedModel_Total_Score_None",
                                rmf_file_key="rmf_file",
                                rmf_file_frame_key="rmf_frame_index",
                                outputdir="./",
                                get_every=1,
                                nframes_trajectory=10000):
        """ Get a trajectory of the modeling run, for generating demonstrative movies
        @param score_key                           The score for ranking models
        @param rmf_file_key                        Key pointing to RMF filename
        @param rmf_file_frame_key                  Key pointing to RMF frame number
        @param outputdir                           The local output directory used in the run
        @param get_every                           Extract every nth frame
        @param nframes_trajectory                  Total number of frames of the trajectory
        """
        from operator import itemgetter
        import math

        trajectory_models = IMP.pmi.io.get_trajectory_models(self.stat_files,
                                                 score_key,
                                                 rmf_file_key,
                                                 rmf_file_frame_key,
                                                 get_every)
        rmf_file_list=trajectory_models[0]
        rmf_file_frame_list=trajectory_models[1]
        score_list=list(map(float, trajectory_models[2]))

        max_score=max(score_list)
        min_score=min(score_list)


        bins=[(max_score-min_score)*math.exp(-float(i))+min_score for i in range(nframes_trajectory)]
        binned_scores=[None]*nframes_trajectory
        binned_model_indexes=[-1]*nframes_trajectory

        for model_index,s in enumerate(score_list):
            bins_score_diffs=[abs(s-b) for b in bins]
            bin_index=min(enumerate(bins_score_diffs), key=itemgetter(1))[0]
            if binned_scores[bin_index]==None:
                binned_scores[bin_index]=s
                binned_model_indexes[bin_index]=model_index
            else:
                old_diff=abs(binned_scores[bin_index]-bins[bin_index])
                new_diff=abs(s-bins[bin_index])
                if new_diff < old_diff:
                    binned_scores[bin_index]=s
                    binned_model_indexes[bin_index]=model_index

        print(binned_scores)
        print(binned_model_indexes)


    def _expand_ambiguity(self,prot,d):
        """If using PMI2, expand the dictionary to include copies as ambiguous options
        This also keeps the states separate.
        """
        newdict = {}
        for key in d:
            val = d[key]
            if '..' in key or (type(val) is tuple and len(val)>=3):
                newdict[key] = val
                continue
            states = IMP.atom.get_by_type(prot,IMP.atom.STATE_TYPE)
            if type(val) is tuple:
                start = val[0]
                stop = val[1]
                name = val[2]
            else:
                start = 1
                stop = -1
                name = val
            for nst in range(len(states)):
                sel = IMP.atom.Selection(prot,molecule=name,state_index=nst)
                copies = sel.get_selected_particles(with_representation=False)
                if len(copies)>1:
                    for nc in range(len(copies)):
                        if len(states)>1:
                            newdict['%s.%i..%i'%(name,nst,nc)] = (start,stop,name,nc,nst)
                        else:
                            newdict['%s..%i'%(name,nc)] = (start,stop,name,nc,nst)
                else:
                    newdict[key] = val
        return newdict


    def clustering(self,
                   score_key="SimplifiedModel_Total_Score_None",
                   rmf_file_key="rmf_file",
                   rmf_file_frame_key="rmf_frame_index",
                   state_number=0,
                   prefiltervalue=None,
                   feature_keys=[],
                   outputdir="./",
                   alignment_components=None,
                   number_of_best_scoring_models=10,
                   rmsd_calculation_components=None,
                   distance_matrix_file='distances.mat',
                   load_distance_matrix_file=False,
                   skip_clustering=False,
                   number_of_clusters=1,
                   display_plot=False,
                   exit_after_display=True,
                   get_every=1,
                   first_and_last_frames=None,
                   density_custom_ranges=None,
                   write_pdb_with_centered_coordinates=False,
                   voxel_size=5.0):
        """ Get the best scoring models, compute a distance matrix, cluster them, and create density maps.
        Tuple format: "molname" just the molecule, or (start,stop,molname,copy_num(optional),state_num(optional)
        Can pass None for copy or state to ignore that field.
        If you don't pass a specific copy number
        @param score_key              The score for ranking models. Use "Total_Score"
               instead of default for PMI2.
        @param rmf_file_key           Key pointing to RMF filename
        @param rmf_file_frame_key     Key pointing to RMF frame number
        @param state_number           State number to analyze
        @param prefiltervalue         Only include frames where the
               score key is below this value
        @param feature_keys           Keywords for which you want to
               calculate average, medians, etc.
               If you pass "Keyname" it'll include everything that matches "*Keyname*"
        @param outputdir               The local output directory used in the run
        @param alignment_components    Dictionary with keys=groupname, values are tuples
               for aligning the structures  e.g. {"Rpb1": (20,100,"Rpb1"),"Rpb2":"Rpb2"}
        @param number_of_best_scoring_models  Num models to keep per run
        @param rmsd_calculation_components    For calculating RMSD
                                               (same format as alignment_components)
        @param distance_matrix_file           Where to store/read the distance matrix
        @param load_distance_matrix_file      Try to load the distance matrix file
        @param skip_clustering                Just extract the best scoring models
                                               and save the pdbs
        @param number_of_clusters             Number of k-means clusters
        @param display_plot                   Display the distance matrix
        @param exit_after_display             Exit after displaying distance matrix
        @param get_every                      Extract every nth frame
        @param first_and_last_frames          A tuple with the first and last frames to be
                                               analyzed. Values are percentages!
                                               Default: get all frames
        @param density_custom_ranges          For density calculation
                                               (same format as alignment_components)
        @param write_pdb_with_centered_coordinates
        @param voxel_size                     Used for the density output
        """
        # Track provenance information to be added to each output model
        prov = []
        self._outputdir = os.path.abspath(outputdir)
        self._number_of_clusters = number_of_clusters
        for p, state in self._protocol_output:
            p.add_replica_exchange_analysis(state, self, density_custom_ranges)

        if self.test_mode:
            return

        if self.rank==0:
            try:
                os.mkdir(outputdir)
            except:
                pass

        if not load_distance_matrix_file:
            if len(self.stat_files)==0: print("ERROR: no stat file found in the given path"); return
            my_stat_files = IMP.pmi.tools.chunk_list_into_segments(
                self.stat_files,self.number_of_processes)[self.rank]

            # read ahead to check if you need the PMI2 score key instead
            po = IMP.pmi.output.ProcessOutput(my_stat_files[0])
            orig_score_key = score_key
            if score_key not in po.get_keys():
                if 'Total_Score' in po.get_keys():
                    score_key = 'Total_Score'
                    warnings.warn(
                        "Using 'Total_Score' instead of "
                        "'SimplifiedModel_Total_Score_None' for the score key",
                        IMP.pmi.ParameterWarning)
            for k in [orig_score_key,score_key,rmf_file_key,rmf_file_frame_key]:
                if k in feature_keys:
                    warnings.warn(
                            "no need to pass " + k + " to feature_keys.",
                            IMP.pmi.ParameterWarning)
                    feature_keys.remove(k)

            best_models = IMP.pmi.io.get_best_models(my_stat_files,
                                                     score_key,
                                                     feature_keys,
                                                     rmf_file_key,
                                                     rmf_file_frame_key,
                                                     prefiltervalue,
                                                     get_every, provenance=prov)
            rmf_file_list=best_models[0]
            rmf_file_frame_list=best_models[1]
            score_list=best_models[2]
            feature_keyword_list_dict=best_models[3]

# ------------------------------------------------------------------------
# collect all the files and scores
# ------------------------------------------------------------------------

            if self.number_of_processes > 1:
                score_list = IMP.pmi.tools.scatter_and_gather(score_list)
                rmf_file_list = IMP.pmi.tools.scatter_and_gather(rmf_file_list)
                rmf_file_frame_list = IMP.pmi.tools.scatter_and_gather(
                    rmf_file_frame_list)
                for k in feature_keyword_list_dict:
                    feature_keyword_list_dict[k] = IMP.pmi.tools.scatter_and_gather(
                        feature_keyword_list_dict[k])

            # sort by score and get the best scoring ones
            score_rmf_tuples = list(zip(score_list,
                                   rmf_file_list,
                                   rmf_file_frame_list,
                                   list(range(len(score_list)))))

            if density_custom_ranges:
                for k in density_custom_ranges:
                    if type(density_custom_ranges[k]) is not list:
                        raise Exception("Density custom ranges: values must be lists of tuples")

            # keep subset of frames if requested
            if first_and_last_frames is not None:
                nframes = len(score_rmf_tuples)
                first_frame = int(first_and_last_frames[0] * nframes)
                last_frame = int(first_and_last_frames[1] * nframes)
                if last_frame > len(score_rmf_tuples):
                    last_frame = -1
                score_rmf_tuples = score_rmf_tuples[first_frame:last_frame]

            # sort RMFs by the score_key in ascending order, and store the rank
            best_score_rmf_tuples = sorted(score_rmf_tuples,
                                           key=lambda x: float(x[0]))[:number_of_best_scoring_models]
            best_score_rmf_tuples=[t+(n,) for n,t in enumerate(best_score_rmf_tuples)]
            # Note in the provenance info that we only kept best-scoring models
            prov.append(IMP.pmi.io.FilterProvenance("Best scoring",
                               0, number_of_best_scoring_models))
            # sort the feature scores in the same way
            best_score_feature_keyword_list_dict = defaultdict(list)
            for tpl in best_score_rmf_tuples:
                index = tpl[3]
                for f in feature_keyword_list_dict:
                    best_score_feature_keyword_list_dict[f].append(
                        feature_keyword_list_dict[f][index])
            my_best_score_rmf_tuples = IMP.pmi.tools.chunk_list_into_segments(
                best_score_rmf_tuples,
                self.number_of_processes)[self.rank]

            # expand the dictionaries to include ambiguous copies
            prot_ahead = IMP.pmi.analysis.get_hiers_from_rmf(self.model,
                                                             0,
                                                             my_best_score_rmf_tuples[0][1])[0]
            if IMP.pmi.get_is_canonical(prot_ahead):
                if rmsd_calculation_components is not None:
                    tmp = self._expand_ambiguity(prot_ahead,rmsd_calculation_components)
                    if tmp!=rmsd_calculation_components:
                        print('Detected ambiguity, expand rmsd components to',tmp)
                        rmsd_calculation_components = tmp
                if alignment_components is not None:
                    tmp = self._expand_ambiguity(prot_ahead,alignment_components)
                    if tmp!=alignment_components:
                        print('Detected ambiguity, expand alignment components to',tmp)
                        alignment_components = tmp

#-------------------------------------------------------------
# read the coordinates
# ------------------------------------------------------------
            rmsd_weights = IMP.pmi.io.get_bead_sizes(self.model,
                                                     my_best_score_rmf_tuples[0],
                                                     rmsd_calculation_components,
                                                     state_number=state_number)
            got_coords = IMP.pmi.io.read_coordinates_of_rmfs(self.model,
                                                             my_best_score_rmf_tuples,
                                                             alignment_components,
                                                             rmsd_calculation_components,
                                                             state_number=state_number)

            # note! the coordinates are simply float tuples, NOT decorators, NOT Vector3D,
            # NOR particles, because these object cannot be serialized. We need serialization
            # for the parallel computation based on mpi.
            all_coordinates=got_coords[0]          # dict:key=component name,val=coords per hit
            alignment_coordinates=got_coords[1]    # same as above, limited to alignment bits
            rmsd_coordinates=got_coords[2]         # same as above, limited to RMSD bits
            rmf_file_name_index_dict=got_coords[3] # dictionary with key=RMF, value=score rank
            all_rmf_file_names=got_coords[4]       # RMF file per hit

# ------------------------------------------------------------------------
# optionally don't compute distance matrix or cluster, just write top files
# ------------------------------------------------------------------------
            if skip_clustering:
                if density_custom_ranges:
                    DensModule = IMP.pmi.analysis.GetModelDensity(
                        density_custom_ranges,
                        voxel=voxel_size)

                dircluster=os.path.join(outputdir,"all_models."+str(n))
                try:
                    os.mkdir(outputdir)
                except:
                    pass
                try:
                    os.mkdir(dircluster)
                except:
                    pass
                clusstat=open(os.path.join(dircluster,"stat."+str(self.rank)+".out"),"w")
                for cnt,tpl in enumerate(my_best_score_rmf_tuples):
                    rmf_name=tpl[1]
                    rmf_frame_number=tpl[2]
                    tmp_dict={}
                    index=tpl[4]
                    for key in best_score_feature_keyword_list_dict:
                        tmp_dict[key]=best_score_feature_keyword_list_dict[key][index]

                    if cnt==0:
                        prots,rs = IMP.pmi.analysis.get_hiers_and_restraints_from_rmf(
                          self.model,
                          rmf_frame_number,
                          rmf_name)
                    else:
                        linking_successful=IMP.pmi.analysis.link_hiers_and_restraints_to_rmf(
                            self.model,
                            prots,
                            rs,
                            rmf_frame_number,
                            rmf_name)
                        if not linking_successful:
                            continue

                    if not prots:
                        continue

                    if IMP.pmi.get_is_canonical(prots[0]):
                        states = IMP.atom.get_by_type(prots[0],IMP.atom.STATE_TYPE)
                        prot = states[state_number]
                    else:
                        prot = prots[state_number]

                    # get transformation aligning coordinates of requested tuples
                    #  to the first RMF file
                    if cnt==0:
                        coords_f1=alignment_coordinates[cnt]
                    if cnt > 0:
                        coords_f2=alignment_coordinates[cnt]
                        if coords_f2:
                            Ali = IMP.pmi.analysis.Alignment(coords_f1, coords_f2)
                            transformation = Ali.align()[1]
                        else:
                            transformation = IMP.algebra.get_identity_transformation_3d()

                        rbs = set()
                        for p in IMP.atom.get_leaves(prot):
                            if not IMP.core.XYZR.get_is_setup(p):
                                IMP.core.XYZR.setup_particle(p)
                                IMP.core.XYZR(p).set_radius(0.0001)
                                IMP.core.XYZR(p).set_coordinates((0, 0, 0))

                            if IMP.core.RigidBodyMember.get_is_setup(p):
                                rb = IMP.core.RigidBodyMember(p).get_rigid_body()
                                rbs.add(rb)
                            else:
                                IMP.core.transform(IMP.core.XYZ(p),
                                                   transformation)
                        for rb in rbs:
                            IMP.core.transform(rb,transformation)

                    o=IMP.pmi.output.Output()
                    self.model.update()
                    out_pdb_fn=os.path.join(dircluster,str(cnt)+"."+str(self.rank)+".pdb")
                    out_rmf_fn=os.path.join(dircluster,str(cnt)+"."+str(self.rank)+".rmf3")
                    o.init_pdb(out_pdb_fn,prot)
                    o.write_pdb(out_pdb_fn,
                                translate_to_geometric_center=write_pdb_with_centered_coordinates)

                    tmp_dict["local_pdb_file_name"]=os.path.basename(out_pdb_fn)
                    tmp_dict["rmf_file_full_path"]=rmf_name
                    tmp_dict["local_rmf_file_name"]=os.path.basename(out_rmf_fn)
                    tmp_dict["local_rmf_frame_number"]=0

                    clusstat.write(str(tmp_dict)+"\n")

                    if IMP.pmi.get_is_canonical(prot):
                        # create a single-state System and write that
                        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.model))
                        h.set_name("System")
                        h.add_child(prot)
                        o.init_rmf(out_rmf_fn, [h], rs)
                    else:
                        o.init_rmf(out_rmf_fn, [prot],rs)

                    o.write_rmf(out_rmf_fn)
                    o.close_rmf(out_rmf_fn)
                    # add the density
                    if density_custom_ranges:
                        DensModule.add_subunits_density(prot)

                if density_custom_ranges:
                    DensModule.write_mrc(path=dircluster)
                    del DensModule
                return



            # broadcast the coordinates
            if self.number_of_processes > 1:
                all_coordinates = IMP.pmi.tools.scatter_and_gather(
                    all_coordinates)
                all_rmf_file_names = IMP.pmi.tools.scatter_and_gather(
                    all_rmf_file_names)
                rmf_file_name_index_dict = IMP.pmi.tools.scatter_and_gather(
                    rmf_file_name_index_dict)
                alignment_coordinates=IMP.pmi.tools.scatter_and_gather(
                    alignment_coordinates)
                rmsd_coordinates=IMP.pmi.tools.scatter_and_gather(
                    rmsd_coordinates)

            if self.rank == 0:
                # save needed informations in external files
                self.save_objects(
                    [best_score_feature_keyword_list_dict,
                     rmf_file_name_index_dict],
                    ".macro.pkl")

# ------------------------------------------------------------------------
# Calculate distance matrix and cluster
# ------------------------------------------------------------------------
            print("setup clustering class")
            self.cluster_obj = IMP.pmi.analysis.Clustering(rmsd_weights)

            for n, model_coordinate_dict in enumerate(all_coordinates):
                template_coordinate_dict = {}
                # let's try to align
                if alignment_components is not None and len(self.cluster_obj.all_coords) == 0:
                    # set the first model as template coordinates
                    self.cluster_obj.set_template(alignment_coordinates[n])
                self.cluster_obj.fill(all_rmf_file_names[n], rmsd_coordinates[n])
            print("Global calculating the distance matrix")

            # calculate distance matrix, all against all
            self.cluster_obj.dist_matrix()

            # perform clustering and optionally display
            if self.rank == 0:
                self.cluster_obj.do_cluster(number_of_clusters)
                if display_plot:
                    if self.rank == 0:
                        self.cluster_obj.plot_matrix(figurename=os.path.join(outputdir,'dist_matrix.pdf'))
                    if exit_after_display:
                        exit()
                self.cluster_obj.save_distance_matrix_file(file_name=distance_matrix_file)

# ------------------------------------------------------------------------
# Alteratively, load the distance matrix from file and cluster that
# ------------------------------------------------------------------------
        else:
            if self.rank==0:
                print("setup clustering class")
                self.cluster_obj = IMP.pmi.analysis.Clustering()
                self.cluster_obj.load_distance_matrix_file(file_name=distance_matrix_file)
                print("clustering with %s clusters" % str(number_of_clusters))
                self.cluster_obj.do_cluster(number_of_clusters)
                [best_score_feature_keyword_list_dict,
                 rmf_file_name_index_dict] = self.load_objects(".macro.pkl")
                if display_plot:
                    if self.rank == 0:
                        self.cluster_obj.plot_matrix(figurename=os.path.join(outputdir,'dist_matrix.pdf'))
                    if exit_after_display:
                        exit()
        if self.number_of_processes > 1:
            self.comm.Barrier()

# ------------------------------------------------------------------------
# now save all informations about the clusters
# ------------------------------------------------------------------------

        if self.rank == 0:
            print(self.cluster_obj.get_cluster_labels())
            for n, cl in enumerate(self.cluster_obj.get_cluster_labels()):
                print("rank %s " % str(self.rank))
                print("cluster %s " % str(n))
                print("cluster label %s " % str(cl))
                print(self.cluster_obj.get_cluster_label_names(cl))
                cluster_size = len(self.cluster_obj.get_cluster_label_names(cl))
                cluster_prov = prov + \
                               [IMP.pmi.io.ClusterProvenance(cluster_size)]

                # first initialize the Density class if requested
                if density_custom_ranges:
                    DensModule = IMP.pmi.analysis.GetModelDensity(
                        density_custom_ranges,
                        voxel=voxel_size)

                dircluster = outputdir + "/cluster." + str(n) + "/"
                try:
                    os.mkdir(dircluster)
                except:
                    pass

                rmsd_dict = {"AVERAGE_RMSD":
                             str(self.cluster_obj.get_cluster_label_average_rmsd(cl))}
                clusstat = open(dircluster + "stat.out", "w")
                for k, structure_name in enumerate(self.cluster_obj.get_cluster_label_names(cl)):
                    # extract the features
                    tmp_dict = {}
                    tmp_dict.update(rmsd_dict)
                    index = rmf_file_name_index_dict[structure_name]
                    for key in best_score_feature_keyword_list_dict:
                        tmp_dict[
                            key] = best_score_feature_keyword_list_dict[
                            key][
                            index]

                    # get the rmf name and the frame number from the list of
                    # frame names
                    rmf_name = structure_name.split("|")[0]
                    rmf_frame_number = int(structure_name.split("|")[1])
                    clusstat.write(str(tmp_dict) + "\n")

                    # extract frame (open or link to existing)
                    if k==0:
                        prots,rs = IMP.pmi.analysis.get_hiers_and_restraints_from_rmf(
                          self.model,
                          rmf_frame_number,
                          rmf_name)
                    else:
                        linking_successful = IMP.pmi.analysis.link_hiers_and_restraints_to_rmf(
                            self.model,
                            prots,
                            rs,
                            rmf_frame_number,
                            rmf_name)
                        if not linking_successful:
                            continue
                    if not prots:
                        continue

                    if IMP.pmi.get_is_canonical(prots[0]):
                        states = IMP.atom.get_by_type(prots[0],IMP.atom.STATE_TYPE)
                        prot = states[state_number]
                    else:
                        prot = prots[state_number]
                    if k==0:
                        IMP.pmi.io.add_provenance(cluster_prov, (prot,))

                    # transform clusters onto first
                    if k > 0:
                        model_index = self.cluster_obj.get_model_index_from_name(
                            structure_name)
                        transformation = self.cluster_obj.get_transformation_to_first_member(
                            cl,
                            model_index)
                        rbs = set()
                        for p in IMP.atom.get_leaves(prot):
                            if not IMP.core.XYZR.get_is_setup(p):
                                IMP.core.XYZR.setup_particle(p)
                                IMP.core.XYZR(p).set_radius(0.0001)
                                IMP.core.XYZR(p).set_coordinates((0, 0, 0))

                            if IMP.core.RigidBodyMember.get_is_setup(p):
                                rb = IMP.core.RigidBodyMember(p).get_rigid_body()
                                rbs.add(rb)
                            else:
                                IMP.core.transform(IMP.core.XYZ(p),
                                                   transformation)
                        for rb in rbs:
                            IMP.core.transform(rb,transformation)

                    # add the density
                    if density_custom_ranges:
                        DensModule.add_subunits_density(prot)

                    # pdb writing should be optimized!
                    o = IMP.pmi.output.Output()
                    self.model.update()
                    o.init_pdb(dircluster + str(k) + ".pdb", prot)
                    o.write_pdb(dircluster + str(k) + ".pdb")

                    if IMP.pmi.get_is_canonical(prot):
                        # create a single-state System and write that
                        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(self.model))
                        h.set_name("System")
                        h.add_child(prot)
                        o.init_rmf(dircluster + str(k) + ".rmf3", [h], rs)
                    else:
                        o.init_rmf(dircluster + str(k) + ".rmf3", [prot],rs)
                    o.write_rmf(dircluster + str(k) + ".rmf3")
                    o.close_rmf(dircluster + str(k) + ".rmf3")

                    del o
                    # IMP.atom.destroy(prot)

                if density_custom_ranges:
                    DensModule.write_mrc(path=dircluster)
                    del DensModule

        if self.number_of_processes>1:
            self.comm.Barrier()

    def get_cluster_rmsd(self,cluster_num):
        if self.cluster_obj is None:
            raise Exception("Run clustering first")
        return self.cluster_obj.get_cluster_label_average_rmsd(cluster_num)

    def save_objects(self, objects, file_name):
        import pickle
        outf = open(file_name, 'wb')
        pickle.dump(objects, outf)
        outf.close()

    def load_objects(self, file_name):
        import pickle
        inputf = open(file_name, 'rb')
        objects = pickle.load(inputf)
        inputf.close()
        return objects

class AnalysisReplicaExchange(object):

    """
    This class contains analysis utilities to investigate ReplicaExchange results.
    """

    ########################
    # Construction and Setup
    ########################

    def __init__(self,model,
                 stat_files,
                 best_models=None,
                 score_key=None,
                 alignment=True):
        """
        Construction of the Class.
        @param model IMP.Model()
        @param stat_files list of string. Can be ascii stat files, rmf files names
        @param best_models Integer. Number of best scoring models, if None: all models will be read
        @param alignment boolean (Default=True). Align before computing the rmsd.
        """

        self.model=model
        self.best_models=best_models
        self.stath0=IMP.pmi.output.StatHierarchyHandler(model,stat_files,self.best_models,score_key,cache=True)
        self.stath1=IMP.pmi.output.StatHierarchyHandler(StatHierarchyHandler=self.stath0)

        self.rbs1, self.beads1 = IMP.pmi.tools.get_rbs_and_beads(IMP.pmi.tools.select_at_all_resolutions(self.stath1))
        self.rbs0, self.beads0 = IMP.pmi.tools.get_rbs_and_beads(IMP.pmi.tools.select_at_all_resolutions(self.stath0))
        self.sel0_rmsd=IMP.atom.Selection(self.stath0)
        self.sel1_rmsd=IMP.atom.Selection(self.stath1)
        self.sel0_alignment=IMP.atom.Selection(self.stath0)
        self.sel1_alignment=IMP.atom.Selection(self.stath1)
        self.clusters=[]
        # fill the cluster list with a single cluster containing all models
        c = IMP.pmi.output.Cluster(0)
        self.clusters.append(c)
        for n0 in range(len(self.stath0)):
            c.add_member(n0)
        self.pairwise_rmsd={}
        self.pairwise_molecular_assignment={}
        self.alignment=alignment
        self.symmetric_molecules={}
        self.issymmetricsel={}
        self.update_seldicts()
        self.molcopydict0=IMP.pmi.tools.get_molecules_dictionary_by_copy(IMP.atom.get_leaves(self.stath0))
        self.molcopydict1=IMP.pmi.tools.get_molecules_dictionary_by_copy(IMP.atom.get_leaves(self.stath1))

    def set_rmsd_selection(self,**kwargs):
        """
        Setup the selection onto which the rmsd is computed
        @param kwargs use IMP.atom.Selection keywords
        """
        self.sel0_rmsd=IMP.atom.Selection(self.stath0,**kwargs)
        self.sel1_rmsd=IMP.atom.Selection(self.stath1,**kwargs)
        self.update_seldicts()

    def set_symmetric(self,molecule_name):
        """
        Store names of symmetric molecules
        """
        self.symmetric_molecules[molecule_name]=0
        self.update_seldicts()

    def set_alignment_selection(self,**kwargs):
        """
        Setup the selection onto which the alignment is computed
        @param kwargs use IMP.atom.Selection keywords
        """
        self.sel0_alignment=IMP.atom.Selection(self.stath0,**kwargs)
        self.sel1_alignment=IMP.atom.Selection(self.stath1,**kwargs)

    ######################
    # Clustering functions
    ######################
    def clean_clusters(self):
        for c in self.clusters: del c
        self.clusters=[]


    def cluster(self, rmsd_cutoff=10, metric=IMP.atom.get_rmsd):
        """
        Cluster the models based on RMSD.
        @param rmsd_cutoff Float the distance cutoff in Angstrom
        @param metric (Default=IMP.atom.get_rmsd) the metric that will be used to compute rmsds
        """
        self.clean_clusters()
        not_clustered=set(range(len(self.stath1)))
        while len(not_clustered)>0:
            self.aggregate(not_clustered, rmsd_cutoff, metric)
        #self.merge_aggregates(rmsd_cutoff, metric)
        self.update_clusters()

    def refine(self,rmsd_cutoff=10):
        """
        Refine the clusters by merging the ones whose centers are close
        @param rmsd_cutoff cutoff distance in Angstorms
        """
        merge_pairs=[]
        clusters_copy=self.clusters
        for c0,c1 in itertools.combinations(self.clusters,2):
            if c0.center_index is None:
                self.compute_cluster_center(c0)
            if c1.center_index is None:
                self.compute_cluster_center(c1)
            d0=self.stath0[c0.center_index]
            d1=self.stath1[c1.center_index]
            rmsd, molecular_assignment = self.rmsd()
            if rmsd <= rmsd_cutoff:
                if c1 in self.clusters:
                    clusters_copy.remove(c1)
                c0+=c1
        self.clusters=clusters_copy
        self.update_clusters()

    ####################
    # Input Output
    ####################

    def set_cluster_assignments(self, cluster_ids):
        if len(cluster_ids)!=len(self.stath0):
            raise ValueError('cluster ids has to be same length as number of frames')

        self.clusters=[]
        for i in sorted(list(set(cluster_ids))):
            self.clusters.append(IMP.pmi.output.Cluster(i))
        for i, (idx, d) in enumerate(zip(cluster_ids, self.stath0)):
            self.clusters[idx].add_member(i,d)

    def get_cluster_data(self, cluster):
        """
        Return the model data from a cluster
        @param cluster IMP.pmi.output.Cluster object
        """
        data=[]
        for m in cluster:
            data.append(m)
        return data

    def save_data(self,filename='data.pkl'):
        """
        Save the data for the whole models into a pickle file
        @param filename string
        """
        self.stath0.save_data(filename)

    def set_data(self,data):
        """
        Set the data from an external IMP.pmi.output.Data
        @param data IMP.pmi.output.Data
        """
        self.stath0.data=data
        self.stath1.data=data

    def load_data(self,filename='data.pkl'):
        """
        Load the data from an external pickled file
        @param filename string
        """
        self.stath0.load_data(filename)
        self.stath1.load_data(filename)
        self.best_models=len(self.stath0)

    def add_cluster(self,rmf_name_list):
        c = IMP.pmi.output.Cluster(len(self.clusters))
        print("creating cluster index "+str(len(self.clusters)))
        self.clusters.append(c)
        current_len=len(self.stath0)

        for rmf in rmf_name_list:
            print("adding rmf "+rmf)
            self.stath0.add_stat_file(rmf)
            self.stath1.add_stat_file(rmf)

        for n0 in range(current_len,len(self.stath0)):
            d0=self.stath0[n0]
            c.add_member(n0,d0)
        self.update_clusters()

    def save_clusters(self,filename='clusters.pkl'):
        """
        Save the clusters into a pickle file
        @param filename string
        """
        try:
            import cPickle as pickle
        except ImportError:
            import pickle
        fl=open(filename,'wb')
        pickle.dump(self.clusters,fl)

    def load_clusters(self,filename='clusters.pkl',append=False):
        """
        Load the clusters from a pickle file
        @param filename string
        @param append bool (Default=False), if True. append the clusters to the ones currently present
        """
        try:
            import cPickle as pickle
        except ImportError:
            import pickle
        fl=open(filename,'rb')
        self.clean_clusters()
        if append:
            self.clusters+=pickle.load(fl)
        else:
            self.clusters=pickle.load(fl)
        self.update_clusters()

    ####################
    # Analysis Functions
    ####################

    def compute_cluster_center(self,cluster):
        """
        Compute the cluster center for a given cluster
        """
        member_distance=defaultdict(float)

        for n0,n1 in itertools.combinations(cluster.members,2):
            d0=self.stath0[n0]
            d1=self.stath1[n1]
            rmsd, _ = self.rmsd()
            member_distance[n0]+=rmsd

        if len(member_distance)>0:
            cluster.center_index=min(member_distance, key=member_distance.get)
        else:
            cluster.center_index=cluster.members[0]

    def save_coordinates(self,cluster,rmf_name=None,reference="Absolute", prefix="./"):
        """
        Save the coordinates of the current cluster a single rmf file
        """
        print("saving coordinates",cluster)
        if self.alignment: self.set_reference(reference,cluster)
        o=IMP.pmi.output.Output()
        if rmf_name is None:
            rmf_name=prefix+'/'+str(cluster.cluster_id)+".rmf3"

        d1=self.stath1[cluster.members[0]]
        self.model.update()
        o.init_rmf(rmf_name, [self.stath1])
        for n1 in cluster.members:
            d1=self.stath1[n1]
            self.model.update()
            self.apply_molecular_assignments(n1)
            if self.alignment: self.align()
            o.write_rmf(rmf_name)
            self.undo_apply_molecular_assignments(n1)
        o.close_rmf(rmf_name)

    def prune_redundant_structures(self,rmsd_cutoff=10):
        """
        remove structures that are similar
        append it to a new cluster
        """
        print("pruning models")
        selected=0
        filtered=[selected]
        remaining=range(1,len(self.stath1),10)

        while len(remaining)>0:
            d0=self.stath0[selected]
            rm=[]
            for n1 in remaining:
                d1=self.stath1[n1]
                if self.alignment: self.align()
                d, _ = self.rmsd()
                if d<=rmsd_cutoff:
                    rm.append(n1)
                    print("pruning model %s, similar to model %s, rmsd %s"%(str(n1),str(selected),str(d)))
            remaining=[x for x in remaining if x not in rm]
            if len(remaining)==0: break
            selected=remaining[0]
            filtered.append(selected)
            remaining.pop(0)
        c = IMP.pmi.output.Cluster(len(self.clusters))
        self.clusters.append(c)
        for n0 in filtered:
            d0=self.stath0[n0]
            c.add_member(n0,d0)
        self.update_clusters()



    def precision(self,cluster):
        """
        Compute the precision of a cluster
        """
        npairs=0
        rmsd=0.0
        precision=None

        if not cluster.center_index is None:
            members1=[cluster.center_index]
        else:
            members1=cluster.members

        for n0 in members1:
            d0=self.stath0[n0]
            for n1 in cluster.members:
                if n0!=n1:
                    npairs+=1
                    d1=self.stath1[n1]
                    self.apply_molecular_assignments(n1)
                    tmp_rmsd, _ = self.rmsd()
                    rmsd+=tmp_rmsd
                    self.undo_apply_molecular_assignments(n1)

        if npairs>0:
            precision=rmsd/npairs
        cluster.precision=precision
        return precision

    def bipartite_precision(self,cluster1,cluster2,verbose=False):
        """
        Compute the bipartite precision (ie the cross-precision)
        between two clusters
        """
        npairs=0
        rmsd=0.0
        for cn0,n0 in enumerate(cluster1.members):
            d0=self.stath0[n0]
            for cn1,n1 in enumerate(cluster2.members):
                d1=self.stath1[n1]
                tmp_rmsd, _ =self.rmsd()
                if verbose: print("--- rmsd between structure %s and structure %s is %s"%(str(cn0),str(cn1),str(tmp_rmsd)))
                rmsd+=tmp_rmsd
                npairs+=1
        precision=rmsd/npairs
        return precision

    def rmsf(self,cluster,molecule,copy_index=0,state_index=0,cluster_ref=None,step=1):
        """
        Compute the Root mean square fluctuations
        of a molecule in a cluster
        Returns an IMP.pmi.tools.OrderedDict() where the keys are the residue indexes and the value is the rmsf
        """
        rmsf=IMP.pmi.tools.OrderedDict()

        #assumes that residue indexes are identical for stath0 and stath1
        if cluster_ref is not None:
            if not cluster_ref.center_index is None:
                members0 = [cluster_ref.center_index]
            else:
                members0 = cluster_ref.members
        else:
            if not cluster.center_index is None:
                members0 = [cluster.center_index]
            else:
                members0 = cluster.members

        s0=IMP.atom.Selection(self.stath0,molecule=molecule,resolution=1,
                              copy_index=copy_index,state_index=state_index)
        ps0=s0.get_selected_particles()
        #get the residue indexes
        residue_indexes=list(IMP.pmi.tools.OrderedSet([IMP.pmi.tools.get_residue_indexes(p)[0] for p in ps0]))

        #get the corresponding particles
        #s0=IMP.atom.Selection(stat_ref,molecule=molecule,residue_indexes=residue_indexes,resolution=1,
        #                      copy_index=copy_index,state_index=state_index)
        #ps0 = s0.get_selected_particles()



        npairs=0
        for n0 in members0:
            d0=self.stath0[n0]
            for n1 in cluster.members[::step]:
                if n0!=n1:
                    print("--- rmsf %s %s"%(str(n0),str(n1)))
                    self.apply_molecular_assignments(n1)

                    s1=IMP.atom.Selection(self.stath1,molecule=molecule,residue_indexes=residue_indexes,resolution=1,
                                          copy_index=copy_index,state_index=state_index)
                    ps1 = s1.get_selected_particles()

                    d1=self.stath1[n1]
                    if self.alignment: self.align()
                    for n,(p0,p1) in enumerate(zip(ps0,ps1)):
                        r=residue_indexes[n]
                        d0=IMP.core.XYZ(p0)
                        d1=IMP.core.XYZ(p1)
                        if r in rmsf:
                            rmsf[r]+=IMP.core.get_distance(d0,d1)
                        else:
                            rmsf[r]=IMP.core.get_distance(d0,d1)
                    npairs+=1
                    self.undo_apply_molecular_assignments(n1)
        for r in rmsf:
            rmsf[r]/=npairs

            for stath in [self.stath0,self.stath1]:
                if molecule not in self.symmetric_molecules:
                    s=IMP.atom.Selection(stath,molecule=molecule,residue_index=r,resolution=1,
                                  copy_index=copy_index,state_index=state_index)
                else:
                    s=IMP.atom.Selection(stath,molecule=molecule,residue_index=r,resolution=1,
                                 state_index=state_index)

                ps = s.get_selected_particles()
                for p in ps:
                    if IMP.pmi.Uncertainty.get_is_setup(p):
                        IMP.pmi.Uncertainty(p).set_uncertainty(rmsf[r])
                    else:
                        IMP.pmi.Uncertainty.setup_particle(p,rmsf[r])

        return rmsf

    def save_densities(self,cluster,density_custom_ranges,voxel_size=5,reference="Absolute", prefix="./",step=1):
        if self.alignment: self.set_reference(reference,cluster)
        dens = IMP.pmi.analysis.GetModelDensity(density_custom_ranges,
                                                voxel=voxel_size)

        for n1 in cluster.members[::step]:
            print("density "+str(n1))
            d1=self.stath1[n1]
            self.apply_molecular_assignments(n1)
            if self.alignment: self.align()
            dens.add_subunits_density(self.stath1)
            self.undo_apply_molecular_assignments(n1)
        dens.write_mrc(path=prefix+'/',suffix=str(cluster.cluster_id))
        del dens

    def contact_map(self,cluster,contact_threshold=15,log_scale=False,consolidate=False,molecules=None,prefix='./',reference="Absolute"):
        if self.alignment: self.set_reference(reference,cluster)
        import numpy as np
        import matplotlib.pyplot as plt
        import matplotlib.cm as cm
        from scipy.spatial.distance import cdist
        import IMP.pmi.topology
        if molecules is None:
            mols=[IMP.pmi.topology.PMIMoleculeHierarchy(mol) for mol in IMP.pmi.tools.get_molecules(IMP.atom.get_leaves(self.stath1))]
        else:
            mols=[IMP.pmi.topology.PMIMoleculeHierarchy(mol) for mol in IMP.pmi.tools.get_molecules(IMP.atom.Selection(self.stath1,molecules=molecules).get_selected_particles())]
        unique_copies=[mol for mol in mols if mol.get_copy_index() == 0]
        mol_names_unique=dict((mol.get_name(),mol) for mol in unique_copies)
        total_len_unique=sum(max(mol.get_residue_indexes()) for mol in unique_copies)


        # coords = np.ones((total_len,3)) * 1e6 #default to coords "very far away"
        index_dict={}
        prev_stop=0

        if not consolidate:
            for mol in mols:
                seqlen=max(mol.get_residue_indexes())
                index_dict[mol] = range(prev_stop, prev_stop + seqlen)
                prev_stop+=seqlen

        else:
            for mol in unique_copies:
                seqlen=max(mol.get_residue_indexes())
                index_dict[mol] = range(prev_stop, prev_stop + seqlen)
                prev_stop+=seqlen


        for ncl,n1 in enumerate(cluster.members):
            print(ncl)
            d1=self.stath1[n1]
            #self.apply_molecular_assignments(n1)
            coord_dict=IMP.pmi.tools.OrderedDict()
            for mol in mols:
                mol_name=mol.get_name()
                copy_index=mol.get_copy_index()
                rindexes = mol.get_residue_indexes()
                coords = np.ones((max(rindexes),3))
                for rnum in rindexes:
                    sel = IMP.atom.Selection(mol, residue_index=rnum, resolution=1)
                    selpart = sel.get_selected_particles()
                    if len(selpart) == 0: continue
                    selpart = selpart[0]
                    coords[rnum - 1, :] = IMP.core.XYZ(selpart).get_coordinates()
                coord_dict[mol]=coords

            if not consolidate:
                coords=np.concatenate(list(coord_dict.values()))
                dists = cdist(coords, coords)
                binary_dists = np.where((dists <= contact_threshold) & (dists >= 1.0), 1.0, 0.0)
            else:
                binary_dists_dict={}
                for mol1 in mols:
                    len1 = max(mol1.get_residue_indexes())
                    for mol2 in mols:
                        name1=mol1.get_name()
                        name2=mol2.get_name()
                        dists = cdist(coord_dict[mol1], coord_dict[mol2])
                        if (name1, name2) not in binary_dists_dict:
                            binary_dists_dict[(name1, name2)] = np.zeros((len1,len1))
                        binary_dists_dict[(name1, name2)] += np.where((dists <= contact_threshold) & (dists >= 1.0), 1.0, 0.0)
                binary_dists=np.zeros((total_len_unique,total_len_unique))

                for name1,name2 in  binary_dists_dict:
                    r1=index_dict[mol_names_unique[name1]]
                    r2=index_dict[mol_names_unique[name2]]
                    binary_dists[min(r1):max(r1)+1,min(r2):max(r2)+1] = np.where((binary_dists_dict[(name1, name2)]>=1.0),1.0,0.0)

            if ncl==0:
                dist_maps = [dists]
                av_dist_map = dists
                contact_freqs = binary_dists
            else:
                dist_maps.append(dists)
                av_dist_map += dists
                contact_freqs += binary_dists

            #self.undo_apply_molecular_assignments(n1)

        if log_scale:
            contact_freqs =-np.log(1.0-1.0/(len(cluster)+1)*contact_freqs)
        else:
            contact_freqs =1.0/len(cluster)*contact_freqs
        av_dist_map=1.0/len(cluster)*contact_freqs

        fig = plt.figure(figsize=(100, 100))
        ax = fig.add_subplot(111)
        ax.set_xticks([])
        ax.set_yticks([])
        gap_between_components=50
        colormap = cm.Blues
        colornorm=None


        #if cbar_labels is not None:
        #    if len(cbar_labels)!=4:
        #        print("to provide cbar labels, give 3 fields (first=first input file, last=last input) in oppose order of input contact maps")
        #        exit()
        # set the list of proteins on the x axis
        if not consolidate:
            sorted_tuple=sorted([(IMP.pmi.topology.PMIMoleculeHierarchy(mol).get_extended_name(),mol) for mol in mols])
            prot_list=list(zip(*sorted_tuple))[1]
        else:
            sorted_tuple=sorted([(IMP.pmi.topology.PMIMoleculeHierarchy(mol).get_name(),mol) for mol in unique_copies])
            prot_list=list(zip(*sorted_tuple))[1]

        prot_listx = prot_list
        nresx = gap_between_components + \
            sum([max(mol.get_residue_indexes())
                + gap_between_components for mol in prot_listx])

        # set the list of proteins on the y axis
        prot_listy = prot_list
        nresy = gap_between_components + \
            sum([max(mol.get_residue_indexes())
                + gap_between_components for mol in prot_listy])

        # this is the residue offset for each protein
        resoffsetx = {}
        resendx = {}
        res = gap_between_components
        for mol in prot_listx:
            resoffsetx[mol] = res
            res += max(mol.get_residue_indexes())
            resendx[mol] = res
            res += gap_between_components

        resoffsety = {}
        resendy = {}
        res = gap_between_components
        for mol in prot_listy:
            resoffsety[mol] = res
            res += max(mol.get_residue_indexes())
            resendy[mol] = res
            res += gap_between_components

        resoffsetdiagonal = {}
        res = gap_between_components
        for mol in IMP.pmi.tools.OrderedSet(prot_listx + prot_listy):
            resoffsetdiagonal[mol] = res
            res += max(mol.get_residue_indexes())
            res += gap_between_components

        # plot protein boundaries
        xticks = []
        xlabels = []
        for n, prot in enumerate(prot_listx):
            res = resoffsetx[prot]
            end = resendx[prot]
            for proty in prot_listy:
                resy = resoffsety[proty]
                endy = resendy[proty]
                ax.plot([res, res], [resy, endy], linestyle='-',color='gray', lw=0.4)
                ax.plot([end, end], [resy, endy], linestyle='-',color='gray', lw=0.4)
            xticks.append((float(res) + float(end)) / 2)
            xlabels.append(IMP.pmi.topology.PMIMoleculeHierarchy(prot).get_extended_name())

        yticks = []
        ylabels = []
        for n, prot in enumerate(prot_listy):
            res = resoffsety[prot]
            end = resendy[prot]
            for protx in prot_listx:
                resx = resoffsetx[protx]
                endx = resendx[protx]
                ax.plot([resx, endx], [res, res], linestyle='-',color='gray', lw=0.4)
                ax.plot([resx, endx], [end, end], linestyle='-',color='gray', lw=0.4)
            yticks.append((float(res) + float(end)) / 2)
            ylabels.append(IMP.pmi.topology.PMIMoleculeHierarchy(prot).get_extended_name())

        # plot the contact map

        tmp_array = np.zeros((nresx, nresy))
        ret={}
        for px in prot_listx:
            for py in prot_listy:
                resx = resoffsetx[px]
                lengx = resendx[px] - 1
                resy = resoffsety[py]
                lengy = resendy[py] - 1
                indexes_x = index_dict[px]
                minx = min(indexes_x)
                maxx = max(indexes_x)
                indexes_y = index_dict[py]
                miny = min(indexes_y)
                maxy = max(indexes_y)
                tmp_array[resx:lengx,resy:lengy] = contact_freqs[minx:maxx,miny:maxy]
                ret[(px,py)]=np.argwhere(contact_freqs[minx:maxx,miny:maxy] == 1.0)+1

        cax = ax.imshow(tmp_array,
                  cmap=colormap,
                  norm=colornorm,
                  origin='lower',
                  alpha=0.6,
                  interpolation='nearest')

        ax.set_xticks(xticks)
        ax.set_xticklabels(xlabels, rotation=90)
        ax.set_yticks(yticks)
        ax.set_yticklabels(ylabels)
        plt.setp(ax.get_xticklabels(), fontsize=6)
        plt.setp(ax.get_yticklabels(), fontsize=6)

        # display and write to file
        fig.set_size_inches(0.005 * nresx, 0.005 * nresy)
        [i.set_linewidth(2.0) for i in ax.spines.values()]
        #if cbar_labels is not None:
        #    cbar = fig.colorbar(cax, ticks=[0.5,1.5,2.5,3.5])
        #    cbar.ax.set_yticklabels(cbar_labels)# vertically oriented colorbar

        plt.savefig(prefix+"/contact_map."+str(cluster.cluster_id)+".pdf", dpi=300,transparent="False")
        return ret


    def plot_rmsd_matrix(self,filename):
        import numpy as np
        self.compute_all_pairwise_rmsd()
        distance_matrix = np.zeros(
            (len(self.stath0), len(self.stath1)))
        for (n0,n1) in self.pairwise_rmsd:
            distance_matrix[n0, n1] = self.pairwise_rmsd[(n0,n1)]

        import matplotlib as mpl
        mpl.use('Agg')
        import matplotlib.pylab as pl
        from scipy.cluster import hierarchy as hrc

        fig = pl.figure(figsize=(10,8))
        ax = fig.add_subplot(212)
        dendrogram = hrc.dendrogram(
            hrc.linkage(distance_matrix),
            color_threshold=7,
            no_labels=True)
        leaves_order = dendrogram['leaves']
        ax.set_xlabel('Model')
        ax.set_ylabel('RMSD [Angstroms]')

        ax2 = fig.add_subplot(221)
        cax = ax2.imshow(
            distance_matrix[leaves_order,
                                     :][:,
                                        leaves_order],
            interpolation='nearest')
        cb = fig.colorbar(cax)
        cb.set_label('RMSD [Angstroms]')
        ax2.set_xlabel('Model')
        ax2.set_ylabel('Model')

        pl.savefig(filename, dpi=300)
        pl.close(fig)

    ####################
    # Internal Functions
    ####################

    def update_clusters(self):
        """
        Update the cluster id numbers
        """
        for n,c in enumerate(self.clusters):
            c.cluster_id=n

    def get_molecule(self, hier, name, copy):
        s=IMP.atom.Selection(hier, molecule=name, copy_index=copy)
        return IMP.pmi.tools.get_molecules(s.get_selected_particles()[0])[0]

    def update_seldicts(self):
        """
        Update the seldicts
        """
        self.seldict0=IMP.pmi.tools.get_selections_dictionary(self.sel0_rmsd.get_selected_particles())
        self.seldict1=IMP.pmi.tools.get_selections_dictionary(self.sel1_rmsd.get_selected_particles())
        for mol in self.seldict0:
            for sel in self.seldict0[mol]:
                self.issymmetricsel[sel]=False
        for mol in self.symmetric_molecules:
            self.symmetric_molecules[mol]=len(self.seldict0[mol])
            for sel in self.seldict0[mol]:
                self.issymmetricsel[sel]=True


    def align(self):
        print("alignment")
        tr = IMP.atom.get_transformation_aligning_first_to_second(self.sel1_alignment, self.sel0_alignment)

        for rb in self.rbs1:
            IMP.core.transform(rb, tr)

        for bead in self.beads1:
            IMP.core.transform(IMP.core.XYZ(bead), tr)

        self.model.update()

    def aggregate(self, idxs, rmsd_cutoff=10, metric=IMP.atom.get_rmsd):
        '''
        initial filling of the clusters.
        '''
        n0 = idxs.pop()
        print("clustering model "+str(n0))
        d0 = self.stath0[n0]
        c = IMP.pmi.output.Cluster(len(self.clusters))
        print("creating cluster index "+str(len(self.clusters)))
        self.clusters.append(c)
        c.add_member(n0,d0)
        clustered = set([n0])
        for n1 in idxs:
            print("--- trying to add model "+str(n1)+" to cluster "+str(len(self.clusters)))
            d1 = self.stath1[n1]
            if self.alignment: self.align()
            rmsd, _ = self.rmsd(metric=metric)
            if rmsd<rmsd_cutoff:
                print("--- model "+str(n1)+" added, rmsd="+str(rmsd))
                c.add_member(n1,d1)
                clustered.add(n1)
            else:
                print("--- model "+str(n1)+" NOT added, rmsd="+str(rmsd))
        idxs-=clustered

    def merge_aggregates(self, rmsd_cutoff, metric=IMP.atom.get_rmsd):
        """
        merge the clusters that have close members
        @param rmsd_cutoff cutoff distance in Angstorms
        """
        # before merging, clusters are spheres of radius rmsd_cutoff centered on the 1st element
        # here we only try to merge clusters whose centers are closer than 2*rmsd_cutoff
        to_merge = []
        print("merging...")
        for c0, c1 in filter(lambda x: len(x[0].members)>1, itertools.combinations(self.clusters, 2)):
            n0, n1 = [c.members[0] for c in (c0,c1)]
            d0 = self.stath0[n0]
            d1 = self.stath1[n1]
            rmsd, _ = self.rmsd()
            if rmsd<2*rmsd_cutoff and self.have_close_members(c0,c1,rmsd_cutoff,metric):
                to_merge.append((c0,c1))

        for c0, c in reversed(to_merge):
            self.merge(c0,c)

        #keep only full clusters
        self.clusters = [c for c in filter(lambda x: len(x.members)>0, self.clusters)]


    def have_close_members(self, c0, c1, rmsd_cutoff, metric):
        '''
        returns true if c0 and c1 have members that are closer than rmsd_cutoff
        '''
        print("check close members for clusters "+str(c0.cluster_id)+" and "+str(c1.cluster_id))
        for n0, n1 in itertools.product(c0.members[1:], c1.members):
            d0 = self.stath0[n0]
            d1 = self.stath1[n1]
            rmsd, _ = self.rmsd(metric=metric)
            if rmsd<rmsd_cutoff:
                return True

        return False


    def merge(self, c0, c1):
        '''
        merge two clusters
        '''
        c0+=c1
        c1.members=[]
        c1.data={}

    def rmsd_helper(self, sels0, sels1, metric):
        '''
        a function that returns the permutation best_sel of sels0 that minimizes metric
        '''
        best_rmsd2 = float('inf')
        best_sel = None
        if self.issymmetricsel[sels0[0]]:
            #this cases happens when symmetries were defined
            N = len(sels0)
            for offset in range(N):
                order=[(offset+i)%N for i in range(N)]
                sels = [sels0[(offset+i)%N] for i in range(N)]
                sel0 = sels[0]
                sel1 = sels1[0]
                r=metric(sel0, sel1)
                rmsd2=r*r*N
                ###print(order,rmsd2)
                if rmsd2 < best_rmsd2:
                    best_rmsd2 = rmsd2
                    best_sel = sels
                    best_order=order
        else:
            for sels in itertools.permutations(sels0):
                rmsd2=0.0
                for sel0, sel1 in itertools.takewhile(lambda x: rmsd2<best_rmsd2, zip(sels, sels1)):
                    r=metric(sel0, sel1)
                    rmsd2+=r*r
                if rmsd2 < best_rmsd2:
                    best_rmsd2 = rmsd2
                    best_sel = sels
        ###for i,sel in enumerate(best_sel):
        ###    p0 = sel.get_selected_particles()[0]
        ###    p1 = sels1[i].get_selected_particles()[0]
        ###    m0 = IMP.pmi.tools.get_molecules([p0])[0]
        ###    m1 = IMP.pmi.tools.get_molecules([p1])[0]
        ###    c0 = IMP.atom.Copy(m0).get_copy_index()
        ###    c1 = IMP.atom.Copy(m1).get_copy_index()
        ###    name0=m0.get_name()
        ###    name1=m1.get_name()
        ###    print("WWW",name0,name1,c0,c1)
        ###print(best_order,best_rmsd2,m0,m1)
        return  best_sel, best_rmsd2

    def compute_all_pairwise_rmsd(self):
        for d0 in self.stath0:
            for d1 in self.stath1:
                rmsd, _ = self.rmsd()

    def rmsd(self,metric=IMP.atom.get_rmsd):
        '''
        Computes the RMSD. Resolves ambiguous pairs assignments
        '''
        # here we memoize the rmsd and molecular assignment so that it's not done multiple times
        n0=self.stath0.current_index
        n1=self.stath1.current_index
        if ((n0,n1) in self.pairwise_rmsd) and ((n0,n1) in self.pairwise_molecular_assignment):
            return self.pairwise_rmsd[(n0,n1)], self.pairwise_molecular_assignment[(n0,n1)]

        if self.alignment:
            self.align()
        #if it's not yet memoized
        total_rmsd=0.0
        total_N=0
        # this is a dictionary which keys are the molecule names, and values are the list of IMP.atom.Selection for all molecules that share the molecule name
        seldict_best_order={}
        molecular_assignment={}
        for molname, sels0 in self.seldict0.items():
            sels_best_order, best_rmsd2 = self.rmsd_helper(sels0, self.seldict1[molname], metric)

            Ncoords = len(sels_best_order[0].get_selected_particles())
            Ncopies = len(self.seldict1[molname])
            total_rmsd += Ncoords*best_rmsd2
            total_N += Ncoords*Ncopies

            for sel0, sel1 in zip(sels_best_order, self.seldict1[molname]):
                p0 = sel0.get_selected_particles()[0]
                p1 = sel1.get_selected_particles()[0]
                m0 = IMP.pmi.tools.get_molecules([p0])[0]
                m1 = IMP.pmi.tools.get_molecules([p1])[0]
                c0 = IMP.atom.Copy(m0).get_copy_index()
                c1 = IMP.atom.Copy(m1).get_copy_index()
                ###print(molname,c0,c1)
                molecular_assignment[(molname,c0)]=(molname,c1)

        total_rmsd = math.sqrt(total_rmsd/total_N)

        self.pairwise_rmsd[(n0,n1)]=total_rmsd
        self.pairwise_molecular_assignment[(n0,n1)]=molecular_assignment
        self.pairwise_rmsd[(n1,n0)]=total_rmsd
        self.pairwise_molecular_assignment[(n1,n0)]=molecular_assignment
        ###print(n0,n1,molecular_assignment)
        return total_rmsd, molecular_assignment

    def set_reference(self,reference,cluster):
        """
        Fix the reference structure for structural alignment, rmsd and chain assignemnt
        @param reference can be either "Absolute" (cluster center of the first cluster)
                        or Relative (cluster center of the current cluster)
        """
        if reference=="Absolute":
            d0=self.stath0[0]
        elif reference=="Relative":
            if cluster.center_index:
                n0=cluster.center_index
            else:
                n0=cluster.members[0]
            d0=self.stath0[n0]

    def apply_molecular_assignments(self, n1):
        """
        compute the molecular assignemnts between multiple copies
        of the same sequence. It changes the Copy index of Molecules
        """
        d1=self.stath1[n1]
        _, molecular_assignment = self.rmsd()
        for (m0, c0), (m1,c1) in molecular_assignment.items():
            mol0 = self.molcopydict0[m0][c0]
            mol1 = self.molcopydict1[m1][c1]
            cik0=IMP.atom.Copy(mol0).get_copy_index_key()
            p1=IMP.atom.Copy(mol1).get_particle()
            p1.set_value(cik0,c0)

    def undo_apply_molecular_assignments(self, n1):
        """
        Undo the Copy index assignment
        """
        d1=self.stath1[n1]
        _, molecular_assignment = self.rmsd()
        mols_newcopys = []
        for (m0, c0), (m1,c1) in molecular_assignment.items():
            mol0 = self.molcopydict0[m0][c0]
            mol1 = self.molcopydict1[m1][c1]
            cik0=IMP.atom.Copy(mol0).get_copy_index_key()
            p1=IMP.atom.Copy(mol1).get_particle()
            p1.set_value(cik0,c1)

    ####################
    # Container Functions
    ####################

    def __repr__(self):
        s= "AnalysisReplicaExchange\n"
        s+="---- number of clusters %s \n"%str(len(self.clusters))
        s+="---- number of models %s \n"%str(len(self.stath0))
        return s

    def __getitem__(self,int_slice_adaptor):
        if type(int_slice_adaptor) is int:
            return self.clusters[int_slice_adaptor]
        elif type(int_slice_adaptor) is slice:
            return self.__iter__(int_slice_adaptor)
        else:
            raise TypeError("Unknown Type")

    def __len__(self):
        return len(self.clusters)

    def __iter__(self,slice_key=None):
        if slice_key is None:
            for i in range(len(self)):
                yield self[i]
        else:
            for i in range(len(self))[slice_key]:
                yield self[i]
