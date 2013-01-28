## \example em2d/clustering_of_pdb_models.py
## This example clusters pdb models of an structure, chosen from a
## selection file.
##
## It is assumed that all the pdb files belong to the same structure
## and that the order of the atoms in the pdb files is the same in all files.
##
## After the clustering procedure, a linkage matrix is generated.
##

import IMP
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import os
import sys
import csv
"""
    Clustering of pdb models.
    This script clusters pdb models of an structure, chosen from a
    selection file.
    - It is assumed that all the pdb files belong to the same structure
    and that the order of the atoms in the pdb files is the same in all files
    - After the clustering procedure, a linkage matrix is generated.


"""

if sys.platform == 'win32':
    sys.stderr.write("this example does not currently work on Windows\n")
    sys.exit(0)

def get_columns(fn,cols=[],delimiter=" ",comment="#"):
    """ ge the columns of a file:
        cols - a list of columns to extract. E.g [0,3,5]
               If empty, all the columns are extracted
        lines starting with the comment character are ignored """
    columns=[[] for i in cols]
    # get a reader for the file
    reader=csv.reader(open(fn,"r"),delimiter=delimiter,skipinitialspace=True)
    for row in reader:
        if(row!=[] and row[0][0]!=comment): # not empty or comment row
            if(cols==[]):
                for i in range(0,len(row)):
                    columns[i].append(row[i])
            else:
                for i in range(0,len(cols)):
                    columns[i].append(row[cols[i]])
    return columns


def argmin(sequence):
    """ Argmin function: Returns the pair (min_value,min_index),
        where min_index is the index of the minimum value
    """
    min_value = sequence[0]
    min_index = 0
    for i in range(0,len(sequence)):
#        print "argmin - checking ",sequence[i]
        if(sequence[i]<min_value):
            min_value = sequence[i]
            min_index =i
#            print "argmin - selecting ",min_value,min_index
    return min_value,min_index

#***************************


fn_selection = em2d.get_example_path("all-models-1z5s.sel")
fn_em2d_scores = em2d.get_example_path("em2d_scores_for_clustering.data")
# Load models
print "Reading models ..."
model = IMP.Model()
ssel = atom.ATOMPDBSelector()
coords =[]
fn_models = em2d.read_selection_file(fn_selection)
n_models = len(fn_models)
hierarchies=[]
for fn in fn_models:
    fn_model=em2d.get_example_path(fn)
    h=atom.read_pdb(fn_model,model,ssel,True)
    hierarchies.append(h)
    xyz=core.XYZs(atom.get_leaves(h))
    coords.append( [x.get_coordinates() for x in xyz])

print "Computing matrix of RMSD ..."
rmsds=[[0.0 for i in range(0,n_models)] for n in range(0,n_models)]
transformations=[[[] for i in range(0,n_models)]  for j in range(0,n_models)]
# fill rmsd and transformations
for i in xrange(0,n_models):
    for j in xrange(i+1,n_models):
        if(i!=j):
            t=IMP.algebra.get_transformation_aligning_first_to_second(
                                              coords[i],
                                              coords[j])
            transformations[i][j]=t
            transformations[j][i]=t.get_inverse()
            temp = [t.get_transformed(v) for v in coords[i]]
            rmsd=IMP.atom.get_rmsd(temp,coords[j])
            rmsds[i][j]=rmsd
            rmsds[j][i]=rmsd

# cluster
print "Clustering (Complete linkage method)..."
cluster_set = em2d.do_hierarchical_clustering_complete_linkage(rmsds)
mat2=cluster_set.get_linkage_matrix()
print "Complete Linkage Matrix"
for m in mat2:
    print m

# Read scores from the scores file
em2d_scores = get_columns(fn_em2d_scores,[1])
em2d_scores = em2d_scores[0]

# get biggest clusters below a certain rmsd
rmsd_cutoff=1.4
print "clusters below cutoff",rmsd_cutoff,"Angstroms"
clusters=cluster_set.get_clusters_below_cutoff(rmsd_cutoff)
for c in clusters:
    elems=cluster_set.get_cluster_elements(c)
    scores_elements=[]
    for cid in elems:
        scores_elements.append(em2d_scores[cid])
    print "Cluster",c,":",elems,scores_elements,
    # find model with best score
    min_value,min_index= argmin(scores_elements)
    min_elem_id=elems[min_index]
    # The representative element is the one with the minimum em2d score
    print "representative element",min_elem_id,min_value
    for i in elems:
        pdb_name="cluster-%03d-elem-%03d.pdb" % (c,i)

        if(i!=min_elem_id):
            print "Writing element",i,"aligned to ",min_elem_id,":",pdb_name
            T=core.Transform(transformations[i][min_elem_id])
            ps=atom.get_leaves(hierarchies[i])
            for p in ps:
                T.apply(p)
        else:
            print "Writing representative element",min_elem_id,":",pdb_name
        atom.write_pdb(hierarchies[i],pdb_name)
