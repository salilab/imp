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
                 crosslink_restraints=None,
                 monte_carlo_temperature=1.0,
                 simulated_annealing=False,
                 simulated_annealing_minimum_temperature=1.0,
                 simulated_annealing_maximum_temperature=2.5,
                 simulated_annealing_minimum_temperature_nframes=100,
                 simulated_annealing_maximum_temperature_nframes=100,
                 replica_exchange_minimum_temperature=1.0,
                 replica_exchange_maximum_temperature=2.5,
                 num_sample_rounds=1,
                 number_of_best_scoring_models=500,
                 monte_carlo_steps=10,
                 molecular_dynamics_steps=10,
                 molecular_dynamics_max_time_step=1.0,
                 number_of_frames=1000,
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
                  that will be included in output (statistics files). Any object
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
           @param num_sample_rounds        Number of rounds of MC/MD per cycle
           @param number_of_best_scoring_models Number of top-scoring PDB models
                  to keep around for analysis
           @param monte_carlo_steps        Number of MC steps per round
           @param molecular_dynamics_steps  Number of MD steps per round
           @param molecular_dynamics_max_time_step Max time step for MD
           @param number_of_frames         Number of REX frames to run
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

        self.crosslink_restraints = crosslink_restraints
        self.em_object_for_rmf = em_object_for_rmf
        self.monte_carlo_sample_objects = monte_carlo_sample_objects
        if sample_objects is not None:
            self.monte_carlo_sample_objects+=sample_objects
        self.molecular_dynamics_sample_objects=molecular_dynamics_sample_objects
        self.output_objects = output_objects
        self.replica_exchange_object = replica_exchange_object
        self.molecular_dynamics_max_time_step = molecular_dynamics_max_time_step
        self.vars["monte_carlo_temperature"] = monte_carlo_temperature
        self.vars[
            "replica_exchange_minimum_temperature"] = replica_exchange_minimum_temperature
        self.vars[
            "replica_exchange_maximum_temperature"] = replica_exchange_maximum_temperature

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
        if root_hier:
            self.output_objects.append(IMP.pmi.io.TotalScoreOutput(self.model))

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
            self.output_objects.append(sampler_mc)
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
            self.output_objects.append(sampler_md)
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
        self.output_objects.append(rex)

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
        self.output_objects.append(sw)

        print("Setting up stat file")
        output = IMP.pmi.output.Output(atomistic=self.vars["atomistic"])
        low_temp_stat_file = globaldir + \
            self.vars["stat_file_name_suffix"] + "." + str(myindex) + ".out"
        if not self.test_mode:
            output.init_stat2(low_temp_stat_file,
                              self.output_objects,
                              extralabels=["rmf_file", "rmf_frame_index"])

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
                            output_hierarchies)
            if self.crosslink_restraints:
                output.add_restraints_to_rmf(
                    init_suffix + "." + str(myindex) + ".rmf3",
                    self.crosslink_restraints)
            output.write_rmf(init_suffix + "." + str(myindex) + ".rmf3")
            output.close_rmf(init_suffix + "." + str(myindex) + ".rmf3")

#----------------------------------------------

        if not self.test_mode:
            print("Setting up production rmf files")
            rmfname = rmf_dir + "/" + str(myindex) + ".rmf3"
            output.init_rmf(rmfname, output_hierarchies, geometries=self.vars["geometries"])

            if self.crosslink_restraints:
                output.add_restraints_to_rmf(rmfname, self.crosslink_restraints)

        ntimes_at_low_temp = 0

        if myindex == 0:
            self.show_info()
        self.replica_exchange_object.set_was_used(True)
        for i in range(self.vars["number_of_frames"]):
            for nr in range(self.vars["num_sample_rounds"]):
                if sampler_md is not None:
                    sampler_md.optimize(self.vars["molecular_dynamics_steps"])
                if sampler_mc is not None:
                    sampler_mc.optimize(self.vars["monte_carlo_steps"])
            score = IMP.pmi.tools.get_restraint_set(self.model).evaluate(False)
            output.set_output_entry("score", score)

            my_temp_index = int(rex.get_my_temp() * temp_index_factor)

            if min_temp_index == my_temp_index:
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
                    output.write_stat2(low_temp_stat_file)
                ntimes_at_low_temp += 1

            if not self.test_mode:
                output.write_stat2(replica_stat_file)
            rex.swap_temp(i, score)

