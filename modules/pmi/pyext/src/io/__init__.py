"""@namespace IMP.pmi.io
   Utility classes and functions for reading and storing PMI files
"""

from __future__ import print_function
import IMP
import IMP.algebra
import IMP.atom
import IMP.pmi
import IMP.rmf
import IMP.pmi.analysis
import IMP.pmi.output
import IMP.pmi.tools
import RMF
import sys,os
import numpy as np
import re
from collections import defaultdict
import itertools

def parse_dssp(dssp_fn, limit_to_chains='',name_map=None):
    """read dssp file, get SSEs. values are all PDB residue numbering.
    @param dssp_fn The file to read
    @param limit_to_chains Only read/return these chain IDs
    @param name_map If passed, return tuples organized by molecule name

    Returns a dictionary with keys helix, beta, loop
    Each contains a list of SSEs.
    Each SSE is a list of elements (e.g. strands in a sheet)
    Each element is a pair (chain,(residue_indexes))

    Example for a structure with helix A:5-7 and Beta strands A:1-3,A:9-11:
    ret = { 'helix' : [ [ ('A',5,7 ) ],... ]
            'beta'  : [ [ ('A',1,3),
                          ('A',9,11),...],...]
            'loop'  : same format as helix
          }
    """

    def convert_chain(ch):
        if name_map is None:
            return ch
        else:
            return name_map[ch]

    # setup
    helix_classes = 'GHI'
    strand_classes = 'EB'
    loop_classes = [' ', '', 'T', 'S']
    sse_dict = {}
    for h in helix_classes:
        sse_dict[h] = 'helix'
    for s in strand_classes:
        sse_dict[s] = 'beta'
    for l in loop_classes:
        sse_dict[l] = 'loop'
    sses = {'helix':[],
            'beta':[],
            'loop':[]}

    # read file and parse
    start = False

    # temporary beta dictionary indexed by DSSP's ID
    beta_dict = defaultdict(list)
    prev_sstype = None
    prev_beta_id = None
    for line in open(dssp_fn, 'r'):
        fields = line.split()
        chain_break = False
        if len(fields) < 2:
            continue
        if fields[1] == "RESIDUE":
            # Start parsing from here
            start = True
            continue
        if not start:
            continue
        if line[9] == " ":
            chain_break = True
        elif limit_to_chains != '' and line[11] not in limit_to_chains:
            continue

        # gather line info
        if not chain_break:
            pdb_res_num = int(line[5:10])
            chain = line[11]
            sstype = sse_dict[line[16]]
            beta_id = line[33]

        # decide whether to extend or store the SSE
        if prev_sstype is None:
            cur_sse = [pdb_res_num,pdb_res_num,convert_chain(chain)]
        elif sstype != prev_sstype or chain_break:
            # add cur_sse to the right place
            if prev_sstype in ['helix', 'loop']:
                sses[prev_sstype].append([cur_sse])
            elif prev_sstype == 'beta':
                beta_dict[prev_beta_id].append(cur_sse)
            cur_sse = [pdb_res_num,pdb_res_num,convert_chain(chain)]
        else:
            cur_sse[1] = pdb_res_num
        if chain_break:
            prev_sstype = None
            prev_beta_id = None
        else:
            prev_sstype = sstype
            prev_beta_id = beta_id

    # final SSE processing
    if not prev_sstype is None:
        if prev_sstype in ['helix', 'loop']:
            sses[prev_sstype].append([cur_sse])
        elif prev_sstype == 'beta':
            beta_dict[prev_beta_id].append(cur_sse)
    # gather betas
    for beta_sheet in beta_dict:
        sses['beta'].append(beta_dict[beta_sheet])
    return sses

