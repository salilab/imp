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
def parse_dssp(dssp_fn, limit_to_chains=''):
    """read dssp file, get SSEs. values are all PDB residue numbering.
    Returns a SubsequenceData object containing labels helix, beta, loop.
    Each one is a list of SelectionDictionaries

    Example for a structure with helix A:5-7 and Beta strands A:1-3,A:9-11:
    helix : [ [ {'chain':'A','residue_indexes': [5,6,7]} ] ]
    beta  : [ [ {'chain':'A','residue_indexes': [1,2,3]},
                {'chain':'A','residue_indexes': [9,10,11]} ] ]
    loop  : same format as helix
    """
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
    sses = SubsequenceData()

    # read file and parse
    start = False
    # temporary beta dictionary indexed by DSSP's ID
    beta_dict = defaultdict(list)
    prev_sstype = None
    cur_sse = {'chain':'','residue_tuple':[-1,-1]}
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
            cur_sse = {'chain':chain,'residue_tuple':[pdb_res_num,pdb_res_num]}
        elif sstype != prev_sstype or chain_break:
            # add cur_sse to the right place
            if prev_sstype in ['helix', 'loop']:
                sses.add_subsequence(prev_sstype,Subsequence(**cur_sse))
            elif prev_sstype == 'beta':
                beta_dict[prev_beta_id].append(cur_sse)
            cur_sse = {'chain':chain,'residue_tuple':[pdb_res_num,pdb_res_num]}
        else:
            cur_sse['residue_tuple'][1]=pdb_res_num
        if chain_break:
            prev_sstype = None
            prev_beta_id = None
        else:
            prev_sstype = sstype
            prev_beta_id = beta_id

    # final SSE processing
    if not prev_sstype is None:
        if prev_sstype in ['helix', 'loop']:
            sses.add_subsequence(prev_sstype,Subsequence(**cur_sse))
        elif prev_sstype == 'beta':
            beta_dict[prev_beta_id].append(cur_sse)
    # gather betas
    for beta_sheet in beta_dict:
        seq = Subsequence()
        for strand in beta_dict[beta_sheet]:
            seq.add_range(**strand)
        sses.add_subsequence('beta',seq)
    return sses

def parse_xlinks_davis(data_fn,
                       max_num=-1,
                       name_map={},
                       named_offsets={},
                       use_chains={}):
    """ Format from Trisha Davis. Lines are:
    ignore ignore seq1 seq2 >Name(res) >Name(res) score
    @param data_fn       The data file name
    @param max_num       Maximum number of XL to read (-1 is all)
    @param name_map      Dictionary mapping text file names to the molecule name
    @param named_offsets Integer offsets to apply to the indexing in the file
    Output is a CrossLinkData object containing SelectionDictionaries
    data[unique_id] =
              [ { 'r1': {'molecule':'A','residue_index':5},
                  'r2': {'molecule':'B','residue_index':100},
                  'score': 123 },
                { 'r1': {'molecule':'C','residue_index':63},
                  'r2': {'molecule':'D','residue_index':94},
                  'score': 600 }
              ]
    """

    inf=open(data_fn,'r')
    data=defaultdict(list)
    found=set()
    data = CrossLinkData()
    for nl,l in enumerate(inf):
        if max_num==-1 or nl<max_num:
            ig1,ig2,seq1,seq2,s1,s2,score=l.split()
            score=float(score)
            m1=re.search(r'>([\w-]+)\((\w+)\)',s1)
            m2=re.search(r'>([\w-]+)\((\w+)\)',s2)

            n1=m1.group(1)
            r1=int(m1.group(2))
            if n1 in name_map:
                n1=name_map[n1]
            if n1 in named_offsets:
                r1+=named_offsets[n1]

            n2=m2.group(1)
            r2=int(m2.group(2))
            if n2 in name_map:
                n2=name_map[n2]
            if n2 in named_offsets:
                r2+=named_offsets[n2]
            key=tuple(sorted(['%s.%i'%(n1,r1),'%s.%i'%(n2,r2)]))
            if key in found:
                print('skipping duplicated xl',key)
                continue
            found.add(key)
            data.add_cross_link(nl,
                                {'molecule':n1,'residue_index':r1},
                                {'molecule':n2,'residue_index':r2},
                                score)
    inf.close()
    return data