# ----------------------------------------------------------------------
class BuildSystem(object):
    """A macro to build a IMP::pmi::topology::System based on a TopologyReader object.
    Easily create multi-state systems by calling this macro
    repeatedly with different TopologyReader objects!
    A useful function is get_molecules() which returns the PMI Molecules grouped by state
    as a dictionary with key = (molecule name), value = IMP.pmi.topology.Molecule

    Quick multi-state system:
    @code{.python}
    mdl = IMP.Model()
    reader1 = IMP.pmi.topology.TopologyReader(tfile1)
    reader2 = IMP.pmi.topology.TopologyReader(tfile2)

    bs = IMP.pmi.macros.BuildSystem(mdl)
    bs.add_state(reader1)
    bs.add_state(reader2)
    bs.execute_macro() # build everything including degrees of freedom

    IMP.atom.show_molecular_hierarchy(bs.get_hierarchy())
    ### now you have a two state system, you add restraints etc
    @endcode

    \note The "domain name" entry of the topology reader is not used.
    All molecules are set up by the component name, but split into rigid bodies
    as requested.
    """
    def __init__(self,
                 mdl,
                 sequence_connectivity_scale=4.0,
                 force_create_gmm_files=False):
        """Constructor
        @param mdl An IMP Model
        @param sequence_connectivity_scale For scaling the connectivity restraint
        @param force_create_gmm_files If True, will sample and create GMMs
                  no matter what. If False, will only only sample if the
                  files don't exist. If number of Gaussians is zero, won't
                  do anything.
        """
        self.mdl = mdl
        self.system = IMP.pmi.topology.System(self.mdl)
        self._readers = [] #internal storage of the TopologyReaders (one per state)
        self._domains = [] #internal storage of key=domain name, value=(atomic_res,non_atomic_res). (one per state)
        self.force_create_gmm_files = force_create_gmm_files
    def add_state(self,reader):
        """Add a state using the topology info in a IMP::pmi::topology::TopologyReader object.
        When you are done adding states, call execute_macro()
        @param reader The TopologyReader object
        """
        state = self.system.create_state()
        self._readers.append(reader)
        these_domains = {}

        ### setup representation
        # setup each component once, then each domain gets seperate structure, representation
        for molname in reader.get_unique_molecules():
            mlist = reader.get_unique_molecules()[molname]
            seq = IMP.pmi.topology.Sequences(mlist[0].fasta_file)
            mol = state.create_molecule(molname,seq[mlist[0].fasta_id],mlist[0].chain)
            for domain in mlist:
                # we build everything in the residue range, even if it extends beyond what's in the actual PDB file
                if domain.residue_range==[] or domain.residue_range is None:
                    domain_res = mol.get_residues()
                else:
                    start = domain.residue_range[0]+domain.pdb_offset
                    if domain.residue_range[1]==-1:
                        end = -1
                    else:
                        end = domain.residue_range[1]+domain.pdb_offset
                    domain_res = mol.residue_range(start,end)
                if domain.pdb_file=="BEADS":
                    mol.add_representation(domain_res,
                                           resolutions=[domain.bead_size],
                                           setup_particles_as_densities=(domain.em_residues_per_gaussian!=0))
                    these_domains[domain.domain_name] = (set(),domain_res)
                elif domain.pdb_file=="IDEAL_HELIX":
                    mol.add_representation(domain_res,
                                           resolutions=reader.resolutions,
                                           ideal_helix=True,
                                           density_residues_per_component=domain.em_residues_per_gaussian,
                                           density_prefix=domain.density_prefix,
                                           density_force_compute=self.force_create_gmm_files)
                    these_domains[domain.domain_name] = (domain_res,set())
                else:
                    domain_atomic = mol.add_structure(domain.pdb_file,
                                                      domain.chain,
                                                      domain.residue_range,
                                                      domain.pdb_offset,
                                                      soft_check=True)
                    domain_non_atomic = domain_res - domain_atomic
                    if domain.em_residues_per_gaussian==0:
                        mol.add_representation(domain_atomic,
                                               resolutions=reader.resolutions)
                        if len(domain_non_atomic)>0:
                            mol.add_representation(domain_non_atomic,
                                                   resolutions=[domain.bead_size])
                    else:
                        mol.add_representation(domain_atomic,
                                               resolutions=reader.resolutions,
                                               density_residues_per_component=domain.em_residues_per_gaussian,
                                               density_prefix=domain.density_prefix,
                                               density_force_compute=self.force_create_gmm_files)
                        if len(domain_non_atomic)>0:
                            mol.add_representation(domain_non_atomic,
                                                   resolutions=[domain.bead_size],
                                                   setup_particles_as_densities=True)
                    these_domains[domain.domain_name] = (domain_atomic,domain_non_atomic)
            self._domains.append(these_domains)
        print('State',len(self.system.states),'added')

    def get_molecules(self):
        """Return list of all molecules grouped by state.
        For each state, it's a dictionary of Molecules where key is the molecule name
        """
        return [s.get_molecules() for s in self.system.get_states()]

    def execute_macro(self):
        """Builds representations and sets up degrees of freedom"""
        print("BuildSystem: building representations")
        self.root_hier = self.system.build()

        print("BuildSystem: setting up degrees of freedom")
        self.dof = IMP.pmi.dof.DegreesOfFreedom(self.mdl)
        for nstate,reader in enumerate(self._readers):
            rbs = reader.get_rigid_bodies()
            srbs = reader.get_super_rigid_bodies()
            csrbs = reader.get_chains_of_super_rigid_bodies()

            # add rigid bodies
            domains_in_rbs = set()
            for rblist in rbs:
                all_res = IMP.pmi.tools.OrderedSet()
                bead_res = IMP.pmi.tools.OrderedSet()
                for domain in rblist:
                    all_res|=self._domains[nstate][domain][0]
                    bead_res|=self._domains[nstate][domain][1]
                    domains_in_rbs.add(domain)
                all_res|=bead_res
                self.dof.create_rigid_body(all_res,
                                           nonrigid_parts=bead_res)

            # if you have any BEAD domains not in an RB, set them as flexible beads
            for molname in reader.get_unique_molecules():
                mlist = reader.get_unique_molecules()[molname]
                for domain in mlist:
                    if domain.pdb_file=="BEADS" and domain not in domains_in_rbs:
                        self.dof.create_flexible_beads(self._domains[nstate][domain.domain_name][1])

            # add super rigid bodies
            for srblist in srbs:
                all_res = IMP.pmi.tools.OrderedSet()
                for domain in srblist:
                    all_res|=self._domains[nstate][domain][0]
                self.dof.create_super_rigid_body(all_res)

            # add chains of super rigid bodies
            for csrblist in csrbs:
                all_res = IMP.pmi.tools.OrderedSet()
                for domain in csrblist:
                    all_res|=self._domains[nstate][domain][0]
                all_res = list(all_res)
                all_res.sort(key=lambda r:r.get_index())
                self.dof.create_super_rigid_body(all_res,chain_min_length=2,chain_max_length=3)
        return self.root_hier,self.dof