def save_best_models(mdl,
                     out_dir,
                     stat_files,
                     number_of_best_scoring_models=10,
                     get_every=1,
                     score_key="SimplifiedModel_Total_Score_None",
                     feature_keys=None,
                     rmf_file_key="rmf_file",
                     rmf_file_frame_key="rmf_frame_index",
                     override_rmf_dir=None):
    """Given a list of stat files, read them all and find the best models.
    Save to a single RMF along with a stat file.
    @param mdl The IMP Model
    @param out_dir The output directory. Will save 3 files (RMF, stat, summary)
    @param stat_files List of all stat files to collect
    @param number_of_best_scoring_models Num best models to gather
    @param get_every Skip frames
    @param score_key Used for the ranking
    @param feature_keys Keys to keep around
    @param rmf_file_key The key that says RMF file name
    @param rmf_file_frame_key The key that says RMF frame number
    @param override_rmf_dir For output, change the name of the RMF directory (experiment)
    """

    # start by splitting into jobs
    try:
        from mpi4py import MPI
        comm = MPI.COMM_WORLD
        rank = comm.Get_rank()
        number_of_processes = comm.size
    except ImportError:
        rank = 0
        number_of_processes = 1
    my_stat_files=IMP.pmi.tools.chunk_list_into_segments(
        stat_files,number_of_processes)[rank]

    # filenames
    out_stat_fn = os.path.join(out_dir,"top_"+str(number_of_best_scoring_models)+".out")
    out_rmf_fn = os.path.join(out_dir,"top_"+str(number_of_best_scoring_models)+".rmf3")

    # extract all the models
    all_fields=[]
    for nsf,sf in enumerate(my_stat_files):

        # get list of keywords
        root_directory_of_stat_file = os.path.dirname(os.path.dirname(sf))
        print("getting data from file %s" % sf)
        po = IMP.pmi.output.ProcessOutput(sf)
        all_keys = [score_key,
                    rmf_file_key,
                    rmf_file_frame_key]
        for k in po.get_keys():
            for fk in feature_keys:
                if fk in k:
                    all_keys.append(k)
        fields = po.get_fields(all_keys,
                               get_every=get_every)

        # check that all lengths are all equal
        length_set = set([len(fields[f]) for f in fields])
        minlen = min(length_set)
        # if some of the fields are missing, truncate
        # the feature files to the shortest one
        if len(length_set) > 1:
            print("get_best_models: the statfile is not synchronous")
            minlen = min(length_set)
            for f in fields:
                fields[f] = fields[f][0:minlen]
        if nsf==0:
            all_fields=fields
        else:
            for k in fields:
                all_fields[k]+=fields[k]

        if override_rmf_dir is not None:
            for i in range(minlen):
                all_fields[rmf_file_key][i]=os.path.join(
                    override_rmf_dir,os.path.basename(all_fields[rmf_file_key][i]))

    # gather info, sort, write
    if number_of_processes!=1:
        comm.Barrier()
    if rank!=0:
        comm.send(all_fields, dest=0, tag=11)
    else:
        for i in range(1,number_of_processes):
            data_tmp = comm.recv(source=i, tag=11)
            for k in all_fields:
                all_fields[k]+=data_tmp[k]

        # sort by total score
        order = sorted(range(len(all_fields[score_key])),
                       key=lambda i: float(all_fields[score_key][i]))

        # write the stat and RMF files
        stat = open(out_stat_fn,'w')
        rh0 = RMF.open_rmf_file_read_only(
            os.path.join(root_directory_of_stat_file,all_fields[rmf_file_key][0]))
        prots = IMP.rmf.create_hierarchies(rh0,mdl)
        del rh0
        outf = RMF.create_rmf_file(out_rmf_fn)
        IMP.rmf.add_hierarchies(outf,prots)
        for nm,i in enumerate(order[:number_of_best_scoring_models]):
            dline=dict((k,all_fields[k][i]) for k in all_fields)
            dline['orig_rmf_file']=dline[rmf_file_key]
            dline['orig_rmf_frame_index']=dline[rmf_file_frame_key]
            dline[rmf_file_key]=out_rmf_fn
            dline[rmf_file_frame_key]=nm
            rh = RMF.open_rmf_file_read_only(
                os.path.join(root_directory_of_stat_file,all_fields[rmf_file_key][i]))
            IMP.rmf.link_hierarchies(rh,prots)
            IMP.rmf.load_frame(rh,
                               RMF.FrameID(all_fields[rmf_file_frame_key][i]))
            IMP.rmf.save_frame(outf)
            del rh
            stat.write(str(dline)+'\n')
        stat.close()
        print('wrote stats to',out_stat_fn)
        print('wrote rmfs to',out_rmf_fn)