class Subsequence(object):
    """ A light class to store multiple not-necessarily-contiguous residue ranges."""
    def __init__(self,chain=None,molecule=None,residue_tuple=None,subsequences=None):
        """Create subsequence and optionally pass the first contiguous range.
        @param chain The chain ID
        @param molecule The molecule name
        @param residue_tuple PDB-style inclusive residue range
        @param subsequences A list of other subsequences to combine (not implemented)
        """
        self.seqs=[]
        if chain or molecule or residue_tuple:
            self.add_range(chain,molecule,residue_tuple)
        if subsequences:
            pass
    def add_range(self,chain=None,molecule=None,residue_tuple=None):
        """Add some stuff to this subsequence
        @param chain The chain ID
        @param molecule The molecule name
        @param residue_tuple PDB-style inclusive residue range
        """
        self.seqs.append({'chain':chain,'molecule':molecule,'residue_tuple':residue_tuple})

    def join(self,new_subsequence):
        for s in new_subsequence:
            self.seqs.append(s)

    def get_selection(self,hier,**kwargs):
        """Create an IMP Selection from this subsequence
        @param hier An IMP hierarchy or list of them
        \note any additional keyword arguments will be appended to the selection
        """
        for nseq,seq in enumerate(self.seqs):
            args=kwargs
            if seq['chain']:
                args['chain']=seq['chain']
            if seq['molecule']:
                args['molecule']=seq['molecule']
            if seq['residue_tuple']:
                args['residue_indexes']=list(range(seq['residue_tuple'][0],
                                              seq['residue_tuple'][1]+1))
            sel = IMP.atom.Selection(hier,**args)
            if nseq==0:
                ret=sel
            else:
                ret|=sel
        return ret
    def __repr__(self):
        rep=''
        for nseq,seq in enumerate(self.seqs):
            this_str=[]
            if seq['residue_tuple'] is not None:
                this_str.append('%i-%i'%(seq['residue_tuple'][0],seq['residue_tuple'][1]))
            if seq['molecule'] is not None:
                this_str.append('%s'%seq['molecule'])
            if seq['chain'] is not None:
                this_str.append('%s'%seq['chain'])
            rep+='.'.join(this_str)
            if nseq < len(self.seqs)-1:
                rep+='_'
        return rep

    def __getitem__(self,key):
        return self.seqs[key]

    def __iter__(self):
        return self.seqs.__iter__()

    def __add__(self,other):
        self.join(other)
        return self

class SubsequenceData(object):
    """ Group a bunch of subsequences with certain labels
    Use cases: storing lists of secondary structures from DSSP or PSIPRED
               storing lists of molecules that should be made symmetric
    """
    def __init__(self):
        """Setup groups of subsequences
        """
        self.data=defaultdict(list)

    def add_subsequence(self,label,subsequence):
        """ Append a new cross-link to a given unique ID
        @param label         label for this subsequence (e.g., 'helix')
        @param subsequence   a Subsequence object to store under that label
                             e.g. Subsequence(chain='A',residue_tuple=(10,15))
        """
        if type(subsequence) is not Subsequence:
            raise InputError('must provide a subsequence object')
        self.data[label].append(subsequence)

    def __getitem__(self,key):
        return self.data[key]

    def __repr__(self):
        return self.data.__repr__()

    def keys(self):
        return self.data.keys()

class CrossLink(object):
    """A class to store the selection commands for a single crosslink.
    """
    def __init__(self,unique_id,r1,r2,score):
        """Add a crosslink.
        @param unique_id The id is used to group crosslinks that are alternatives
        @param r1 A dictionary of selection keywords for the first residue
        @param r2 A dictionary of selection keywards for the second residue
        @param score A score that might be used later for filtering
        @note The dictionaries can contain any Selection argument like
              molecule or residue_index
        """
        self.unique_id = unique_id
        self.r1 = r1
        self.r2 = r2
        self.score = score

    def __repr__(self):
        return "CrossLink id: "+str(self.unique_id)+" r1: "+repr(self.r1)+", r2: "+repr(self.r2)

    def intersects(self,other):
        if self.r1==other.r1 or self.r1==other.r2 or self.r2==other.r1 or self.r2==other.r2:
            return True
        else:
            return False

    def get_selection(self,mh,**kwargs):
        """Return a list of atom pairs (particles) for this crosslink.
        Found by selecting everything with r1 and r2 then returning the
         cartesian product.
        @note you may want to provide some atom specifiers like
         atom_type=IMP.atom.AtomType("CA")
        @param mh The hierarchy to select from
        @param kwargs Any additional selection arguments
        """
        rsel1=self.r1.copy()
        rsel1.update(kwargs)
        rsel2=self.r2.copy()
        rsel2.update(kwargs)
        sel1 = IMP.atom.Selection(mh,**rsel1).get_selected_particles()
        sel2 = IMP.atom.Selection(mh,**rsel2).get_selected_particles()
        if len(sel1)==0:
            raise Exception("this selection is empty",rsel1)
        if len(sel2)==0:
            raise Exception("this selection is empty",rsel2)

        '''
        # Check no repeating copy numbers....not sure if this is general
        for s in (sel1,sel2):
            idxs=[IMP.atom.get_copy_index(p) for p in s]
            if len(idxs)!=len(set(idxs)):
                raise Exception("this XL is selecting more than one particle per copy")
        '''
        ret = []
        for p1,p2 in itertools.product(sel1,sel2):
            ret.append((p1,p2))
        return ret