@IMP.deprecated_object("2.5", "Use BuildSystem instead")
class BuildModel(object):
    """A macro to build a Representation based on a Topology and lists of movers
    DEPRECATED - Use BuildSystem instead.
    """
    def __init__(self,
                 model,
                 component_topologies,
                 list_of_rigid_bodies=[],
                 list_of_super_rigid_bodies=[],
                 chain_of_super_rigid_bodies=[],
                 sequence_connectivity_scale=4.0,
                 add_each_domain_as_rigid_body=False,
                 force_create_gmm_files=False):
        """Constructor.
           @param model The IMP model
           @param component_topologies List of
                  IMP.pmi.topology.ComponentTopology items
           @param list_of_rigid_bodies List of lists of domain names that will
                  be moved as rigid bodies.
           @param list_of_super_rigid_bodies List of lists of domain names
                  that will move together in an additional Monte Carlo move.
           @param chain_of_super_rigid_bodies List of lists of domain names
                  (choices can only be from the same molecule). Each of these
                  groups will be moved rigidly. This helps to sample more
                  efficiently complex topologies, made of several rigid bodies,
                  connected by flexible linkers.
           @param sequence_connectivity_scale For scaling the connectivity
                  restraint
           @param add_each_domain_as_rigid_body That way you don't have to
                  put all of them in the list
           @param force_create_gmm_files If True, will sample and create GMMs
                  no matter what. If False, will only only sample if the
                  files don't exist. If number of Gaussians is zero, won't
                  do anything.
        """
        self.m = model
        self.simo = IMP.pmi.representation.Representation(self.m,
                                                          upperharmonic=True,
                                                          disorderedlength=False)

        data=component_topologies
        if list_of_rigid_bodies==[]:
            print("WARNING: No list of rigid bodies inputted to build_model()")
        if list_of_super_rigid_bodies==[]:
            print("WARNING: No list of super rigid bodies inputted to build_model()")
        if chain_of_super_rigid_bodies==[]:
            print("WARNING: No chain of super rigid bodies inputted to build_model()")
        all_dnames = set([d for sublist in list_of_rigid_bodies+list_of_super_rigid_bodies\
                      +chain_of_super_rigid_bodies for d in sublist])
        all_available = set([c.domain_name for c in component_topologies])
        if not all_dnames <= all_available:
            raise ValueError("All requested movers must reference domain "
                             "names in the component topologies")

        self.domain_dict={}
        self.resdensities={}
        super_rigid_bodies={}
        chain_super_rigid_bodies={}
        rigid_bodies={}

        for c in data:
            comp_name         = c.name
            hier_name         = c.domain_name
            color             = c.color
            fasta_file        = c.fasta_file
            fasta_id          = c.fasta_id
            pdb_name          = c.pdb_file
            chain_id          = c.chain
            res_range         = c.residue_range
            offset            = c.pdb_offset
            bead_size         = c.bead_size
            em_num_components = c.em_residues_per_gaussian
            em_txt_file_name  = c.gmm_file
            em_mrc_file_name  = c.mrc_file

            if comp_name not in self.simo.get_component_names():
                self.simo.create_component(comp_name,color=0.0)
                self.simo.add_component_sequence(comp_name,fasta_file,fasta_id)

            # create hierarchy (adds resolutions, beads) with autobuild and optionally add EM data
            if em_num_components==0:
                read_em_files=False
                include_res0=False
            else:
                if (not os.path.isfile(em_txt_file_name)) or force_create_gmm_files:
                    read_em_files=False
                    include_res0=True
                else:
                    read_em_files=True
                    include_res0=False

            outhier=self.autobuild(self.simo,comp_name,pdb_name,chain_id,
                                   res_range,include_res0,beadsize=bead_size,
                                   color=color,offset=offset)
            if em_num_components!=0:
                if read_em_files:
                    print("will read GMM files")
                else:
                    print("will calculate GMMs")

                dens_hier,beads=self.create_density(self.simo,comp_name,outhier,em_txt_file_name,
                                                    em_mrc_file_name,em_num_components,read_em_files)
                self.simo.add_all_atom_densities(comp_name, particles=beads)
                dens_hier+=beads
            else:
                dens_hier=[]

            self.resdensities[hier_name]=dens_hier
            self.domain_dict[hier_name]=outhier+dens_hier

        # setup basic restraints
        for c in self.simo.get_component_names():
            self.simo.setup_component_sequence_connectivity(c,scale=sequence_connectivity_scale)
            self.simo.setup_component_geometry(c)

        # create movers
        for rblist in list_of_rigid_bodies:
            rb=[]
            for rbmember in rblist:
                rb+=[h for h in self.domain_dict[rbmember]]
            self.simo.set_rigid_body_from_hierarchies(rb)
        for srblist in list_of_super_rigid_bodies:
            srb=[]
            for srbmember in rblist:
                srb+=[h for h in self.domain_dict[srbmember]]
            self.simo.set_super_rigid_body_from_hierarchies(srb)
        for clist in chain_of_super_rigid_bodies:
            crb=[]
            for crbmember in rblist:
                crb+=[h for h in self.domain_dict[crbmember]]
            self.simo.set_chain_of_super_rigid_bodies(crb,2,3)

        self.simo.set_floppy_bodies()
        self.simo.setup_bonds()

    def get_representation(self):
        '''Return the Representation object'''
        return self.simo

    def get_density_hierarchies(self,hier_name_list):
        # return a list of density hierarchies
        # specify the list of hierarchy names
        dens_hier_list=[]
        for hn in hier_name_list:
            print(hn)
            dens_hier_list+=self.resdensities[hn]
        return dens_hier_list

    def set_gmm_models_directory(self,directory_name):
        self.gmm_models_directory=directory_name

    def get_pdb_bead_bits(self,hierarchy):
        pdbbits=[]
        beadbits=[]
        helixbits=[]
        for h in hierarchy:
            if "_pdb" in h.get_name():pdbbits.append(h)
            if "_bead" in h.get_name():beadbits.append(h)
            if "_helix" in h.get_name():helixbits.append(h)
        return (pdbbits,beadbits,helixbits)

    def scale_bead_radii(self,nresidues,scale):
        scaled_beads=set()
        for h in self.domain_dict:
            (pdbbits,beadbits,helixbits)=self.get_pdb_bead_bits(self.domain_dict[h])
            slope=(1.0-scale)/(1.0-float(nresidues))

            for b in beadbits:
                # I have to do the following
                # because otherwise we'll scale more than once
                if b not in scaled_beads:
                    scaled_beads.add(b)
                else:
                    continue
                radius=IMP.core.XYZR(b).get_radius()
                num_residues=len(IMP.pmi.tools.get_residue_indexes(b))
                scale_factor=slope*float(num_residues)+1.0
                print(scale_factor)
                new_radius=scale_factor*radius
                IMP.core.XYZR(b).set_radius(new_radius)
                print(b.get_name())
                print("particle with radius "+str(radius)+" and "+str(num_residues)+" residues scaled to a new radius "+str(new_radius))


    def create_density(self,simo,compname,comphier,txtfilename,mrcfilename,num_components,read=True):
        #density generation for the EM restraint
        (pdbbits,beadbits,helixbits)=self.get_pdb_bead_bits(comphier)
        #get the number of residues from the pdb bits
        res_ind=[]
        for pb in pdbbits+helixbits:
            for p in IMP.core.get_leaves(pb):
                res_ind+=IMP.pmi.tools.get_residue_indexes(p)

        number_of_residues=len(set(res_ind))
        outhier=[]
        if read:
            if len(pdbbits)!=0:
                outhier+=simo.add_component_density(compname,
                                         pdbbits,
                                         num_components=num_components,
                                         resolution=0,
                                         inputfile=txtfilename)
            if len(helixbits)!=0:
                outhier+=simo.add_component_density(compname,
                                         helixbits,
                                         num_components=num_components,
                                         resolution=1,
                                         inputfile=txtfilename)


        else:
            if len(pdbbits)!=0:
                num_components=number_of_residues//abs(num_components)+1
                outhier+=simo.add_component_density(compname,
                                         pdbbits,
                                         num_components=num_components,
                                         resolution=0,
                                         outputfile=txtfilename,
                                         outputmap=mrcfilename,
                                         multiply_by_total_mass=True)

            if len(helixbits)!=0:
                num_components=number_of_residues//abs(num_components)+1
                outhier+=simo.add_component_density(compname,
                                         helixbits,
                                         num_components=num_components,
                                         resolution=1,
                                         outputfile=txtfilename,
                                         outputmap=mrcfilename,
                                         multiply_by_total_mass=True)

        return outhier,beadbits

    def autobuild(self,simo,comname,pdbname,chain,resrange,include_res0=False,
                  beadsize=5,color=0.0,offset=0):
        if pdbname is not None and pdbname is not "IDEAL_HELIX" and pdbname is not "BEADS" :
            if include_res0:
                outhier=simo.autobuild_model(comname,
                                 pdbname=pdbname,
                                 chain=chain,
                                 resrange=resrange,
                                 resolutions=[0,1,10],
                                 offset=offset,
                                 color=color,
                                 missingbeadsize=beadsize)
            else:
                outhier=simo.autobuild_model(comname,
                                 pdbname=pdbname,
                                 chain=chain,
                                 resrange=resrange,
                                 resolutions=[1,10],
                                 offset=offset,
                                 color=color,
                                 missingbeadsize=beadsize)


        elif pdbname is not None and pdbname is "IDEAL_HELIX" and pdbname is not "BEADS" :
            outhier=simo.add_component_ideal_helix(comname,
                                                resolutions=[1,10],
                                                resrange=resrange,
                                                color=color,
                                                show=False)

        elif pdbname is not None and pdbname is not "IDEAL_HELIX" and pdbname is "BEADS" :
            outhier=simo.add_component_necklace(comname,resrange[0],resrange[1],beadsize,color=color)

        else:

            seq_len=len(simo.sequence_dict[comname])
            outhier=simo.add_component_necklace(comname,
                                  begin=1,
                                  end=seq_len,
                                  length=beadsize)

        return outhier