def get_best_models(stat_files,
                    score_key="SimplifiedModel_Total_Score_None",
                    feature_keys=None,
                    rmf_file_key="rmf_file",
                    rmf_file_frame_key="rmf_frame_index",
                    prefiltervalue=None,
                    get_every=1):
    """ Given a list of stat files, read them all and find the best models.
    Returns the best rmf filenames, frame numbers, scores, and values for feature keywords
    """
    rmf_file_list=[]              # best RMF files
    rmf_file_frame_list=[]        # best RMF frames
    score_list=[]                 # best scores
    feature_keyword_list_dict=defaultdict(list)  # best values of the feature keys
    for sf in stat_files:
        root_directory_of_stat_file = os.path.dirname(os.path.dirname(os.path.abspath(sf)))
        print("getting data from file %s" % sf)
        po = IMP.pmi.output.ProcessOutput(sf)

        try:
            file_keywords = po.get_keys()
        except:
            continue

        final_keywords = [score_key,
                          rmf_file_key,
                          rmf_file_frame_key]

        for file_k in file_keywords:
            for requested_key in feature_keys:
                if requested_key in file_k:
                    final_keywords.append(k)

        if prefiltervalue is None:
            fields = po.get_fields(final_keywords,
                                   get_every=get_every)
        else:
            fields = po.get_fields(final_keywords,
                                   filtertuple=(score_key,"<",prefiltervalue),
                                   get_every=get_every)

        # check that all lengths are all equal
        length_set = set()
        for f in fields:
            length_set.add(len(fields[f]))

        # if some of the fields are missing, truncate
        # the feature files to the shortest one
        if len(length_set) > 1:
            print("get_best_models: the statfile is not synchronous")
            minlen = min(length_set)
            for f in fields:
                fields[f] = fields[f][0:minlen]

        # append to the lists
        score_list += fields[score_key]
        for rmf in fields[rmf_file_key]:
            rmf_file_list.append(os.path.join(root_directory_of_stat_file,rmf))

        rmf_file_frame_list += fields[rmf_file_frame_key]

        if final_keywords is not None:
            for k in final_keywords:
                feature_keyword_list_dict[k] += fields[k]

    return rmf_file_list,rmf_file_frame_list,score_list,feature_keyword_list_dict

def get_trajectory_models(stat_files,
                          score_key="SimplifiedModel_Total_Score_None",
                          rmf_file_key="rmf_file",
                          rmf_file_frame_key="rmf_frame_index",
                          get_every=1):
    """ Given a list of stat files, read them all and find a trajectory of models.
    Returns the rmf filenames, frame numbers, scores, and values for feature keywords
    """
    rmf_file_list=[]              # best RMF files
    rmf_file_frame_list=[]        # best RMF frames
    score_list=[]                 # best scores
    for sf in stat_files:
        root_directory_of_stat_file = os.path.dirname(os.path.dirname(sf))
        print("getting data from file %s" % sf)
        po = IMP.pmi.output.ProcessOutput(sf)
        keywords = po.get_keys()

        feature_keywords = [score_key,
                            rmf_file_key,
                            rmf_file_frame_key]

        fields = po.get_fields(feature_keywords,
                                   get_every=get_every)

        # check that all lengths are all equal
        length_set = set()
        for f in fields:
            length_set.add(len(fields[f]))

        # if some of the fields are missing, truncate
        # the feature files to the shortest one
        if len(length_set) > 1:
            print("get_best_models: the statfile is not synchronous")
            minlen = min(length_set)
            for f in fields:
                fields[f] = fields[f][0:minlen]

        # append to the lists
        score_list += fields[score_key]
        for rmf in fields[rmf_file_key]:
            rmf_file_list.append(os.path.join(root_directory_of_stat_file,rmf))

        rmf_file_frame_list += fields[rmf_file_frame_key]

    return rmf_file_list,rmf_file_frame_list,score_list


