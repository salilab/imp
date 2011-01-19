import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.em2d
import os
import csv
from math import *

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

def get_rows(fn,delimiter=" ",comment="#"):
    """ ge the rows of a file:
        rows - a list of rows to extract. E.g [0,3,5]
        If empty, all the rows are extracted
        lines starting with the comment character are ignored """
    rows=[]
    # get a reader for the file
    reader=csv.reader(open(fn,"r"),delimiter=delimiter,skipinitialspace=True)
    for row in reader:
        if(row!=[] and row[0][0]!=comment): # not empty or comment row
            rows.append(row)
    return rows

def compare_linkage_matrix(mat1,mat2):
    """Compares two linkage matrices from the clustering routines """

    n_rows = len(mat1)
    for i in range(0,n_rows):
        for j in range(0,3):
            self.assertAlmostEqual(mat1[i][j],mat2[i][j], delta=0.01,
                 msg="Linkage matrices are not equal")



class ClusteringTests(IMP.test.TestCase):

    def test_cluster_of_models(self):
        """Test hierarquical clustering of models"""
        input_dir=self.get_input_file_name("")
        sub_dir = input_dir+"/clustering/"
        os.chdir(sub_dir)
        fn_selection = "all-models-1z5s.sel"
        # Load models
        model = IMP.Model()
        ssel = IMP.atom.ATOMPDBSelector()
        coords =[]
        fn_models = IMP.em2d.read_selection_file(fn_selection)
        n_models = len(fn_models)
        for fn in fn_models:
            h=IMP.atom.read_pdb(fn,model,ssel,True,True);
            xyz=IMP.core.XYZs(IMP.atom.get_leaves(h))
            coords.append( [x.get_coordinates() for x in xyz])
        # compute rmsds
        x=IMP.Floats(0.0 for i in range(0,n_models))
        rmsds=[IMP.Floats(0.0 for i in range(0,n_models))
                  for n in xrange(0,n_models)]
        for i in xrange(0,n_models):
            for j in xrange(i+1,n_models):
                if(i!=j):
                    t=IMP.algebra.get_transformation_aligning_first_to_second(
                                                      coords[i],
                                                      coords[j])
                    temp = [t.get_transformed(v) for v in coords[i]]
                    rmsds[i][j]=IMP.atom.get_rmsd(temp,coords[j])
                    rmsds[j][i]=rmsds[i][j]
        # cluster
        linkage_mats=[]
        cluster_set = \
            IMP.em2d.do_hierarchical_clustering_single_linkage(rmsds)
        mat1=cluster_set.get_linkage_matrix()
        print "Single Linkage Matrix"
        for m in mat1:
            print m
        mat1 = [[x for x in row] for row in mat1]
        linkage_mats.append(mat1)
        cluster_set = \
            IMP.em2d.do_hierarchical_clustering_complete_linkage(rmsds)
        mat2=cluster_set.get_linkage_matrix()
        print "Complete Linkage Matrix"
        for m in mat2:
            print m
        mat2 = [[x for x in row] for row in mat2]
        linkage_mats.append(mat2)
        cluster_set = \
            IMP.em2d.do_hierarchical_clustering_average_distance_linkage(rmsds)

        mat3=cluster_set.get_linkage_matrix()
        print "Average distance Linkage Matrix"
        for m in mat3:
            print m
        mat3 = [[x for x in row] for row in mat3]
        linkage_mats.append(mat3)
        # check
        filenames = ["single_linkage_results.txt",
                     "complete_linkage_results.txt",
                     "average_distance_linkage_results.txt"]

        for i in range(0,len(linkage_mats)):
            rows = get_rows(filenames[i])
            print rows
            stored_mat=[[float(col) for col in row] for row in rows]
            mat = linkage_mats[i]
            msg = "Linkage matrices are not equal in "+ filenames[i]
            for j in range(0,len(linkage_mats[i])):
                for k in range(0,3):
                    self.assertAlmostEqual(mat[j][k],stored_mat[j][k],
                                           delta=0.01,msg=msg)
        os.chdir(input_dir)

if __name__ == '__main__':
    IMP.test.main()