@IMP.deprecated_object("2.5", "Use BuildSystem instead")
class BuildModel1(object):
    """Deprecated building macro - use BuildSystem()"""

    def __init__(self, representation):
        """Constructor.
           @param representation The PMI representation
        """
        self.simo=representation
        self.gmm_models_directory="."
        self.rmf_file={}
        self.rmf_frame_number={}
        self.rmf_names_map={}

    def set_gmm_models_directory(self,directory_name):
        self.gmm_models_directory=directory_name

    def build_model(self,data_structure,sequence_connectivity_scale=4.0,
                         sequence_connectivity_resolution=10,rmf_file=None,rmf_frame_number=0,rmf_file_map=None):
        """Create model.
        @param data_structure List of lists containing these entries:
             comp_name, hier_name, color, fasta_file, fasta_id, pdb_name, chain_id,
             res_range, read_em_files, bead_size, rb, super_rb,
             em_num_components, em_txt_file_name, em_mrc_file_name
        @param sequence_connectivity_scale
        @param rmf_file
        @param rmf_frame_number
        @param rmf_file_map : a dictionary that map key=component_name:value=(rmf_file_name,
                                 rmf_frame_number,
                                 rmf_component_name)
        """
        self.domain_dict={}
        self.resdensities={}
        super_rigid_bodies={}
        chain_super_rigid_bodies={}
        rigid_bodies={}

        for d in data_structure:
            comp_name         = d[0]
            hier_name         = d[1]
            color             = d[2]
            fasta_file        = d[3]
            fasta_id          = d[4]
            pdb_name          = d[5]
            chain_id          = d[6]
            res_range         = d[7][0:2]
            try:
                offset         = d[7][2]
            except:
                offset         = 0
            read_em_files     = d[8]
            bead_size         = d[9]
            rb                = d[10]
            super_rb          = d[11]
            em_num_components = d[12]
            em_txt_file_name  = d[13]
            em_mrc_file_name  = d[14]
            chain_of_super_rb = d[15]

            if comp_name not in self.simo.get_component_names():
                self.simo.create_component(comp_name,color=0.0)
                self.simo.add_component_sequence(comp_name,fasta_file,fasta_id)
            outhier=self.autobuild(self.simo,comp_name,pdb_name,chain_id,res_range,read=read_em_files,beadsize=bead_size,color=color,offset=offset)


            if not read_em_files is None:
                if em_txt_file_name is " ": em_txt_file_name=self.gmm_models_directory+"/"+hier_name+".txt"
                if em_mrc_file_name is " ": em_mrc_file_name=self.gmm_models_directory+"/"+hier_name+".mrc"


                dens_hier,beads=self.create_density(self.simo,comp_name,outhier,em_txt_file_name,em_mrc_file_name,em_num_components,read_em_files)

                self.simo.add_all_atom_densities(comp_name, particles=beads)
                dens_hier+=beads

            else:
                dens_hier=[]

            self.resdensities[hier_name]=dens_hier
            self.domain_dict[hier_name]=outhier+dens_hier

            if rb is not None:
                if rb not in rigid_bodies:
                    rigid_bodies[rb]=[h for h in self.domain_dict[hier_name]]
                else:
                    rigid_bodies[rb]+=[h for h in self.domain_dict[hier_name]]


            if super_rb is not None:
                for k in super_rb:
                    if k not in super_rigid_bodies:
                        super_rigid_bodies[k]=[h for h in self.domain_dict[hier_name]]
                    else:
                        super_rigid_bodies[k]+=[h for h in self.domain_dict[hier_name]]

            if chain_of_super_rb is not None:
                for k in chain_of_super_rb:
                    if k not in chain_super_rigid_bodies:
                        chain_super_rigid_bodies[k]=[h for h in self.domain_dict[hier_name]]
                    else:
                        chain_super_rigid_bodies[k]+=[h for h in self.domain_dict[hier_name]]



        self.rigid_bodies=rigid_bodies

        for c in self.simo.get_component_names():
            if rmf_file is not None:
                rf=rmf_file
                rfn=rmf_frame_number
                self.simo.set_coordinates_from_rmf(c, rf,rfn)
            elif rmf_file_map:
                for k in rmf_file_map:
                    cname=k
                    rf=rmf_file_map[k][0]
                    rfn=rmf_file_map[k][1]
                    rcname=rmf_file_map[k][2]
                    self.simo.set_coordinates_from_rmf(cname, rf,rfn,rcname)
            else:
                if c in self.rmf_file:
                    rf=self.rmf_file[c]
                    rfn=self.rmf_frame_number[c]
                    rfm=self.rmf_names_map[c]
                    self.simo.set_coordinates_from_rmf(c, rf,rfn,representation_name_to_rmf_name_map=rfm)
            self.simo.setup_component_sequence_connectivity(c,resolution=sequence_connectivity_resolution,scale=sequence_connectivity_scale)
            self.simo.setup_component_geometry(c)

        for rb in rigid_bodies:
            self.simo.set_rigid_body_from_hierarchies(rigid_bodies[rb])

        for k in super_rigid_bodies:
            self.simo.set_super_rigid_body_from_hierarchies(super_rigid_bodies[k])

        for k in chain_super_rigid_bodies:
            self.simo.set_chain_of_super_rigid_bodies(chain_super_rigid_bodies[k],2,3)

        self.simo.set_floppy_bodies()
        self.simo.setup_bonds()



    def set_rmf_file(self,component_name,rmf_file,rmf_frame_number,rmf_names_map=None):
        self.rmf_file[component_name]=rmf_file
        self.rmf_frame_number[component_name]=rmf_frame_number
        self.rmf_names_map[component_name]=rmf_names_map

    def get_density_hierarchies(self,hier_name_list):
        # return a list of density hierarchies
        # specify the list of hierarchy names
        dens_hier_list=[]
        for hn in hier_name_list:
            print(hn)
            dens_hier_list+=self.resdensities[hn]
        return dens_hier_list

    def get_pdb_bead_bits(self,hierarchy):
        pdbbits=[]
        beadbits=[]
        helixbits=[]
        for h in hierarchy:
            if "_pdb" in h.get_name():pdbbits.append(h)
            if "_bead" in h.get_name():beadbits.append(h)
            if "_helix" in h.get_name():helixbits.append(h)
        return (pdbbits,beadbits,helixbits)

    def scale_bead_radii(self,nresidues,scale):
        scaled_beads=set()
        for h in self.domain_dict:
            (pdbbits,beadbits,helixbits)=self.get_pdb_bead_bits(self.domain_dict[h])
            slope=(1.0-scale)/(1.0-float(nresidues))

            for b in beadbits:
                # I have to do the following
                # because otherwise we'll scale more than once
                if b not in scaled_beads:
                    scaled_beads.add(b)
                else:
                    continue
                radius=IMP.core.XYZR(b).get_radius()
                num_residues=len(IMP.pmi.tools.get_residue_indexes(b))
                scale_factor=slope*float(num_residues)+1.0
                print(scale_factor)
                new_radius=scale_factor*radius
                IMP.core.XYZR(b).set_radius(new_radius)
                print(b.get_name())
                print("particle with radius "+str(radius)+" and "+str(num_residues)+" residues scaled to a new radius "+str(new_radius))


    def create_density(self,simo,compname,comphier,txtfilename,mrcfilename,num_components,read=True):
        #density generation for the EM restraint
        (pdbbits,beadbits,helixbits)=self.get_pdb_bead_bits(comphier)

        #get the number of residues from the pdb bits
        res_ind=[]
        for pb in pdbbits+helixbits:
            for p in IMP.core.get_leaves(pb):
                res_ind+=IMP.pmi.tools.get_residue_indexes(p)

        number_of_residues=len(set(res_ind))
        outhier=[]
        if read:
            if len(pdbbits)!=0:
                outhier+=simo.add_component_density(compname,
                                         pdbbits,
                                         num_components=num_components, # number of gaussian into which the simulated density is approximated
                                         resolution=0,      # resolution that you want to calculate the simulated density
                                         inputfile=txtfilename) # read what it was calculated before
            if len(helixbits)!=0:
                outhier+=simo.add_component_density(compname,
                                         helixbits,
                                         num_components=num_components, # number of gaussian into which the simulated density is approximated
                                         resolution=1,      # resolution that you want to calculate the simulated density
                                         inputfile=txtfilename) # read what it was calculated before


        else:
            if len(pdbbits)!=0:
                if num_components<0:
                    #if negative calculate the number of gmm components automatically
                    # from the number of residues
                    num_components=number_of_residues/abs(num_components)
                outhier+=simo.add_component_density(compname,
                                         pdbbits,
                                         num_components=num_components, # number of gaussian into which the simulated density is approximated
                                         resolution=0,      # resolution that you want to calculate the simulated density
                                         outputfile=txtfilename, # do the calculation
                                         outputmap=mrcfilename,
                                         multiply_by_total_mass=True) # do the calculation and output the mrc

            if len(helixbits)!=0:
                if num_components<0:
                    #if negative calculate the number of gmm components automatically
                    # from the number of residues
                    num_components=number_of_residues/abs(num_components)
                outhier+=simo.add_component_density(compname,
                                         helixbits,
                                         num_components=num_components, # number of gaussian into which the simulated density is approximated
                                         resolution=1,      # resolution that you want to calculate the simulated density
                                         outputfile=txtfilename, # do the calculation
                                         outputmap=mrcfilename,
                                         multiply_by_total_mass=True) # do the calculation and output the mrc

        return outhier,beadbits

    def autobuild(self,simo,comname,pdbname,chain,resrange,read=True,beadsize=5,color=0.0,offset=0):

        if pdbname is not None and pdbname is not "IDEAL_HELIX" and pdbname is not "BEADS" and pdbname is not "DENSITY" :
            if resrange[-1]==-1: resrange=(resrange[0],len(simo.sequence_dict[comname]))
            if read==False:
                outhier=simo.autobuild_model(comname,
                                 pdbname=pdbname,
                                 chain=chain,
                                 resrange=resrange,
                                 resolutions=[0,1,10],
                                 offset=offset,
                                 color=color,
                                 missingbeadsize=beadsize)
            else:
                outhier=simo.autobuild_model(comname,
                                 pdbname=pdbname,
                                 chain=chain,
                                 resrange=resrange,
                                 resolutions=[1,10],
                                 offset=offset,
                                 color=color,
                                 missingbeadsize=beadsize)

        elif pdbname is not None and pdbname is "IDEAL_HELIX" and pdbname is not "BEADS" and pdbname is not "DENSITY" :
            outhier=simo.add_component_ideal_helix(comname,
                                                resolutions=[1,10],
                                                resrange=resrange,
                                                color=color,
                                                show=False)

        elif pdbname is not None and pdbname is not "IDEAL_HELIX" and pdbname is "BEADS" and pdbname is not "DENSITY" :
            seq_len=resrange[1]
            if resrange[1]==-1:
                seq_len=len(simo.sequence_dict[comname])
            outhier=simo.add_component_necklace(comname,resrange[0],seq_len,beadsize,color=color)

        elif pdbname is not None and pdbname is not "IDEAL_HELIX" and pdbname is not "BEADS" and pdbname is "DENSITY" :
            outhier=[]

        else:

            seq_len=len(simo.sequence_dict[comname])
            outhier=simo.add_component_necklace(comname,
                                  begin=1,
                                  end=seq_len,
                                  length=beadsize)

        return outhier

    def set_coordinates(self,hier_name,xyz_tuple):
        hier=self.domain_dict[hier_name]
        for h in IMP.atom.get_leaves(hier):
            p=h.get_particle()
            if IMP.core.NonRigidMember.get_is_setup(p):
                pass
            else:
                IMP.core.XYZ(p).set_coordinates(xyz_tuple)

    def save_rmf(self,rmfname):

        o=IMP.pmi.output.Output()
        o.init_rmf(rmfname,[self.simo.prot])
        o.write_rmf(rmfname)
        o.close_rmf(rmfname)