def read_coordinates_of_rmfs(model,
                             rmf_tuples,
                             alignment_components=None,
                             rmsd_calculation_components=None,
                             state_number=0):
    """ Read in coordinates of a set of RMF tuples.
    Returns the coordinates split as requested (all, alignment only, rmsd only) as well as
    RMF file names (as keys in a dictionary, with values being the rank number) and just a plain list
    @param model      The IMP model
    @param rmf_tuples [score,filename,frame number,original order number, rank]
    @param alignment_components Tuples to specify what you're aligning on
    @param rmsd_calculation_components Tuples to specify what components are used for RMSD calc
    """
    all_coordinates = []
    rmsd_coordinates = []
    alignment_coordinates = []
    all_rmf_file_names = []
    rmf_file_name_index_dict = {} # storing the features

    for cnt, tpl in enumerate(rmf_tuples):
        rmf_file = tpl[1]
        frame_number = tpl[2]
        if cnt==0:
            prots = IMP.pmi.analysis.get_hiers_from_rmf(model,
                                                        frame_number,
                                                        rmf_file)
        else:
            IMP.pmi.analysis.link_hiers_to_rmf(model,prots,frame_number,rmf_file)

        if not prots:
            continue
        if IMP.pmi.get_is_canonical(prots[0]):
            states = IMP.atom.get_by_type(prots[0],IMP.atom.STATE_TYPE)
            prot = states[state_number]
        else:
            prot = prots[state_number]

        # getting the particles
        part_dict = IMP.pmi.analysis.get_particles_at_resolution_one(prot)
        all_particles = [pp for key in part_dict for pp in part_dict[key]]
        all_ps_set = set(all_particles)
        model_coordinate_dict = {}
        template_coordinate_dict={}
        rmsd_coordinate_dict={}

        for pr in part_dict:
            model_coordinate_dict[pr] = np.array(
                [np.array(IMP.core.XYZ(i).get_coordinates()) for i in part_dict[pr]])
        # for each file, get (as floats) a list of all coordinates
        #  of all requested tuples, organized as dictionaries.
        for tuple_dict,result_dict in zip((alignment_components,rmsd_calculation_components),
                                          (template_coordinate_dict,rmsd_coordinate_dict)):

            if tuple_dict is None:
                continue

            # PMI2: do selection of resolution and name at the same time
            if IMP.pmi.get_is_canonical(prot):
                for pr in tuple_dict:
                    ps = IMP.pmi.tools.select_by_tuple_2(prot,tuple_dict[pr],resolution=1)
                    result_dict[pr] = [list(map(float,IMP.core.XYZ(p).get_coordinates()))
                                       for p in ps]
            else:
                for pr in tuple_dict:
                    if type(tuple_dict[pr]) is str:
                        name=tuple_dict[pr]
                        s=IMP.atom.Selection(prot,molecule=name)
                    elif type(tuple_dict[pr]) is tuple:
                        name=tuple_dict[pr][2]
                        rend=tuple_dict[pr][1]
                        rbegin=tuple_dict[pr][0]
                        s=IMP.atom.Selection(prot,molecule=name,residue_indexes=range(rbegin,rend+1))
                    ps=s.get_selected_particles()
                    filtered_particles=[p for p in ps if p in all_ps_set]
                    result_dict[pr] = \
                        [list(map(float,IMP.core.XYZ(p).get_coordinates())) for p in filtered_particles]

        all_coordinates.append(model_coordinate_dict)
        alignment_coordinates.append(template_coordinate_dict)
        rmsd_coordinates.append(rmsd_coordinate_dict)
        frame_name = rmf_file + '|' + str(frame_number)
        all_rmf_file_names.append(frame_name)
        rmf_file_name_index_dict[frame_name] = tpl[4]
    return all_coordinates,alignment_coordinates,rmsd_coordinates,rmf_file_name_index_dict,all_rmf_file_names