class CrossLinkData(object):
    """A class for storing groups of crosslinks.
    Acts like a dictionary where keys are unique IDs and values are CrossLinks
    Equivalent (using objects instead of dicts) to a data structure like so:
         data[1030] =
              [ { 'r1': {'molecule':'A','residue_index':5},
                  'r2': {'molecule':'B','residue_index':100},
                   'Score': 123 },
                { 'r1': {'molecule':'C','residue_index':63},
                  'r2': {'molecule':'D','residue_index':94},
                  'Score': 600 }
              ]
    """
    def __init__(self):
        """Setup a CrossLinkData object"""
        self.data = defaultdict(list)
    def add_cross_link(self,unique_id,kws1,kws2,score):
        """Add a crosslink. They are organized by their unique_ids.
        @param unique_id The id is used to group crosslinks that are alternatives
        @param kws1 A dictionary of selection keywords for the first residue
        @param kws2 A dictionary of selection keywards for the second residue
        @param score A score that might be used later for filtering
        @note The dictionaries can contain any Selection argument like
              molecule or residue_index
        """
        self.data[unique_id].append(CrossLink(unique_id,kws1,kws2,score))
    def copy_cross_link(self,xl):
        if type(xl) is not CrossLink:
            raise Exception("CrossLinkData::copy_cross_link() requires a Crosslink object")
        self.data[xl.unique_id].append(xl)
    def __getitem__(self, key):
        return self.data[key]
    def __repr__(self):
        ret="CrossLinkData with these entries:\n"
        for d in self.data:
            for xl in self.data[d]:
                ret+=repr(xl)+'\n'
        return ret
    def keys(self):
        return self.data.keys()
    def values(self):
        return self.data.values()
    def __contains__(self, item):
        return item in self.data
    def __iter__(self):
        return iter(self.data)
    def __len__(self):
        return len(self.data)

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
        root_directory_of_stat_file = os.path.dirname(os.path.dirname(sf))
        print("getting data from file %s" % sf)
        po = IMP.pmi.output.ProcessOutput(sf)
        keywords = po.get_keys()

        feature_keywords = [score_key,
                            rmf_file_key,
                            rmf_file_frame_key]

        for k in keywords:
            for fk in feature_keys:
                if fk in k:
                    feature_keywords.append(k)

        if prefiltervalue is None:
            fields = po.get_fields(feature_keywords,
                                   get_every=get_every)
        else:
            fields = po.get_fields(feature_keywords,
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

        if feature_keywords is not None:
            for k in feature_keywords:
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

        prot=prots[state_number]
        # getting the particles
        part_dict = IMP.pmi.analysis.get_particles_at_resolution_one(prot)
        all_particles=[pp for key in part_dict for pp in part_dict[key]]
        all_ps_set=set(all_particles)
        # getting the coordinates
        model_coordinate_dict = {}
        template_coordinate_dict={}
        rmsd_coordinate_dict={}
        for pr in part_dict:
            model_coordinate_dict[pr] = np.array(
               [np.array(IMP.core.XYZ(i).get_coordinates()) for i in part_dict[pr]])

        if alignment_components is not None:
            for pr in alignment_components:
                if type(alignment_components[pr]) is str:
                    name=alignment_components[pr]
                    s=IMP.atom.Selection(prot,molecule=name)
                elif type(alignment_components[pr]) is tuple:
                    name=alignment_components[pr][2]
                    rend=alignment_components[pr][1]
                    rbegin=alignment_components[pr][0]
                    s=IMP.atom.Selection(prot,molecule=name,residue_indexes=range(rbegin,rend+1))
                ps=s.get_selected_particles()
                filtered_particles=[p for p in ps if p in all_ps_set]
                template_coordinate_dict[pr] = \
                    [list(map(float,IMP.core.XYZ(i).get_coordinates())) for i in filtered_particles]

        if rmsd_calculation_components is not None:
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
                rmsd_coordinate_dict[pr] = \
                    [list(map(float,IMP.core.XYZ(i).get_coordinates())) for i in filtered_particles]

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
    rmf_file = rmf_tuple[1]
    frame_number = rmf_tuple[2]

    prots = IMP.pmi.analysis.get_hiers_from_rmf(model,
                                              frame_number,
                                              rmf_file)

    prot=prots[state_number]

    # getting the particles
    part_dict = IMP.pmi.analysis.get_particles_at_resolution_one(prot)
    all_particles=[pp for key in part_dict for pp in part_dict[key]]
    all_ps_set=set(all_particles)
    # getting the coordinates
    rmsd_bead_size_dict={}

    if rmsd_calculation_components is not None:
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