# -----------------------------------------------------------------------

@IMP.deprecated_object("2.5", "Use BuildSystem instead")
def BuildModel0(
    m,
    data,
    resolutions=[1,
                 10],
    missing_bead_size=20,
        residue_per_gaussian=None):
    '''
    Construct a component for each subunit (no splitting, nothing fancy).

    You can pass the resolutions and the bead size for the missing residue regions.
    To use this macro, you must provide the following data structure:

    Component  pdbfile    chainid  rgb color     fastafile     sequence id
                                                                      in fastafile

data = [("Rpb1",     pdbfile,   "A",     0.00000000,  (fastafile,    0)),
      ("Rpb2",     pdbfile,   "B",     0.09090909,  (fastafile,    1)),
      ("Rpb3",     pdbfile,   "C",     0.18181818,  (fastafile,    2)),
      ("Rpb4",     pdbfile,   "D",     0.27272727,  (fastafile,    3)),
      ("Rpb5",     pdbfile,   "E",     0.36363636,  (fastafile,    4)),
      ("Rpb6",     pdbfile,   "F",     0.45454545,  (fastafile,    5)),
      ("Rpb7",     pdbfile,   "G",     0.54545455,  (fastafile,    6)),
      ("Rpb8",     pdbfile,   "H",     0.63636364,  (fastafile,    7)),
      ("Rpb9",     pdbfile,   "I",     0.72727273,  (fastafile,    8)),
      ("Rpb10",    pdbfile,   "L",     0.81818182,  (fastafile,    9)),
      ("Rpb11",    pdbfile,   "J",     0.90909091,  (fastafile,   10)),
      ("Rpb12",    pdbfile,   "K",     1.00000000,  (fastafile,   11))]

    '''

    r = IMP.pmi.representation.Representation(m)

    # the dictionary for the hierarchies,
    hierarchies = {}

    for d in data:
        # retrieve the information from the data structure
        component_name = d[0]
        pdb_file = d[1]
        chain_id = d[2]
        color_id = d[3]
        fasta_file = d[4][0]
        # this function
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fasta_file)
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
                                        resolutions=resolutions,
                                        missingbeadsize=missing_bead_size)

        r.show_component_table(component_name)

        r.set_rigid_bodies([component_name])

        r.set_chain_of_super_rigid_bodies(
            hierarchies,
            min_length=2,
            max_length=2)

        r.setup_component_sequence_connectivity(component_name, resolution=1)
        r.setup_component_geometry(component_name)

    r.setup_bonds()
    # put it at the end of rigid bodies
    r.set_floppy_bodies()

    # set current coordinates as reference for RMSD calculation
    r.set_current_coordinates_as_reference_for_rmsd("Reference")

    return r