def get_bead_sizes(model,rmf_tuple,rmsd_calculation_components=None,state_number=0):
    '''
    @param model      The IMP model
    @param rmf_tuple  score,filename,frame number,original order number, rank
    @param rmsd_calculation_components Tuples to specify what components are used for RMSD calc
    '''
    if rmsd_calculation_components is None:
        return {}

    rmf_file = rmf_tuple[1]
    frame_number = rmf_tuple[2]
    prots = IMP.pmi.analysis.get_hiers_from_rmf(model,
                                                frame_number,
                                                rmf_file)

    if IMP.pmi.get_is_canonical(prots[0]):
        states = IMP.atom.get_by_type(prots[0],IMP.atom.STATE_TYPE)
        prot = states[state_number]
    else:
        prot = prots[state_number]

    rmsd_bead_size_dict = {}

    # PMI2: do selection of resolution and name at the same time
    if IMP.pmi.get_is_canonical(prot):
        for pr in rmsd_calculation_components:
            ps = IMP.pmi.tools.select_by_tuple_2(prot,rmsd_calculation_components[pr],resolution=1)
            rmsd_bead_size_dict[pr] = [len(IMP.pmi.tools.get_residue_indexes(p))
                                       for p in ps]
    else:
        # getting the particles
        part_dict = IMP.pmi.analysis.get_particles_at_resolution_one(prot)
        all_particles=[pp for key in part_dict for pp in part_dict[key]]
        all_ps_set=set(all_particles)

        # getting the coordinates
        for pr in rmsd_calculation_components:
            if type(rmsd_calculation_components[pr]) is str:
                name=rmsd_calculation_components[pr]
                s=IMP.atom.Selection(prot,molecule=name)
            elif type(rmsd_calculation_components[pr]) is tuple:
                name=rmsd_calculation_components[pr][2]
                rend=rmsd_calculation_components[pr][1]
                rbegin=rmsd_calculation_components[pr][0]
                s=IMP.atom.Selection(prot,molecule=name,residue_indexes=range(rbegin,rend+1))
            ps=s.get_selected_particles()
            filtered_particles=[p for p in ps if p in all_ps_set]
            rmsd_bead_size_dict[pr] = \
                [len(IMP.pmi.tools.get_residue_indexes(p)) for p in filtered_particles]

    return rmsd_bead_size_dict

class RMSDOutput(object):
    """A helper output based on dist to initial coordinates"""
    def __init__(self,ps,label,init_coords=None):
        self.mdl = ps[0].get_model()
        self.ps = ps
        if init_coords is None:
            self.init_coords = [IMP.core.XYZ(p).get_coordinates() for p in self.ps]
        else:
            self.init_coords = init_coords
        self.label = label
    def get_output(self):
        self.mdl.update()
        output = {}
        coords = [IMP.core.XYZ(p).get_coordinates() for p in self.ps]
        rmsd = IMP.algebra.get_rmsd(coords,self.init_coords)
        output["RMSD_"+self.label] = str(rmsd)
        return output

class TotalScoreOutput(object):
    """A helper output for model evaluation"""
    def __init__(self,mdl):
        self.mdl = mdl
        self.rs = IMP.pmi.tools.get_restraint_set(self.mdl)
    def get_output(self):
        self.mdl.update()
        score = self.rs.evaluate(False)
        output = {}
        output["Total_Score"] = str(score)
        return output
