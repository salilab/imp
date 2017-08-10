from __future__ import print_function
import IMP
import IMP.em
import IMP.pmi
import IMP.rmf
import IMP.pmi.analysis
import IMP.pmi.output
import IMP.pmi.macros
from optparse import OptionParser
import os
import glob

def parse_args():
    usage = """%prog [options] <analysis_option> <input file>
    Currently implemented analysis options: cluster, precision, rmsf
    The input file contains all the setup options:

    (general)
    subunits
    global_output_dir = "./"

    (cluster)
    feature_keys = None
    outputdir = "kmeans_2_1/"
    number_of_best_scoring_models = 100
    alignment_components = None
    rmsd_calculation_components = components_names
    distance_matrix_file = "distance.rawmatrix.pkl"
    feature_keys = []
    load_distance_matrix_file = False,
    skip_clustering = 0
    display_plot = 0
    exit_after_display = 0
    get_every = 1
    is_mpi = 1
    number_of_clusters = 1
    voxel_size = 3.0
    """

    parser = OptionParser(usage)
    parser.add_option("-m","--use_mpi",dest="use_mpi",action="store_true",default=False,
                      help="Use MPI (works for some scripts)")
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("incorrect number of arguments")
    return options,args

def str2bool(s):
    if s=='1':
        return True
    else:
        return False

def run():
    # handle input
    options,args = parse_args()
    inf = open(args[1],'r')
    mdl = IMP.Model()

    info={'merge_directories' : './',
          'global_output_dir' : './',
          'feature_keys':None,
          'number_of_best_scoring_models' : 100,
          'distance_matrix_file' : "distance.rawmatrix.pkl",
          'outputdir' : "kmeans_2_1/" ,
          'load_distance_matrix_file' : 0,
          'skip_clustering' : 0,
          'display_plot' : 0,
          'exit_after_display' : 0,
          'get_every' : 0,
          'number_of_clusters' : 1,
          'voxel_size' : 3.0}
    new_dict={}
    for l in inf:
        fields = l.split()
        key = fields[0]
        if len(fields)>2:
            new_dict[key]=fields[1:]
        else:
            new_dict[key]=fields[1]
    info.update(new_dict)
    if type(info['merge_directories']) is not list:
        info['merge_directories']=[info['merge_directories']]
    info['number_of_best_scoring_models']=int(info['number_of_best_scoring_models'])
    info['load_distance_matrix_file']=str2bool(info['load_distance_matrix_file'])
    info['skip_clustering']=str2bool(info['skip_clustering'])
    info['display_plot']=str2bool(info['display_plot'])
    info['exit_after_display']=str2bool(info['exit_after_display'])
    info['get_every']=int(info['get_every'])
    info['number_of_clusters']=int(info['number_of_clusters'])
    info['voxel_size']=float(info['voxel_size'])
    info['prefilter_value']=float(info['prefilter_value'])

    density_sels={}
    rmsd_sels={}
    for s in info['subunits']:
        density_sels[s]=[s]
        rmsd_sels[s]=s

    if args[0]=="cluster":
        print('\nRUNNING CLUSTERING WITH THESE OPTIONS')
        for k in info:
            print(k,':',info[k])

        mc=IMP.pmi.macros.AnalysisReplicaExchange0(mdl,
                                                   stat_file_name_suffix="stat",
                                                   merge_directories=info['merge_directories'],
                                                   global_output_directory=info['global_output_dir'],
                                                   rmf_dir="rmfs/")

        mc.clustering("SimplifiedModel_Total_Score_None",
                      "rmf_file",
                      "rmf_frame_index",
                      prefiltervalue=info['prefilter_value'],
                      number_of_best_scoring_models=info['number_of_best_scoring_models'],
                      alignment_components=None,
                      rmsd_calculation_components=rmsd_sels,
                      distance_matrix_file="distance.rawmatrix.pkl",
                      outputdir=info['output_dir'],
                      feature_keys=info['feature_keys'],
                      load_distance_matrix_file=info['load_distance_matrix_file'],
                      skip_clustering=info['skip_clustering'],
                      display_plot=info['display_plot'],
                      exit_after_display=info['exit_after_display'],
                      get_every=info['get_every'],
                      is_mpi=options.use_mpi,
                      number_of_clusters=info['number_of_clusters'],
                      voxel_size=info['voxel_size'],
                      density_custom_ranges=density_sels)

    elif args[0]=='precision':
        print('\nRUNNING PRECISION WITH THESE OPTIONS')
        for k in info:
            print(k,':',info[k])
        for cldir in glob.glob(os.path.join(info['output_dir'],'cluster.*')):
            print('precision in dir',cldir)
            rmfs=glob.glob(cldir+'/*.rmf3')
            frames=[0]*len(rmfs)
            pr=IMP.pmi.analysis.Precision(mdl,'one',selection_dictionary=density_sels)
            pr.set_precision_style('pairwise_rmsd')
            pr.add_structures(zip(rmfs,frames),is_mpi=options.use_mpi)
            pr.get_precision(cldir+"/precision.out",is_mpi=options.use_mpi,skip=1)

    elif args[0]=='rmsf':
        print('\nRUNNING RMSF WITH THESE OPTIONS')
        for k in info:
            print(k,':',info[k])
        for cldir in glob.glob(os.path.join(info['output_dir'],'cluster.*')):
            rmfs=glob.glob(cldir+'/*.rmf3')
            frames=[0]*len(rmfs)
            pr=IMP.pmi.analysis.Precision(mdl,'one',selection_dictionary=density_sels)
            pr.set_precision_style('pairwise_rmsd')
            pr.add_structures(zip(rmfs,frames),is_mpi=options.use_mpi)
            pr.get_rmsf(cldir+"/",is_mpi=options.use_mpi,skip=1)

    else:
        print('ERROR: the only analysis options are cluster, precision, and rmsf')

if __name__=="__main__":
    run()