# ----------------------------------------------------------------------

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
                 global_analysis_result_directory="./analysis/"):
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
        """

        try:
            from mpi4py import MPI
            self.comm = MPI.COMM_WORLD
            self.rank = self.comm.Get_rank()
            self.number_of_processes = self.comm.size
        except ImportError:
            self.rank = 0
            self.number_of_processes = 1

        self.model = model
        stat_dir = global_output_directory
        self.stat_files = []
        # it contains the position of the root directories
        for rd in merge_directories:
            stat_files = glob.glob(os.path.join(rd,stat_dir,"stat.*.out"))
            if len(stat_files)==0:
                print("WARNING: no stat files found in",os.path.join(rd,stat_dir))
            self.stat_files += stat_files


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
        """ Get the best scoring models, compute a distance matrix, cluster them, and create density maps
        @param score_key                      The score for ranking models
        @param rmf_file_key                   Key pointing to RMF filename
        @param rmf_file_frame_key             Key pointing to RMF frame number
        @param state_number                   State number to analyze
        @param prefiltervalue                 Only include frames where the
                                               score key is below this value
        @param feature_keys                   Keywords for which you want to
                                               calculate average, medians, etc,
        @param outputdir                      The local output directory used in the run
        @param alignment_components           Dictionary with keys=groupname,
                                               values are list of tuples for aligning
                                               the structures
                                               e.g. {"Rpb1": (20,100,"Rpb1"),"Rpb2":"Rpb2"}
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
        if self.rank==0:
            try:
                os.mkdir(outputdir)
            except:
                pass

        if not load_distance_matrix_file:
            if len(self.stat_files)==0: print("ERROR: no stat file found in the given path"); return
            my_stat_files=IMP.pmi.tools.chunk_list_into_segments(
                self.stat_files,self.number_of_processes)[self.rank]
            best_models = IMP.pmi.io.get_best_models(my_stat_files,
                                                     score_key,
                                                     feature_keys,
                                                     rmf_file_key,
                                                     rmf_file_frame_key,
                                                     prefiltervalue,
                                                     get_every)
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
                    o.init_rmf(out_rmf_fn,[prot],rs)
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
            Clusters = IMP.pmi.analysis.Clustering(rmsd_weights)

            for n, model_coordinate_dict in enumerate(all_coordinates):
                template_coordinate_dict = {}
                # let's try to align
                if alignment_components is not None and len(Clusters.all_coords) == 0:
                    # set the first model as template coordinates
                    Clusters.set_template(alignment_coordinates[n])
                Clusters.fill(all_rmf_file_names[n], rmsd_coordinates[n])
            print("Global calculating the distance matrix")

            # calculate distance matrix, all against all
            Clusters.dist_matrix()

            # perform clustering and optionally display
            if self.rank == 0:
                Clusters.do_cluster(number_of_clusters)
                if display_plot:
                    if self.rank == 0:
                        Clusters.plot_matrix(figurename=os.path.join(outputdir,'dist_matrix.pdf'))
                    if exit_after_display:
                        exit()
                Clusters.save_distance_matrix_file(file_name=distance_matrix_file)

