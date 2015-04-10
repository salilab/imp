## \example pmi/setup_analysis.py

"""This script creates input files for pmi/bin/run_clustering.py
"""
from __future__ import print_function
import sys,os
import glob

output_prefix="./test_output/run"
nclusters = 2

### all the possible subunit groups
head                   = ("med6","med8","med11","med17","med18","med20","med22")
middle                 = ("med1","med4","med7","med9","med10","med14","med17","med18","med19","med21","med31")
sub_middle             = ("med4","med9","med7","med21","med31")
tail                   = ("med2","med3","med5","med14","med15","med16")
full                   = tuple(set(head+middle+tail))

std = ["ISDCrossLinkMS_Distance_intrarb",
       "ISDCrossLinkMS_Distance_interrb",
       "ISDCrossLinkMS_Data_Score",
       "SimplifiedModel_Linker_Score_None",
       "ISDCrossLinkMS_Psi",
       "ISDCrossLinkMS_Sigma"]

em = ["GaussianEMRestraint_None"]

# subunits for each job
jobs = [[sub_middle,std+em],
        [middle,std],
        [middle,std],
        [middle,std],
        [middle,std],
        [middle,std],
        [middle,std],
        [full,std+em],
        [full,std+em],
        [middle,std+em],
        [middle,std+em],
        [tail,std+em],
        [tail,std+em],
        [tail,std+em],
        [tail,std+em],
        [full,std+em],
        [full,std+em],
        [full,std+em],
        [full,std+em],
        [full,std+em],
        [full,std+em],
        [middle,std+em],
        [middle,std+em],
        [tail,std+em],
        [tail,std+em],
        [middle,std+em],
        [middle,std+em],
        [full,std+em],
        [tail,std+em],
        [tail,std+em]]

### info common to all jobs


other_options={'global_output_dir' : 'output/',
               'number_of_best_scoring_models' : 500,
               'distance_matrix_file' : "distance.rawmatrix.pkl",
               'load_distance_matrix_file' : 0,
               'skip_clustering' : 0,
               'display_plot' : 0,
               'exit_after_display' : 0,
               'get_every' : 1,
               'number_of_clusters' : nclusters,
               'voxel_size' : 3.0}

for nj,(subunits,feature_keys) in enumerate(jobs):
    if not os.path.isdir(output_prefix+str(nj+1)):
        continue
    out_fn = os.path.join(output_prefix+str(nj+1),'analysis.txt')
    print('writing',out_fn)
    d = os.path.dirname(out_fn)
    outf=open(out_fn,'w')
    dirs=[]

    for dd in glob.glob(os.path.join(output_prefix+str(nj+1),'*')):
        if os.path.isdir(dd) and ('modeling' in dd or 'jackmodel' in dd):
            if os.path.isfile(os.path.join(dd,'best.scores.rex.py')):
                dirs.append(dd)
    if len(dirs)==0:
        continue
    output_dir = '%s/clustering/kmeans_2_%i/'%(d,nclusters)
    if not os.path.isdir(os.path.join(d,'clustering')):
        os.mkdir(os.path.join(d,'clustering'))

    outf.write('subunits ')
    for subunit in subunits:
        outf.write(subunit+' ')

    outf.write('\nmerge_directories ')
    for md in dirs:
        outf.write(md+' ')

    outf.write('\noutput_dir '+output_dir)

    outf.write('\nfeature_keys ')
    for feature in feature_keys:
        outf.write(feature+' ')
    outf.write('\n')

    for key in other_options:
        outf.write(key+" "+str(other_options[key])+'\n')

    # lastly, get prefilter value
    scores=[]
    for d in dirs:
        inf=open(os.path.join(d,'best.scores.rex.py'),'r')
        l=inf.readlines()[0].replace('self.','')
        exec(l)
        scores.append(best_score_list[-1])
        inf.close()
    outf.write('prefilter_value %.2f\n'%max(scores))

    outf.close()