# ------------------------------------------------------------------------
# Alteratively, load the distance matrix from file and cluster that
# ------------------------------------------------------------------------
        else:
            if self.rank==0:
                print("setup clustering class")
                Clusters = IMP.pmi.analysis.Clustering()
                Clusters.load_distance_matrix_file(file_name=distance_matrix_file)
                print("clustering with %s clusters" % str(number_of_clusters))
                Clusters.do_cluster(number_of_clusters)
                [best_score_feature_keyword_list_dict,
                 rmf_file_name_index_dict] = self.load_objects(".macro.pkl")
                if display_plot:
                    if self.rank == 0:
                        Clusters.plot_matrix(figurename=os.path.join(outputdir,'dist_matrix.pdf'))
                    if exit_after_display:
                        exit()
        if self.number_of_processes > 1:
            self.comm.Barrier()

# ------------------------------------------------------------------------
# now save all informations about the clusters
# ------------------------------------------------------------------------

        if self.rank == 0:
            print(Clusters.get_cluster_labels())
            for n, cl in enumerate(Clusters.get_cluster_labels()):
                print("rank %s " % str(self.rank))
                print("cluster %s " % str(n))
                print("cluster label %s " % str(cl))
                print(Clusters.get_cluster_label_names(cl))

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
                             str(Clusters.get_cluster_label_average_rmsd(cl))}
                clusstat = open(dircluster + "stat.out", "w")
                for k, structure_name in enumerate(Clusters.get_cluster_label_names(cl)):
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

                    # transform clusters onto first
                    if k > 0:
                        model_index = Clusters.get_model_index_from_name(
                            structure_name)
                        transformation = Clusters.get_transformation_to_first_member(
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
