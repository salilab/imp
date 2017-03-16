#!/usr/bin/env python

"""@namespace IMP.pmi.analysis
   Tools for clustering and cluster analysis
"""
from __future__ import print_function
import IMP
import IMP.algebra
import IMP.em
import IMP.pmi
import IMP.pmi.tools
import IMP.pmi.output
import IMP.rmf
import RMF
import IMP.pmi.analysis
from operator import itemgetter
from copy import deepcopy
from math import log,sqrt
import itertools
import numpy as np


class Alignment(object):
    """Performs alignment and RMSD calculation for two sets of coordinates

    The class also takes into account non-equal stoichiometry of the proteins.
    If this is the case, the protein names of proteins in multiple copies
    should be specified in the following form:
    nameA..1, nameA..2 (note two dots).
    """

    def __init__(self, template, query, weights=None):
        """Constructor.
           @param query {'p1':coords(L,3), 'p2':coords(L,3)}
           @param template {'p1':coords(L,3), 'p2':coords(L,3)}
           @param weights optional weights for each set of coordinates
        """
        self.query = query
        self.template = template
        self.weights=weights

        if len(self.query.keys()) != len(self.template.keys()):
            raise ValueError('''the number of proteins
                               in template and query does not match!''')

    def permute(self):
        # get unique protein names for each protein
        # this is, unfortunately, expecting that names are all 'Molname..X'
        #  where X is different for each copy.
        self.proteins = sorted(self.query.keys())
        prots_uniq = [i.split('..')[0] for i in self.proteins]

        # for each unique name, store list of permutations
        # e.g. for keys A..1,A..2 store P[A] = [[A..1,A..2],[A..2,A..1]]
        # then store the product: [[[A1,A2],[B1,B2]],[[A1,A2],[B2,B1]],
        #                          [[A2,A1],[B1,B2]],[[A2,A1],[B2,B1]]]
        P = {}
        for p in prots_uniq:
            np = prots_uniq.count(p)
            copies = [i for i in self.proteins if i.split('..')[0] == p]
            prmts = list(itertools.permutations(copies, len(copies)))
            P[p] = prmts
        self.P = P
        self.Product = list(itertools.product(*P.values()))

    def get_rmsd(self):

        self.permute()

        template_xyz = []
        weights = []
        torder = sum([list(i) for i in self.Product[0]], [])
        for t in torder:
            template_xyz += [IMP.algebra.Vector3D(i) for i in self.template[t]]
            if self.weights is not None:
                weights += [i for i in self.weights[t]]
        #template_xyz = np.array(template_xyz)

        self.rmsd = 10000000000.
        for comb in self.Product:

            order = sum([list(i) for i in comb], [])
            query_xyz = []
            for p in order:
                query_xyz += [IMP.algebra.Vector3D(i) for i in self.query[p]]
            #query_xyz = np.array(query_xyz)
            #if len(template_xyz) != len(query_xyz):
            #    print '''Alignment.get_rmsd: ERROR: the number of coordinates
            #                   in template and query does not match!'''
            #    exit()

            if self.weights is not None:
                dist=IMP.algebra.get_weighted_rmsd(template_xyz, query_xyz, weights)
            else:
                dist=IMP.algebra.get_rmsd(template_xyz, query_xyz)
            #dist = sqrt(
            #    sum(np.diagonal(cdist(template_xyz, query_xyz) ** 2)) / len(template_xyz))
            if dist < self.rmsd:
                self.rmsd = dist
        return self.rmsd

    def align(self):
        from scipy.spatial.distance import cdist

        self.permute()

        # create flat coordinate list from template in standard order
        # then loop through the permutations and try to align and get RMSD
        # should allow you to try all mappings within each protein
        template_xyz = []
        torder = sum([list(i) for i in self.Product[0]], [])
        for t in torder:
            template_xyz += [IMP.algebra.Vector3D(i) for i in self.template[t]]
        # template_xyz = np.array(template_xyz)
        self.rmsd, Transformation = 10000000000., ''

        # then for each permutation, get flat list of coords and get RMSD
        for comb in self.Product:
            order = sum([list(i) for i in comb], [])
            query_xyz = []
            for p in order:
                query_xyz += [IMP.algebra.Vector3D(i) for i in self.query[p]]
            #query_xyz = np.array(query_xyz)

            if len(template_xyz) != len(query_xyz):
                raise ValueError('''the number of coordinates
                               in template and query does not match!''')

            transformation = IMP.algebra.get_transformation_aligning_first_to_second(
                query_xyz,
                template_xyz)
            query_xyz_tr = [transformation.get_transformed(n)
                            for n in query_xyz]

            dist = sqrt(
                sum(np.diagonal(cdist(template_xyz, query_xyz_tr) ** 2)) / len(template_xyz))
            if dist < self.rmsd:
                self.rmsd = dist
                Transformation = transformation

        # return the transformation
        return (self.rmsd, Transformation)


# TEST for the alignment ###
"""
Proteins = {'a..1':np.array([np.array([-1.,1.])]),
            'a..2':np.array([np.array([1.,1.,])]),
            'a..3':np.array([np.array([-2.,1.])]),
            'b':np.array([np.array([0.,-1.])]),
            'c..1':np.array([np.array([-1.,-1.])]),
            'c..2':np.array([np.array([1.,-1.])]),
            'd':np.array([np.array([0.,0.])]),
            'e':np.array([np.array([0.,1.])])}

Ali = Alignment(Proteins, Proteins)
Ali.permute()
if Ali.get_rmsd() == 0.0: print 'successful test!'
else: print 'ERROR!'; exit()
"""


# ----------------------------------
class Violations(object):

    def __init__(self, filename):

        self.violation_thresholds = {}
        self.violation_counts = {}

        data = open(filename)
        D = data.readlines()
        data.close()

        for d in D:
            d = d.strip().split()
            self.violation_thresholds[d[0]] = float(d[1])

    def get_number_violated_restraints(self, rsts_dict):
        num_violated = 0
        for rst in self.violation_thresholds:
            if rst not in rsts_dict:
                continue  # print rst;
            if float(rsts_dict[rst]) > self.violation_thresholds[rst]:
                num_violated += 1
                if rst not in self.violation_counts:
                    self.violation_counts[rst] = 1
                else:
                    self.violation_counts[rst] += 1
        return num_violated


# ----------------------------------
class Clustering(object):
    """A class to cluster structures.
    Uses scipy's cdist function to compute distance matrices
    and sklearn's kmeans clustering module.
    """
    def __init__(self,rmsd_weights=None):
        """Constructor.
           @param rmsd_weights Flat list of weights for each particle
                               (if they're coarse)
        """
        try:
            from mpi4py import MPI
            self.comm = MPI.COMM_WORLD
            self.rank = self.comm.Get_rank()
            self.number_of_processes = self.comm.size
        except ImportError:
            self.number_of_processes = 1
            self.rank = 0
        self.all_coords = {}
        self.structure_cluster_ids = None
        self.tmpl_coords = None
        self.rmsd_weights=rmsd_weights

    def set_template(self, part_coords):

        self.tmpl_coords = part_coords

    def fill(self, frame, Coords):
        """Add coordinates for a single model."""

        self.all_coords[frame] = Coords

    def dist_matrix(self):

        self.model_list_names = list(self.all_coords.keys())
        self.model_indexes = list(range(len(self.model_list_names)))
        self.model_indexes_dict = dict(
            list(zip(self.model_list_names, self.model_indexes)))
        model_indexes_unique_pairs = list(itertools.combinations(self.model_indexes, 2))

        my_model_indexes_unique_pairs = IMP.pmi.tools.chunk_list_into_segments(
            model_indexes_unique_pairs,
            self.number_of_processes)[self.rank]

        print("process %s assigned with %s pairs" % (str(self.rank), str(len(my_model_indexes_unique_pairs))))

        (raw_distance_dict, self.transformation_distance_dict) = self.matrix_calculation(self.all_coords,
                                                                                         self.tmpl_coords,
                                                                                         my_model_indexes_unique_pairs)

        if self.number_of_processes > 1:
            raw_distance_dict = IMP.pmi.tools.scatter_and_gather(
                raw_distance_dict)
            pickable_transformations = self.get_pickable_transformation_distance_dict(
            )
            pickable_transformations = IMP.pmi.tools.scatter_and_gather(
                pickable_transformations)
            self.set_transformation_distance_dict_from_pickable(
                pickable_transformations)

        self.raw_distance_matrix = np.zeros(
            (len(self.model_list_names), len(self.model_list_names)))
        for item in raw_distance_dict:
            (f1, f2) = item
            self.raw_distance_matrix[f1, f2] = raw_distance_dict[item]
            self.raw_distance_matrix[f2, f1] = raw_distance_dict[item]

    def get_dist_matrix(self):
        return self.raw_distance_matrix

    def do_cluster(self, number_of_clusters,seed=None):
        """Run K-means clustering
        @param number_of_clusters Num means
        @param seed the random seed
        """
        from sklearn.cluster import KMeans
        if seed is not None:
            np.random.seed(seed)
        try:
            # check whether we have the right version of sklearn
            kmeans = KMeans(n_clusters=number_of_clusters)
        except TypeError:
            # sklearn older than 0.12
            kmeans = KMeans(k=number_of_clusters)
        kmeans.fit_predict(self.raw_distance_matrix)

        self.structure_cluster_ids = kmeans.labels_

    def get_pickable_transformation_distance_dict(self):
        pickable_transformations = {}
        for label in self.transformation_distance_dict:
            tr = self.transformation_distance_dict[label]
            trans = tuple(tr.get_translation())
            rot = tuple(tr.get_rotation().get_quaternion())
            pickable_transformations[label] = (rot, trans)
        return pickable_transformations

    def set_transformation_distance_dict_from_pickable(
        self,
            pickable_transformations):
        self.transformation_distance_dict = {}
        for label in pickable_transformations:
            tr = pickable_transformations[label]
            trans = IMP.algebra.Vector3D(tr[1])
            rot = IMP.algebra.Rotation3D(tr[0])
            self.transformation_distance_dict[
                label] = IMP.algebra.Transformation3D(rot, trans)

    def save_distance_matrix_file(self, file_name='cluster.rawmatrix.pkl'):
        import pickle
        outf = open(file_name + ".data", 'wb')

        # to pickle the transformation dictionary
        # you have to save the arrays correposnding to
        # the transformations

        pickable_transformations = self.get_pickable_transformation_distance_dict(
        )
        pickle.dump(
            (self.structure_cluster_ids,
             self.model_list_names,
             pickable_transformations),
            outf)

        np.save(file_name + ".npy", self.raw_distance_matrix)

    def load_distance_matrix_file(self, file_name='cluster.rawmatrix.pkl'):
        import pickle

        inputf = open(file_name + ".data", 'rb')
        (self.structure_cluster_ids, self.model_list_names,
         pickable_transformations) = pickle.load(inputf)
        inputf.close()

        self.raw_distance_matrix = np.load(file_name + ".npy")

        self.set_transformation_distance_dict_from_pickable(
            pickable_transformations)
        self.model_indexes = list(range(len(self.model_list_names)))
        self.model_indexes_dict = dict(
            list(zip(self.model_list_names, self.model_indexes)))

    def plot_matrix(self, figurename="clustermatrix.pdf"):
        import matplotlib as mpl
        mpl.use('Agg')
        import matplotlib.pylab as pl
        from scipy.cluster import hierarchy as hrc

        fig = pl.figure(figsize=(10,8))
        ax = fig.add_subplot(212)
        dendrogram = hrc.dendrogram(
            hrc.linkage(self.raw_distance_matrix),
            color_threshold=7,
            no_labels=True)
        leaves_order = dendrogram['leaves']
        ax.set_xlabel('Model')
        ax.set_ylabel('RMSD [Angstroms]')

        ax2 = fig.add_subplot(221)
        cax = ax2.imshow(
            self.raw_distance_matrix[leaves_order,
                                     :][:,
                                        leaves_order],
            interpolation='nearest')
        cb = fig.colorbar(cax)
        cb.set_label('RMSD [Angstroms]')
        ax2.set_xlabel('Model')
        ax2.set_ylabel('Model')

        pl.savefig(figurename, dpi=300)
        pl.close(fig)

    def get_model_index_from_name(self, name):
        return self.model_indexes_dict[name]

    def get_cluster_labels(self):
        # this list
        return list(set(self.structure_cluster_ids))

    def get_number_of_clusters(self):
        return len(self.get_cluster_labels())

    def get_cluster_label_indexes(self, label):
        return (
            [i for i, l in enumerate(self.structure_cluster_ids) if l == label]
        )

    def get_cluster_label_names(self, label):
        return (
            [self.model_list_names[i]
                for i in self.get_cluster_label_indexes(label)]
        )

    def get_cluster_label_average_rmsd(self, label):

        indexes = self.get_cluster_label_indexes(label)

        if len(indexes) > 1:
            sub_distance_matrix = self.raw_distance_matrix[
                indexes, :][:, indexes]
            average_rmsd = np.sum(sub_distance_matrix) / \
                (len(sub_distance_matrix)
                 ** 2 - len(sub_distance_matrix))
        else:
            average_rmsd = 0.0
        return average_rmsd

    def get_cluster_label_size(self, label):
        return len(self.get_cluster_label_indexes(label))

    def get_transformation_to_first_member(
        self,
        cluster_label,
            structure_index):
        reference = self.get_cluster_label_indexes(cluster_label)[0]
        return self.transformation_distance_dict[(reference, structure_index)]

    def matrix_calculation(self, all_coords, template_coords, list_of_pairs):

        model_list_names = list(all_coords.keys())
        rmsd_protein_names = list(all_coords[model_list_names[0]].keys())
        raw_distance_dict = {}
        transformation_distance_dict = {}
        if template_coords is None:
            do_alignment = False
        else:
            do_alignment = True
            alignment_template_protein_names = list(template_coords.keys())

        for (f1, f2) in list_of_pairs:

            if not do_alignment:
                # here we only get the rmsd,
                # we need that for instance when you want to cluster conformations
                # globally, eg the EM map is a reference
                transformation = IMP.algebra.get_identity_transformation_3d()

                coords_f1 = dict([(pr, all_coords[model_list_names[f1]][pr])
                                 for pr in rmsd_protein_names])
                coords_f2 = {}
                for pr in rmsd_protein_names:
                    coords_f2[pr] = all_coords[model_list_names[f2]][pr]

                Ali = Alignment(coords_f1, coords_f2, self.rmsd_weights)
                rmsd = Ali.get_rmsd()

            elif do_alignment:
                # here we actually align the conformations first
                # and than calculate the rmsd. We need that when the
                # protein(s) is the reference
                coords_f1 = dict([(pr, all_coords[model_list_names[f1]][pr])
                                 for pr in alignment_template_protein_names])
                coords_f2 = dict([(pr, all_coords[model_list_names[f2]][pr])
                                 for pr in alignment_template_protein_names])

                Ali = Alignment(coords_f1, coords_f2)
                template_rmsd, transformation = Ali.align()

                # here we calculate the rmsd
                # we will align two models based n the nuber of subunits provided
                # and transform coordinates of model 2 to model 1
                coords_f1 = dict([(pr, all_coords[model_list_names[f1]][pr])
                                 for pr in rmsd_protein_names])
                coords_f2 = {}
                for pr in rmsd_protein_names:
                    coords_f2[pr] = [transformation.get_transformed(
                        i) for i in all_coords[model_list_names[f2]][pr]]

                Ali = Alignment(coords_f1, coords_f2, self.rmsd_weights)
                rmsd = Ali.get_rmsd()

            raw_distance_dict[(f1, f2)] = rmsd
            raw_distance_dict[(f2, f1)] = rmsd
            transformation_distance_dict[(f1, f2)] = transformation
            transformation_distance_dict[(f2, f1)] = transformation

        return raw_distance_dict, transformation_distance_dict


class Precision(object):
    """A class to evaluate the precision of an ensemble.

    Also can evaluate the cross-precision of multiple ensembles.
    Supports MPI for coordinate reading.
    Recommended procedure:
      -# initialize object and pass the selection for evaluating precision
      -# call add_structures() to read in the data (specify group name)
      -# call get_precision() to evaluate inter/intra precision
      -# call get_rmsf() to evaluate within-group fluctuations
    """
    def __init__(self,model,
                 resolution=1,
                 selection_dictionary={}):
        """Constructor.
           @param model The IMP Model
           @param resolution Use 1 or 10 (kluge: requires that "_Res:X" is
                  part of the hier name)
           @param selection_dictionary Dictionary where keys are names for
                  selections and values are selection tuples for scoring
                  precision. "All" is automatically made as well
        """
        try:
            from mpi4py import MPI
            self.comm = MPI.COMM_WORLD
            self.rank = self.comm.Get_rank()
            self.number_of_processes = self.comm.size
        except ImportError:
            self.number_of_processes = 1
            self.rank = 0

        self.styles = ['pairwise_rmsd','pairwise_drmsd_k','pairwise_drmsd_Q',
                       'pairwise_drms_k','pairwise_rmsd','drmsd_from_center']
        self.style = 'pairwise_drmsd_k'
        self.structures_dictionary = {}
        self.reference_structures_dictionary = {}
        self.prots = []
        self.protein_names = None
        self.len_particles_resolution_one = None
        self.model = model
        self.rmf_names_frames = {}
        self.reference_rmf_names_frames = None
        self.reference_structure = None
        self.reference_prot = None
        self.selection_dictionary = selection_dictionary
        self.threshold = 40.0
        self.residue_particle_index_map = None
        self.prots = None
        if resolution in [1,10]:
            self.resolution = resolution
        else:
            raise KeyError("Currently only allow resolution 1 or 10")

    def _get_structure(self,rmf_frame_index,rmf_name):
        """Read an RMF file and return the particles"""
        rh = RMF.open_rmf_file_read_only(rmf_name)
        if not self.prots:
            print("getting coordinates for frame %i rmf file %s" % (rmf_frame_index, rmf_name))
            self.prots = IMP.rmf.create_hierarchies(rh, self.model)
            IMP.rmf.load_frame(rh, RMF.FrameID(rmf_frame_index))
        else:
            print("linking coordinates for frame %i rmf file %s" % (rmf_frame_index, rmf_name))
            IMP.rmf.link_hierarchies(rh, self.prots)
            IMP.rmf.load_frame(rh, RMF.FrameID(rmf_frame_index))
        del rh

        if self.resolution==1:
            particle_dict = get_particles_at_resolution_one(self.prots[0])
        elif self.resolution==10:
            particle_dict = get_particles_at_resolution_ten(self.prots[0])

        protein_names = list(particle_dict.keys())
        particles_resolution_one = []
        for k in particle_dict:
            particles_resolution_one += (particle_dict[k])

        if self.protein_names==None:
            self.protein_names = protein_names
        else:
            if self.protein_names!=protein_names:
                print("Error: the protein names of the new coordinate set is not compatible with the previous one")

        if self.len_particles_resolution_one==None:
            self.len_particles_resolution_one = len(particles_resolution_one)
        else:
            if self.len_particles_resolution_one!=len(particles_resolution_one):
                raise ValueError("the new coordinate set is not compatible with the previous one")

        return particles_resolution_one

    def add_structure(self,
                      rmf_name,
                      rmf_frame_index,
                      structure_set_name,
                      setup_index_map=False):
        """ Read a structure into the ensemble and store (as coordinates).
        @param rmf_name The name of the RMF file
        @param rmf_frame_index The frame to read
        @param structure_set_name Name for the set that includes this structure
                                  (e.g. "cluster 1")
        @param setup_index_map if requested, set up a dictionary to help
               find residue indexes
        """

        # decide where to put this structure
        if structure_set_name in self.structures_dictionary:
            cdict = self.structures_dictionary[structure_set_name]
            rmflist = self.rmf_names_frames[structure_set_name]
        else:
            self.structures_dictionary[structure_set_name]={}
            self.rmf_names_frames[structure_set_name]=[]
            cdict = self.structures_dictionary[structure_set_name]
            rmflist = self.rmf_names_frames[structure_set_name]

        # read the particles
        try:
            particles_resolution_one = self._get_structure(rmf_frame_index,rmf_name)
        except ValueError:
            print("something wrong with the rmf")
            return 0
        self.selection_dictionary.update({"All":self.protein_names})

        for selection_name in self.selection_dictionary:
            selection_tuple = self.selection_dictionary[selection_name]
            coords = self._select_coordinates(selection_tuple,particles_resolution_one,self.prots[0])
            if selection_name not in cdict:
                cdict[selection_name] = [coords]
            else:
                cdict[selection_name].append(coords)

        rmflist.append((rmf_name,rmf_frame_index))

        # if requested, set up a dictionary to help find residue indexes
        if setup_index_map:
            self.residue_particle_index_map = {}
            for prot_name in self.protein_names:
                self.residue_particle_index_map[prot_name] = \
                       self._get_residue_particle_index_map(
                           prot_name,
                           particles_resolution_one,self.prots[0])

    def add_structures(self,
                       rmf_name_frame_tuples,
                       structure_set_name):
        """Read a list of RMFs, supports parallel
        @param rmf_name_frame_tuples list of (rmf_file_name,frame_number)
        @param structure_set_name Name this set of structures (e.g. "cluster.1")
        """

        # split up the requested list to read in parallel
        my_rmf_name_frame_tuples=IMP.pmi.tools.chunk_list_into_segments(
            rmf_name_frame_tuples,self.number_of_processes)[self.rank]
        for nfr,tup in enumerate(my_rmf_name_frame_tuples):
            rmf_name=tup[0]
            rmf_frame_index=tup[1]
            # the first frame stores the map between residues and particles
            if self.residue_particle_index_map is None:
                setup_index_map = True
            else:
                setup_index_map = False
            self.add_structure(rmf_name,
                               rmf_frame_index,
                               structure_set_name,
                               setup_index_map)

        # synchronize the structures
        if self.number_of_processes > 1:
            self.rmf_names_frames=IMP.pmi.tools.scatter_and_gather(self.rmf_names_frames)
            if self.rank != 0:
                self.comm.send(self.structures_dictionary, dest=0, tag=11)
            elif self.rank == 0:
                for i in range(1, self.number_of_processes):
                    data_tmp = self.comm.recv(source=i, tag=11)
                    for key in self.structures_dictionary:
                        self.structures_dictionary[key].update(data_tmp[key])
                for i in range(1, self.number_of_processes):
                    self.comm.send(self.structures_dictionary, dest=i, tag=11)
            if self.rank != 0:
                self.structures_dictionary = self.comm.recv(source=0, tag=11)

    def _get_residue_particle_index_map(self,prot_name,structure,hier):
        # Creates map from all particles to residue numbers
        residue_particle_index_map = []
        if IMP.pmi.get_is_canonical(hier):
            s = IMP.atom.Selection(hier,molecules=[prot_name],
                                   resolution=1)
        else:
            s = IMP.atom.Selection(hier,molecules=[prot_name])
        all_selected_particles = s.get_selected_particles()
        intersection = list(set(all_selected_particles) & set(structure))
        sorted_intersection = IMP.pmi.tools.sort_by_residues(intersection)
        for p in sorted_intersection:
            residue_particle_index_map.append(IMP.pmi.tools.get_residue_indexes(p))
        return residue_particle_index_map


    def _select_coordinates(self,tuple_selections,structure,prot):
        selected_coordinates=[]
        for t in tuple_selections:
            if type(t)==tuple and len(t)==3:
                if IMP.pmi.get_is_canonical(prot):
                    s = IMP.atom.Selection(prot,molecules=[t[2]],residue_indexes=range(t[0],t[1]+1),
                                           resolution=1)
                else:
                    s = IMP.atom.Selection(prot,molecules=[t[2]],residue_indexes=range(t[0],t[1]+1))
                all_selected_particles = s.get_selected_particles()
                intersection = list(set(all_selected_particles) & set(structure))
                sorted_intersection = IMP.pmi.tools.sort_by_residues(intersection)
                cc = [tuple(IMP.core.XYZ(p).get_coordinates()) for p in sorted_intersection]
                selected_coordinates += cc
            elif type(t)==str:
                if IMP.pmi.get_is_canonical(prot):
                    s = IMP.atom.Selection(prot,molecules=[t],resolution=1)
                else:
                    s = IMP.atom.Selection(prot,molecules=[t])
                all_selected_particles = s.get_selected_particles()
                intersection = list(set(all_selected_particles) & set(structure))
                sorted_intersection = IMP.pmi.tools.sort_by_residues(intersection)
                cc = [tuple(IMP.core.XYZ(p).get_coordinates()) for p in sorted_intersection]
                selected_coordinates += cc
            else:
                raise ValueError("Selection error")
        return selected_coordinates

    def set_threshold(self,threshold):
        self.threshold = threshold

    def _get_distance(self,
                     structure_set_name1,
                     structure_set_name2,
                     selection_name,
                     index1,
                     index2):
        """ Compute distance between structures with various metrics """
        c1 = self.structures_dictionary[structure_set_name1][selection_name][index1]
        c2 = self.structures_dictionary[structure_set_name2][selection_name][index2]
        coordinates1 = [IMP.algebra.Vector3D(c) for c in c1]
        coordinates2 = [IMP.algebra.Vector3D(c) for c in c2]

        if self.style=='pairwise_drmsd_k':
            distance=IMP.atom.get_drmsd(coordinates1,coordinates2)
        if self.style=='pairwise_drms_k':
            distance=IMP.atom.get_drms(coordinates1,coordinates2)
        if self.style=='pairwise_drmsd_Q':
            distance=IMP.atom.get_drmsd_Q(coordinates1,coordinates2,self.threshold)

        if self.style=='pairwise_rmsd':
            distance=IMP.algebra.get_rmsd(coordinates1,coordinates2)
        return distance

    def _get_particle_distances(self,structure_set_name1,structure_set_name2,
                               selection_name,index1,index2):
        c1 = self.structures_dictionary[structure_set_name1][selection_name][index1]
        c2 = self.structures_dictionary[structure_set_name2][selection_name][index2]

        coordinates1 = [IMP.algebra.Vector3D(c) for c in c1]
        coordinates2 = [IMP.algebra.Vector3D(c) for c in c2]

        distances=[np.linalg.norm(a-b) for (a,b) in zip(coordinates1,coordinates2)]

        return distances

    def get_precision(self,
                      structure_set_name1,
                      structure_set_name2,
                      outfile=None,
                      skip=1,
                      selection_keywords=None):
        """ Evaluate the precision of two named structure groups. Supports MPI.
        When the structure_set_name1 is different from the structure_set_name2,
        this evaluates the cross-precision (average pairwise distances).
        @param outfile Name of the precision output file
        @param structure_set_name1  string name of the first structure set
        @param structure_set_name2  string name of the second structure set
        @param skip analyze every (skip) structure for the distance matrix calculation
        @param selection_keywords Specify the selection name you want to calculate on.
               By default this is computed for everything you provided in the constructor,
               plus all the subunits together.
        """
        if selection_keywords is None:
            sel_keys = list(self.selection_dictionary.keys())
        else:
            for k in selection_keywords:
                if k not in self.selection_dictionary:
                    raise KeyError("you are trying to find named selection " \
                        + k + " which was not requested in the constructor")
            sel_keys = selection_keywords

        if outfile is not None:
            of = open(outfile,"w")
        centroid_index = 0
        for selection_name in sel_keys:
            number_of_structures_1 = len(self.structures_dictionary[structure_set_name1][selection_name])
            number_of_structures_2 = len(self.structures_dictionary[structure_set_name2][selection_name])
            distances={}
            structure_pointers_1 = list(range(0,number_of_structures_1,skip))
            structure_pointers_2 = list(range(0,number_of_structures_2,skip))
            pair_combination_list = list(itertools.product(structure_pointers_1,structure_pointers_2))
            if len(pair_combination_list)==0:
                raise ValueError("no structure selected. Check the skip parameter.")

            # compute pairwise distances in parallel
            my_pair_combination_list = IMP.pmi.tools.chunk_list_into_segments(
                pair_combination_list,self.number_of_processes)[self.rank]
            my_length = len(my_pair_combination_list)
            for n,pair in enumerate(my_pair_combination_list):
                progression = int(float(n)/my_length*100.0)
                distances[pair] = self._get_distance(structure_set_name1,structure_set_name2,
                                                     selection_name,pair[0],pair[1])
            if self.number_of_processes > 1:
                distances = IMP.pmi.tools.scatter_and_gather(distances)

            # Finally compute distance to centroid
            if self.rank == 0:
                if structure_set_name1==structure_set_name2:
                    structure_pointers = structure_pointers_1
                    number_of_structures = number_of_structures_1

                    # calculate the distance from the first centroid
                    #  and determine the centroid
                    distance = 0.0
                    distances_to_structure = {}
                    distances_to_structure_normalization = {}

                    for n in structure_pointers:
                        distances_to_structure[n] = 0.0
                        distances_to_structure_normalization[n]=0

                    for k in distances:
                        distance += distances[k]
                        distances_to_structure[k[0]] += distances[k]
                        distances_to_structure[k[1]] += distances[k]
                        distances_to_structure_normalization[k[0]] += 1
                        distances_to_structure_normalization[k[1]] += 1

                    for n in structure_pointers:
                        distances_to_structure[n] = distances_to_structure[n]/distances_to_structure_normalization[n]

                    min_distance = min([distances_to_structure[n] for n in distances_to_structure])
                    centroid_index = [k for k, v in distances_to_structure.items() if v == min_distance][0]
                    centroid_rmf_name = self.rmf_names_frames[structure_set_name1][centroid_index]

                    centroid_distance = 0.0
                    distance_list = []
                    for n in range(number_of_structures):
                        dist = self._get_distance(structure_set_name1,structure_set_name1,
                                                             selection_name,centroid_index,n)
                        centroid_distance += dist
                        distance_list.append(dist)

                    #pairwise_distance=distance/len(distances.keys())
                    centroid_distance /= number_of_structures
                    #average_centroid_distance=sum(distances_to_structure)/len(distances_to_structure)
                    if outfile is not None:
                        of.write(str(selection_name)+" "+structure_set_name1+
                                        " average centroid distance "+str(centroid_distance)+"\n")
                        of.write(str(selection_name)+" "+structure_set_name1+
                                        " centroid index "+str(centroid_index)+"\n")
                        of.write(str(selection_name)+" "+structure_set_name1+
                                        " centroid rmf name "+str(centroid_rmf_name)+"\n")
                        of.write(str(selection_name)+" "+structure_set_name1+
                                        " median centroid distance  "+str(np.median(distance_list))+"\n")

                average_pairwise_distances=sum(distances.values())/len(list(distances.values()))
                if outfile is not None:
                    of.write(str(selection_name)+" "+structure_set_name1+" "+structure_set_name2+
                             " average pairwise distance "+str(average_pairwise_distances)+"\n")
        if outfile is not None:
            of.close()
        return centroid_index

    def get_rmsf(self,
                 structure_set_name,
                 outdir="./",
                 skip=1,
                 set_plot_yaxis_range=None):
        """ Calculate the residue mean square fluctuations (RMSF).
        Automatically outputs as data file and pdf
        @param structure_set_name Which structure set to calculate RMSF for
        @param outdir Where to write the files
        @param skip Skip this number of structures
        @param set_plot_yaxis_range In case you need to change the plot
        """
        # get the centroid structure for the whole complex
        centroid_index = self.get_precision(structure_set_name,
                                            structure_set_name,
                                            outfile=None,
                                            skip=skip)
        if self.rank==0:
            for sel_name in self.protein_names:
                self.selection_dictionary.update({sel_name:[sel_name]})
                try:
                    number_of_structures = len(self.structures_dictionary[structure_set_name][sel_name])
                except KeyError:
                    # that protein was not included in the selection
                    continue
                rpim = self.residue_particle_index_map[sel_name]
                outfile = outdir+"/rmsf."+sel_name+".dat"
                of = open(outfile,"w")
                residue_distances = {}
                residue_nblock = {}
                for index in range(number_of_structures):
                    distances = self._get_particle_distances(structure_set_name,
                                                             structure_set_name,
                                                             sel_name,
                                                             centroid_index,index)
                    for nblock,block in enumerate(rpim):
                        for residue_number in block:
                            residue_nblock[residue_number] = nblock
                            if residue_number not in residue_distances:
                                residue_distances[residue_number] = [distances[nblock]]
                            else:
                                residue_distances[residue_number].append(distances[nblock])

                residues = []
                rmsfs = []
                for rn in residue_distances:
                    residues.append(rn)
                    rmsf = np.std(residue_distances[rn])
                    rmsfs.append(rmsf)
                    of.write(str(rn)+" "+str(residue_nblock[rn])+" "+str(rmsf)+"\n")

                IMP.pmi.output.plot_xy_data(residues,rmsfs,title=sel_name,
                                            out_fn=outdir+"/rmsf."+sel_name,display=False,
                                            set_plot_yaxis_range=set_plot_yaxis_range,
                                            xlabel='Residue Number',ylabel='Standard error')
                of.close()


    def set_reference_structure(self,rmf_name,rmf_frame_index):
        """Read in a structure used for reference computation.
        Needed before calling get_average_distance_wrt_reference_structure()
        @param rmf_name The RMF file to read the reference
        @param rmf_frame_index The index in that file
        """
        particles_resolution_one = self._get_structure(rmf_frame_index,rmf_name)
        self.reference_rmf_names_frames = (rmf_name,rmf_frame_index)

        for selection_name in self.selection_dictionary:
            selection_tuple = self.selection_dictionary[selection_name]
            coords = self._select_coordinates(selection_tuple,
                                              particles_resolution_one,self.prots[0])
            self.reference_structures_dictionary[selection_name] = coords

    def get_rmsd_wrt_reference_structure_with_alignment(self,structure_set_name,alignment_selection_key):
        """First align then calculate RMSD
        @param structure_set_name: the name of the structure set
        @param alignment_selection: the key containing the selection tuples needed to make the alignment stored in self.selection_dictionary
        @return: for each structure in the structure set, returns the rmsd
        """
        if self.reference_structures_dictionary=={}:
            print("Cannot compute until you set a reference structure")
            return

        align_reference_coordinates = self.reference_structures_dictionary[alignment_selection_key]
        align_coordinates = self.structures_dictionary[structure_set_name][alignment_selection_key]
        transformations = []
        for c in align_coordinates:
            Ali = IMP.pmi.analysis.Alignment({"All":align_reference_coordinates}, {"All":c})
            transformation = Ali.align()[1]
            transformations.append(transformation)
        for selection_name in self.selection_dictionary:
            reference_coordinates = self.reference_structures_dictionary[selection_name]
            coordinates2 = [IMP.algebra.Vector3D(c) for c in reference_coordinates]
            distances = []
            for n,sc in enumerate(self.structures_dictionary[structure_set_name][selection_name]):
                coordinates1 = [transformations[n].get_transformed(IMP.algebra.Vector3D(c)) for c in sc]
                distance = IMP.algebra.get_rmsd(coordinates1,coordinates2)
                distances.append(distance)
            print(selection_name,"average rmsd",sum(distances)/len(distances),"median",self._get_median(distances),"minimum distance",min(distances))

    def _get_median(self,list_of_values):
        return np.median(np.array(list_of_values))

    def get_average_distance_wrt_reference_structure(self,structure_set_name):
        """Compare the structure set to the reference structure.
        @param structure_set_name The structure set to compute this on
        @note First call set_reference_structure()
        """
        ret = {}
        if self.reference_structures_dictionary=={}:
            print("Cannot compute until you set a reference structure")
            return
        for selection_name in self.selection_dictionary:
            reference_coordinates = self.reference_structures_dictionary[selection_name]
            coordinates2 = [IMP.algebra.Vector3D(c) for c in reference_coordinates]
            distances = []
            for sc in self.structures_dictionary[structure_set_name][selection_name]:
                coordinates1 = [IMP.algebra.Vector3D(c) for c in sc]
                if self.style=='pairwise_drmsd_k':
                    distance = IMP.atom.get_drmsd(coordinates1,coordinates2)
                if self.style=='pairwise_drms_k':
                    distance = IMP.atom.get_drms(coordinates1,coordinates2)
                if self.style=='pairwise_drmsd_Q':
                    distance = IMP.atom.get_drmsd_Q(coordinates1,coordinates2,self.threshold)
                if self.style=='pairwise_rmsd':
                    distance = IMP.algebra.get_rmsd(coordinates1,coordinates2)
                distances.append(distance)

            print(selection_name,"average distance",sum(distances)/len(distances),"minimum distance",min(distances),'nframes',len(distances))
            ret[selection_name] = {'average_distance':sum(distances)/len(distances),'minimum_distance':min(distances)}
        return ret

    def get_coordinates(self):
        pass

    def set_precision_style(self, style):
        if style in self.styles:
            self.style=style
        else:
            raise ValueError("No such style")

class GetRMSD(object):
    """This object can be added to output_objects in (for example in the ReplicaExchange0 macro).
    It will load a frame from an rmf file and compute the rmsd of a hierarchy with respect to that reference structure.
    The output can be parsed from the stat.*.out files.
    """
    import math

    def __init__(self, hier, rmf_file_name, rmf_frame_index, rmf_state_index, label):
        """Constructor.
           @param hier hierarchy of the structure
           @param rmf_file_name path to the rmf file containing the reference structure
           @param rmf_frame_index the index of the frame containing the reference structure in the rmf file
           @param voxel rmf_state_index the index of the state containing the reference structure in the rmf file
           @param label label that will be used when parsing the stat file
        """

        rmf_fh = RMF.open_rmf_file_read_only(rmf_file_name)
        hh = IMP.rmf.create_hierarchies(rmf_fh, hier.get_model())
        self.rmf_state = hh[0].get_children()[rmf_state_index]
        self.current_state = hier.get_children()[rmf_state_index]
        IMP.rmf.load_frame(rmf_fh, RMF.FrameID(rmf_frame_index))
        self.current_moldict = {}
        self.rmf_moldict = {}
        for mol in self.rmf_state.get_children():
            name = mol.get_name()
            if name in self.rmf_moldict:
                self.rmf_moldict[name].append(mol)
            else:
                self.rmf_moldict[name] = [mol]
        for mol in self.current_state.get_children():
            name = mol.get_name()
            if name in self.current_moldict:
                self.current_moldict[name].append(mol)
            else:
                self.current_moldict[name] = [mol]
        self.label = label

    def get_output(self):
        total_rmsd = 0
        total_N = 0
        for molname, rmf_mols in self.rmf_moldict.iteritems():
            sel_rmf = IMP.atom.Selection(
                rmf_mols, representation_type=IMP.atom.BALLS)
            N = len(sel_rmf.get_selected_particle_indexes())
            total_N += N
            rmsd = []
            for current_mols in itertools.permutations(self.current_moldict[molname]):
                sel_current = IMP.atom.Selection(
                    current_mols, representation_type=IMP.atom.BALLS)
                rmsd.append(IMP.atom.get_rmsd(sel_rmf, sel_current))
            m = min(rmsd)
            total_rmsd += m * m * N
        return {self.label: "%e" % (self.math.sqrt(total_rmsd / total_N))}


class GetModelDensity(object):
    """Compute mean density maps from structures.

    Keeps a dictionary of density maps,
    keys are in the custom ranges. When you call add_subunits_density, it adds
    particle coordinates to the existing density maps.
    """

    def __init__(self, custom_ranges, representation=None, resolution=20.0, voxel=5.0):
        """Constructor.
           @param custom_ranges  Required. It's a dictionary, keys are the
                  density component names, values are selection tuples
                          e.g. {'kin28':[['kin28',1,-1]],
                               'density_name_1' :[('ccl1')],
                               'density_name_2' :[(1,142,'tfb3d1'),
                                                  (143,700,'tfb3d2')],
           @param representation PMI representation, for doing selections.
                          Not needed if you only pass hierarchies
           @param resolution The MRC resolution of the output map (in Angstrom unit)
           @param voxel The voxel size for the output map (lower is slower)
        """

        self.representation = representation
        self.MRCresolution = resolution
        self.voxel = voxel
        self.densities = {}
        self.count_models = 0.0
        self.custom_ranges = custom_ranges

    def add_subunits_density(self, hierarchy=None):
        """Add a frame to the densities.
        @param hierarchy Optionally read the hierarchy from somewhere.
                         If not passed, will just read the representation.
        """
        self.count_models += 1.0

        if hierarchy:
            part_dict = get_particles_at_resolution_one(hierarchy)
            all_particles_by_resolution = []
            for name in part_dict:
                all_particles_by_resolution += part_dict[name]

        for density_name in self.custom_ranges:
            parts = []
            if hierarchy:
                all_particles_by_segments = []

            for seg in self.custom_ranges[density_name]:
                if not hierarchy:
                    # when you have a IMP.pmi.representation.Representation class
                    parts += IMP.tools.select_by_tuple(self.representation,
                                                       seg, resolution=1, name_is_ambiguous=False)
                else:
                    # else, when you have a hierarchy, but not a representation
                    if not IMP.pmi.get_is_canonical(hierarchy):
                        for h in hierarchy.get_children():
                            if not IMP.atom.Molecule.get_is_setup(h):
                                IMP.atom.Molecule.setup_particle(h.get_particle())

                    if type(seg) == str:
                        s = IMP.atom.Selection(hierarchy,molecule=seg)
                    elif type(seg) == tuple:
                        s = IMP.atom.Selection(
                            hierarchy, molecule=seg[2],residue_indexes=range(seg[0], seg[1] + 1))
                    else:
                        raise Exception('could not understand selection tuple '+str(seg))

                    all_particles_by_segments += s.get_selected_particles()
            if hierarchy:
                if IMP.pmi.get_is_canonical(hierarchy):
                    parts = all_particles_by_segments
                else:
                    parts = list(
                        set(all_particles_by_segments) & set(all_particles_by_resolution))
            self._create_density_from_particles(parts, density_name)

    def normalize_density(self):
        pass

    def _create_density_from_particles(self, ps, name,
                                      kernel_type='GAUSSIAN'):
        '''Internal function for adding to densities.
        pass XYZR particles with mass and create a density from them.
        kernel type options are GAUSSIAN, BINARIZED_SPHERE, and SPHERE.'''
        kd = {
            'GAUSSIAN': IMP.em.GAUSSIAN,
            'BINARIZED_SPHERE': IMP.em.BINARIZED_SPHERE,
            'SPHERE': IMP.em.SPHERE}

        dmap = IMP.em.SampledDensityMap(ps, self.MRCresolution, self.voxel)
        dmap.calcRMS()
        dmap.set_was_used(True)
        if name not in self.densities:
            self.densities[name] = dmap
        else:
            bbox1 = IMP.em.get_bounding_box(self.densities[name])
            bbox2 = IMP.em.get_bounding_box(dmap)
            bbox1 += bbox2
            dmap3 = IMP.em.create_density_map(bbox1,self.voxel)
            dmap3.set_was_used(True)
            dmap3.add(dmap)
            dmap3.add(self.densities[name])
            self.densities[name] = dmap3

    def get_density_keys(self):
        return list(self.densities.keys())

    def get_density(self,name):
        """Get the current density for some component name"""
        if name not in self.densities:
            return None
        else:
            return self.densities[name]

    def write_mrc(self, path="./"):
        for density_name in self.densities:
            self.densities[density_name].multiply(1. / self.count_models)
            IMP.em.write_map(
                self.densities[density_name],
                path + "/" + density_name + ".mrc",
                IMP.em.MRCReaderWriter())


class GetContactMap(object):

    def __init__(self, distance=15.):
        self.distance = distance
        self.contactmap = ''
        self.namelist = []
        self.xlinks = 0
        self.XL = {}
        self.expanded = {}
        self.resmap = {}

    def set_prot(self, prot):
        from scipy.spatial.distance import cdist
        self.prot = prot
        self.protnames = []
        coords = []
        radii = []
        namelist = []

        particles_dictionary = get_particles_at_resolution_one(self.prot)

        for name in particles_dictionary:
            residue_indexes = []
            for p in particles_dictionary[name]:
                print(p.get_name())
                residue_indexes += IMP.pmi.tools.get_residue_indexes(p)
                #residue_indexes.add( )

            if len(residue_indexes) != 0:
                self.protnames.append(name)
                for res in range(min(residue_indexes), max(residue_indexes) + 1):
                    d = IMP.core.XYZR(p)
                    new_name = name + ":" + str(res)
                    if name not in self.resmap:
                        self.resmap[name] = {}
                    if res not in self.resmap:
                        self.resmap[name][res] = {}

                    self.resmap[name][res] = new_name
                    namelist.append(new_name)

                    crd = np.array([d.get_x(), d.get_y(), d.get_z()])
                    coords.append(crd)
                    radii.append(d.get_radius())

        coords = np.array(coords)
        radii = np.array(radii)

        if len(self.namelist) == 0:
            self.namelist = namelist
            self.contactmap = np.zeros((len(coords), len(coords)))

        distances = cdist(coords, coords)
        distances = (distances - radii).T - radii
        distances = distances <= self.distance

        print(coords)
        print(radii)
        print(distances)

        self.contactmap += distances

    def get_subunit_coords(self, frame, align=0):
        from scipy.spatial.distance import cdist
        coords = []
        radii = []
        namelist = []
        test, testr = [], []
        for part in self.prot.get_children():
            SortedSegments = []
            print(part)
            for chl in part.get_children():
                start = IMP.atom.get_leaves(chl)[0]
                end = IMP.atom.get_leaves(chl)[-1]

                startres = IMP.atom.Fragment(start).get_residue_indexes()[0]
                endres = IMP.atom.Fragment(end).get_residue_indexes()[-1]
                SortedSegments.append((chl, startres))
            SortedSegments = sorted(SortedSegments, key=itemgetter(1))

            for sgmnt in SortedSegments:
                for leaf in IMP.atom.get_leaves(sgmnt[0]):
                    p = IMP.core.XYZR(leaf)
                    crd = np.array([p.get_x(), p.get_y(), p.get_z()])

                    coords.append(crd)
                    radii.append(p.get_radius())

                    new_name = part.get_name() + '_' + sgmnt[0].get_name() +\
                        '_' + \
                        str(IMP.atom.Fragment(leaf)
                            .get_residue_indexes()[0])
                    namelist.append(new_name)
                    self.expanded[new_name] = len(
                        IMP.atom.Fragment(leaf).get_residue_indexes())
                    if part.get_name() not in self.resmap:
                        self.resmap[part.get_name()] = {}
                    for res in IMP.atom.Fragment(leaf).get_residue_indexes():
                        self.resmap[part.get_name()][res] = new_name

        coords = np.array(coords)
        radii = np.array(radii)
        if len(self.namelist) == 0:
            self.namelist = namelist
            self.contactmap = np.zeros((len(coords), len(coords)))
        distances = cdist(coords, coords)
        distances = (distances - radii).T - radii
        distances = distances <= self.distance
        self.contactmap += distances

    def add_xlinks(
        self,
        filname,
            identification_string='ISDCrossLinkMS_Distance_'):
        # 'ISDCrossLinkMS_Distance_interrb_6629-State:0-20:RPS30_218:eIF3j-1-1-0.1_None'
        self.xlinks = 1
        data = open(filname)
        D = data.readlines()
        data.close()

        for d in D:
            if identification_string in d:
                d = d.replace(
                    "_",
                    " ").replace("-",
                                 " ").replace(":",
                                              " ").split()

                t1, t2 = (d[0], d[1]), (d[1], d[0])
                if t1 not in self.XL:
                    self.XL[t1] = [(int(d[2]) + 1, int(d[3]) + 1)]
                    self.XL[t2] = [(int(d[3]) + 1, int(d[2]) + 1)]
                else:
                    self.XL[t1].append((int(d[2]) + 1, int(d[3]) + 1))
                    self.XL[t2].append((int(d[3]) + 1, int(d[2]) + 1))

    def dist_matrix(self, skip_cmap=0, skip_xl=1):
        K = self.namelist
        M = self.contactmap
        C, R = [], []
        L = sum(self.expanded.values())
        proteins = self.protnames

        # exp new
        if skip_cmap == 0:
            Matrices = {}
            proteins = [p.get_name() for p in self.prot.get_children()]
            missing = []
            for p1 in range(len(proteins)):
                for p2 in range(p1, len(proteins)):
                    pl1, pl2 = max(
                        self.resmap[proteins[p1]].keys()), max(self.resmap[proteins[p2]].keys())
                    pn1, pn2 = proteins[p1], proteins[p2]
                    mtr = np.zeros((pl1 + 1, pl2 + 1))
                    print('Creating matrix for: ', p1, p2, pn1, pn2, mtr.shape, pl1, pl2)
                    for i1 in range(1, pl1 + 1):
                        for i2 in range(1, pl2 + 1):
                            try:
                                r1 = K.index(self.resmap[pn1][i1])
                                r2 = K.index(self.resmap[pn2][i2])
                                r = M[r1, r2]
                                mtr[i1 - 1, i2 - 1] = r
                            except KeyError:
                                missing.append((pn1, pn2, i1, i2))
                                pass
                    Matrices[(pn1, pn2)] = mtr

        # add cross-links
        if skip_xl == 0:
            if self.XL == {}:
                raise ValueError("cross-links were not provided, use add_xlinks function!")
            Matrices_xl = {}
            missing_xl = []
            for p1 in range(len(proteins)):
                for p2 in range(p1, len(proteins)):
                    pl1, pl2 = max(
                        self.resmap[proteins[p1]].keys()), max(self.resmap[proteins[p2]].keys())
                    pn1, pn2 = proteins[p1], proteins[p2]
                    mtr = np.zeros((pl1 + 1, pl2 + 1))
                    flg = 0
                    try:
                        xls = self.XL[(pn1, pn2)]
                    except KeyError:
                        try:
                            xls = self.XL[(pn2, pn1)]
                            flg = 1
                        except KeyError:
                            flg = 2
                    if flg == 0:
                        print('Creating matrix for: ', p1, p2, pn1, pn2, mtr.shape, pl1, pl2)
                        for xl1, xl2 in xls:
                            if xl1 > pl1:
                                print('X' * 10, xl1, xl2)
                                xl1 = pl1
                            if xl2 > pl2:
                                print('X' * 10, xl1, xl2)
                                xl2 = pl2
                            mtr[xl1 - 1, xl2 - 1] = 100
                    elif flg == 1:
                        print('Creating matrix for: ', p1, p2, pn1, pn2, mtr.shape, pl1, pl2)
                        for xl1, xl2 in xls:
                            if xl1 > pl1:
                                print('X' * 10, xl1, xl2)
                                xl1 = pl1
                            if xl2 > pl2:
                                print('X' * 10, xl1, xl2)
                                xl2 = pl2
                            mtr[xl2 - 1, xl1 - 1] = 100
                    else:
                        raise RuntimeError('WTF!')
                    Matrices_xl[(pn1, pn2)] = mtr

        # expand the matrix to individual residues
        #NewM = []
        # for x1 in xrange(len(K)):
        #    lst = []
        #    for x2 in xrange(len(K)):
        #        lst += [M[x1,x2]]*self.expanded[K[x2]]
        #    for i in xrange(self.expanded[K[x1]]): NewM.append(np.array(lst))
        #NewM = np.array(NewM)

        # make list of protein names and create coordinate lists
        C = proteins
        # W is the component length list,
        # R is the contiguous coordinates list
        W, R = [], []
        for i, c in enumerate(C):
            cl = max(self.resmap[c].keys())
            W.append(cl)
            if i == 0:
                R.append(cl)
            else:
                R.append(R[-1] + cl)

        # start plotting
        if filename:
            # Don't require a display
            import matplotlib as mpl
            mpl.use('Agg')
        import matplotlib.pyplot as plt
        import matplotlib.gridspec as gridspec
        import scipy.sparse as sparse

        f = plt.figure()
        gs = gridspec.GridSpec(len(W), len(W),
                               width_ratios=W,
                               height_ratios=W)

        cnt = 0
        for x1, r1 in enumerate(R):
            if x1 == 0:
                s1 = 0
            else:
                s1 = R[x1 - 1]
            for x2, r2 in enumerate(R):
                if x2 == 0:
                    s2 = 0
                else:
                    s2 = R[x2 - 1]

                ax = plt.subplot(gs[cnt])
                if skip_cmap == 0:
                    try:
                        mtr = Matrices[(C[x1], C[x2])]
                    except KeyError:
                        mtr = Matrices[(C[x2], C[x1])].T
                    #cax = ax.imshow(log(NewM[s1:r1,s2:r2] / 1.), interpolation='nearest', vmin=0., vmax=log(NewM.max()))
                    cax = ax.imshow(
                        log(mtr),
                        interpolation='nearest',
                        vmin=0.,
                        vmax=log(NewM.max()))
                    ax.set_xticks([])
                    ax.set_yticks([])
                if skip_xl == 0:
                    try:
                        mtr = Matrices_xl[(C[x1], C[x2])]
                    except KeyError:
                        mtr = Matrices_xl[(C[x2], C[x1])].T
                    cax = ax.spy(
                        sparse.csr_matrix(mtr),
                        markersize=10,
                        color='white',
                        linewidth=100,
                        alpha=0.5)
                    ax.set_xticks([])
                    ax.set_yticks([])

                cnt += 1
                if x2 == 0:
                    ax.set_ylabel(C[x1], rotation=90)
        plt.show()


# ------------------------------------------------------------------
# a few random tools

def get_hiers_from_rmf(model, frame_number, rmf_file):
    # I have to deprecate this function
    print("getting coordinates for frame %i rmf file %s" % (frame_number, rmf_file))

    # load the frame
    rh = RMF.open_rmf_file_read_only(rmf_file)

    try:
        prots = IMP.rmf.create_hierarchies(rh, model)
    except IOError:
        print("Unable to open rmf file %s" % (rmf_file))
        return None
    #IMP.rmf.link_hierarchies(rh, prots)

    try:
        IMP.rmf.load_frame(rh, RMF.FrameID(frame_number))
    except IOError:
        print("Unable to open frame %i of file %s" % (frame_number, rmf_file))
        return None
    model.update()
    del rh

    return prots

def link_hiers_to_rmf(model,hiers,frame_number, rmf_file):
    print("linking hierarchies for frame %i rmf file %s" % (frame_number, rmf_file))
    rh = RMF.open_rmf_file_read_only(rmf_file)
    IMP.rmf.link_hierarchies(rh, hiers)
    try:
        IMP.rmf.load_frame(rh, RMF.FrameID(frame_number))
    except:
        print("Unable to open frame %i of file %s" % (frame_number, rmf_file))
        return False
    model.update()
    del rh
    return True


def get_hiers_and_restraints_from_rmf(model, frame_number, rmf_file):
    # I have to deprecate this function
    print("getting coordinates for frame %i rmf file %s" % (frame_number, rmf_file))

    # load the frame
    rh = RMF.open_rmf_file_read_only(rmf_file)

    try:
        prots = IMP.rmf.create_hierarchies(rh, model)
        rs = IMP.rmf.create_restraints(rh, model)
    except:
        print("Unable to open rmf file %s" % (rmf_file))
        return None,None
    try:
        IMP.rmf.load_frame(rh, RMF.FrameID(frame_number))
    except:
        print("Unable to open frame %i of file %s" % (frame_number, rmf_file))
        return None,None
    model.update()
    del rh

    return prots,rs

def link_hiers_and_restraints_to_rmf(model,hiers,rs, frame_number, rmf_file):
    print("linking hierarchies for frame %i rmf file %s" % (frame_number, rmf_file))
    rh = RMF.open_rmf_file_read_only(rmf_file)
    IMP.rmf.link_hierarchies(rh, hiers)
    IMP.rmf.link_restraints(rh, rs)
    try:
        IMP.rmf.load_frame(rh, RMF.FrameID(frame_number))
    except:
        print("Unable to open frame %i of file %s" % (frame_number, rmf_file))
        return False
    model.update()
    del rh
    return True


def get_hiers_from_rmf(model, frame_number, rmf_file):
    print("getting coordinates for frame %i rmf file %s" % (frame_number, rmf_file))

    # load the frame
    rh = RMF.open_rmf_file_read_only(rmf_file)

    try:
        prots = IMP.rmf.create_hierarchies(rh, model)
    except:
        print("Unable to open rmf file %s" % (rmf_file))
        prot = None
        return prot
    #IMP.rmf.link_hierarchies(rh, prots)
    try:
        IMP.rmf.load_frame(rh, RMF.FrameID(frame_number))
    except:
        print("Unable to open frame %i of file %s" % (frame_number, rmf_file))
        prots = None
    model.update()
    del rh
    return prots


def get_particles_at_resolution_one(prot):
    """Get particles at res 1, or any beads, based on the name.
    No Representation is needed. This is mainly used when the hierarchy
    is read from an RMF file.
    @return a dictionary of component names and their particles
    \note If the root node is named "System" or is a "State", do proper selection.
    """
    particle_dict = {}

    # attempt to give good results for PMI2
    if IMP.pmi.get_is_canonical(prot):
        for mol in IMP.atom.get_by_type(prot,IMP.atom.MOLECULE_TYPE):
            sel = IMP.atom.Selection(mol,resolution=1)
            particle_dict[mol.get_name()] = sel.get_selected_particles()
    else:
        allparticles = []
        for c in prot.get_children():
            name = c.get_name()
            particle_dict[name] = IMP.atom.get_leaves(c)
            for s in c.get_children():
                if "_Res:1" in s.get_name() and "_Res:10" not in s.get_name():
                    allparticles += IMP.atom.get_leaves(s)
                if "Beads" in s.get_name():
                    allparticles += IMP.atom.get_leaves(s)

        particle_align = []
        for name in particle_dict:
            particle_dict[name] = IMP.pmi.tools.sort_by_residues(
                list(set(particle_dict[name]) & set(allparticles)))
    return particle_dict

def get_particles_at_resolution_ten(prot):
    """Get particles at res 10, or any beads, based on the name.
    No Representation is needed.
    This is mainly used when the hierarchy is read from an RMF file.
    @return a dictionary of component names and their particles
    \note If the root node is named "System" or is a "State", do proper selection.
    """
    particle_dict = {}
    # attempt to give good results for PMI2
    if IMP.pmi.get_is_canonical(prot):
        for mol in IMP.atom.get_by_type(prot,IMP.atom.MOLECULE_TYPE):
            sel = IMP.atom.Selection(mol,resolution=10)
            particle_dict[mol.get_name()] = sel.get_selected_particles()
    else:
        allparticles = []
        for c in prot.get_children():
            name = c.get_name()
            particle_dict[name] = IMP.atom.get_leaves(c)
            for s in c.get_children():
                if "_Res:10" in s.get_name():
                    allparticles += IMP.atom.get_leaves(s)
                if "Beads" in s.get_name():
                    allparticles += IMP.atom.get_leaves(s)
        particle_align = []
        for name in particle_dict:
            particle_dict[name] = IMP.pmi.tools.sort_by_residues(
                list(set(particle_dict[name]) & set(allparticles)))
    return particle_dict

def select_by_tuple(first_res_last_res_name_tuple):
    first_res = first_res_last_res_hier_tuple[0]
    last_res = first_res_last_res_hier_tuple[1]
    name = first_res_last_res_hier_tuple[2]

class CrossLinkTable(object):
    """Visualization of crosslinks"""
    def __init__(self):
        self.crosslinks = []
        self.external_csv_data = None
        self.crosslinkedprots = set()
        self.mindist = +10000000.0
        self.maxdist = -10000000.0
        self.contactmap = None

    def set_hierarchy(self, prot):
        self.prot_length_dict = {}
        self.model=prot.get_model()

        for i in prot.get_children():
            name = i.get_name()
            residue_indexes = []
            for p in IMP.atom.get_leaves(i):
                residue_indexes += IMP.pmi.tools.get_residue_indexes(p)

            if len(residue_indexes) != 0:
                self.prot_length_dict[name] = max(residue_indexes)

    def set_coordinates_for_contact_map(self, rmf_name,rmf_frame_index):
        from scipy.spatial.distance import cdist

        rh= RMF.open_rmf_file_read_only(rmf_name)
        prots=IMP.rmf.create_hierarchies(rh, self.model)
        IMP.rmf.load_frame(rh, RMF.FrameID(rmf_frame_index))
        print("getting coordinates for frame %i rmf file %s" % (rmf_frame_index, rmf_name))
        del rh


        coords = []
        radii = []
        namelist = []

        particles_dictionary = get_particles_at_resolution_one(prots[0])

        resindex = 0
        self.index_dictionary = {}

        for name in particles_dictionary:
            residue_indexes = []
            for p in particles_dictionary[name]:
                print(p.get_name())
                residue_indexes = IMP.pmi.tools.get_residue_indexes(p)
                #residue_indexes.add( )

                if len(residue_indexes) != 0:

                    for res in range(min(residue_indexes), max(residue_indexes) + 1):
                        d = IMP.core.XYZR(p)

                        crd = np.array([d.get_x(), d.get_y(), d.get_z()])
                        coords.append(crd)
                        radii.append(d.get_radius())
                        if name not in self.index_dictionary:
                            self.index_dictionary[name] = [resindex]
                        else:
                            self.index_dictionary[name].append(resindex)
                        resindex += 1

        coords = np.array(coords)
        radii = np.array(radii)

        distances = cdist(coords, coords)
        distances = (distances - radii).T - radii

        distances = np.where(distances <= 20.0, 1.0, 0)
        if self.contactmap is None:
            self.contactmap = np.zeros((len(coords), len(coords)))
        self.contactmap += distances

        for prot in prots: IMP.atom.destroy(prot)

    def set_crosslinks(
        self, data_file, search_label='ISDCrossLinkMS_Distance_',
        mapping=None,
        filter_label=None,
        filter_rmf_file_names=None, #provide a list of rmf base names to filter the stat file
        external_csv_data_file=None,
        external_csv_data_file_unique_id_key="Unique ID"):

        # example key ISDCrossLinkMS_Distance_intrarb_937-State:0-108:RPS3_55:RPS30-1-1-0.1_None
        # mapping is a dictionary that maps standard keywords to entry positions in the key string
        # confidence class is a filter that
        # external datafile is a datafile that contains further information on the crosslinks
        # it will use the unique id to create the dictionary keys

        po = IMP.pmi.output.ProcessOutput(data_file)
        keys = po.get_keys()

        xl_keys = [k for k in keys if search_label in k]

        if filter_rmf_file_names is not None:
            rmf_file_key="local_rmf_file_name"
            fs = po.get_fields(xl_keys+[rmf_file_key])
        else:
            fs = po.get_fields(xl_keys)

        # this dictionary stores the occurency of given crosslinks
        self.cross_link_frequency = {}

        # this dictionary stores the series of distances for given crosslinked
        # residues
        self.cross_link_distances = {}

        # this dictionary stores the series of distances for given crosslinked
        # residues
        self.cross_link_distances_unique = {}

        if not external_csv_data_file is None:
            # this dictionary stores the further information on crosslinks
            # labeled by unique ID
            self.external_csv_data = {}
            xldb = IMP.pmi.tools.get_db_from_csv(external_csv_data_file)

            for xl in xldb:
                self.external_csv_data[
                    xl[external_csv_data_file_unique_id_key]] = xl

        # this list keeps track the tuple of cross-links and sample
        # so that we don't count twice the same crosslinked residues in the
        # same sample
        cross_link_frequency_list = []

        self.unique_cross_link_list = []

        for key in xl_keys:
            print(key)
            keysplit = key.replace(
                "_",
                " ").replace(
                "-",
                " ").replace(
                ":",
                " ").split(
            )

            if filter_label!=None:
                if filter_label not in keysplit: continue

            if mapping is None:
                r1 = int(keysplit[5])
                c1 = keysplit[6]
                r2 = int(keysplit[7])
                c2 = keysplit[8]
                try:
                    confidence = keysplit[12]
                except:
                    confidence = '0.0'
                try:
                    unique_identifier = keysplit[3]
                except:
                    unique_identifier = '0'
            else:
                r1 = int(keysplit[mapping["Residue1"]])
                c1 = keysplit[mapping["Protein1"]]
                r2 = int(keysplit[mapping["Residue2"]])
                c2 = keysplit[mapping["Protein2"]]
                try:
                    confidence = keysplit[mapping["Confidence"]]
                except:
                    confidence = '0.0'
                try:
                    unique_identifier = keysplit[mapping["Unique Identifier"]]
                except:
                    unique_identifier = '0'

            self.crosslinkedprots.add(c1)
            self.crosslinkedprots.add(c2)

            # construct the list of distances corresponding to the input rmf
            # files

            dists=[]
            if filter_rmf_file_names is not None:
                for n,d in enumerate(fs[key]):
                    if fs[rmf_file_key][n] in filter_rmf_file_names:
                        dists.append(float(d))
            else:
                dists=[float(f) for f in fs[key]]

            # check if the input confidence class corresponds to the
            # one of the cross-link

            mdist = self.median(dists)

            stdv = np.std(np.array(dists))
            if self.mindist > mdist:
                self.mindist = mdist
            if self.maxdist < mdist:
                self.maxdist = mdist

            # calculate the frequency of unique crosslinks within the same
            # sample
            if not self.external_csv_data is None:
                sample = self.external_csv_data[unique_identifier]["Sample"]
            else:
                sample = "None"

            if (r1, c1, r2, c2,mdist) not in cross_link_frequency_list:
                if (r1, c1, r2, c2) not in self.cross_link_frequency:
                    self.cross_link_frequency[(r1, c1, r2, c2)] = 1
                    self.cross_link_frequency[(r2, c2, r1, c1)] = 1
                else:
                    self.cross_link_frequency[(r2, c2, r1, c1)] += 1
                    self.cross_link_frequency[(r1, c1, r2, c2)] += 1
                cross_link_frequency_list.append((r1, c1, r2, c2))
                cross_link_frequency_list.append((r2, c2, r1, c1))
                self.unique_cross_link_list.append(
                    (r1, c1, r2, c2,mdist))

            if (r1, c1, r2, c2) not in self.cross_link_distances:
                self.cross_link_distances[(
                    r1,
                    c1,
                    r2,
                    c2,
                    mdist,
                    confidence)] = dists
                self.cross_link_distances[(
                    r2,
                    c2,
                    r1,
                    c1,
                    mdist,
                    confidence)] = dists
                self.cross_link_distances_unique[(r1, c1, r2, c2)] = dists
            else:
                self.cross_link_distances[(
                    r2,
                    c2,
                    r1,
                    c1,
                    mdist,
                    confidence)] += dists
                self.cross_link_distances[(
                    r1,
                    c1,
                    r2,
                    c2,
                    mdist,
                    confidence)] += dists

            self.crosslinks.append(
                (r1,
                 c1,
                 r2,
                 c2,
                 mdist,
                 stdv,
                 confidence,
                 unique_identifier,
                 'original'))
            self.crosslinks.append(
                (r2,
                 c2,
                 r1,
                 c1,
                 mdist,
                 stdv,
                 confidence,
                 unique_identifier,
                 'reversed'))

        self.cross_link_frequency_inverted = {}
        for xl in self.unique_cross_link_list:
            (r1, c1, r2, c2, mdist) = xl
            frequency = self.cross_link_frequency[(r1, c1, r2, c2)]
            if frequency not in self.cross_link_frequency_inverted:
                self.cross_link_frequency_inverted[
                    frequency] = [(r1, c1, r2, c2)]
            else:
                self.cross_link_frequency_inverted[
                    frequency].append((r1, c1, r2, c2))

        # -------------

    def median(self, mylist):
        sorts = sorted(mylist)
        length = len(sorts)
        print(length)
        if length == 1:
            return mylist[0]
        if not length % 2:
            return (sorts[length / 2] + sorts[length / 2 - 1]) / 2.0
        return sorts[length / 2]

    def set_threshold(self,threshold):
        self.threshold=threshold

    def set_tolerance(self,tolerance):
        self.tolerance=tolerance

    def colormap(self, dist):
        if dist < self.threshold - self.tolerance:
            return "Green"
        elif dist >= self.threshold + self.tolerance:
            return "Orange"
        else:
            return "Red"

    def write_cross_link_database(self, filename, format='csv'):
        import csv

        fieldnames = [
            "Unique ID", "Protein1", "Residue1", "Protein2", "Residue2",
            "Median Distance", "Standard Deviation", "Confidence", "Frequency", "Arrangement"]

        if not self.external_csv_data is None:
            keys = list(self.external_csv_data.keys())
            innerkeys = list(self.external_csv_data[keys[0]].keys())
            innerkeys.sort()
            fieldnames += innerkeys

        dw = csv.DictWriter(
            open(filename,
                 "w"),
            delimiter=',',
            fieldnames=fieldnames)
        dw.writeheader()
        for xl in self.crosslinks:
            (r1, c1, r2, c2, mdist, stdv, confidence,
             unique_identifier, descriptor) = xl
            if descriptor == 'original':
                outdict = {}
                outdict["Unique ID"] = unique_identifier
                outdict["Protein1"] = c1
                outdict["Protein2"] = c2
                outdict["Residue1"] = r1
                outdict["Residue2"] = r2
                outdict["Median Distance"] = mdist
                outdict["Standard Deviation"] = stdv
                outdict["Confidence"] = confidence
                outdict["Frequency"] = self.cross_link_frequency[
                    (r1, c1, r2, c2)]
                if c1 == c2:
                    arrangement = "Intra"
                else:
                    arrangement = "Inter"
                outdict["Arrangement"] = arrangement
                if not self.external_csv_data is None:
                    outdict.update(self.external_csv_data[unique_identifier])

                dw.writerow(outdict)

    def plot(self, prot_listx=None, prot_listy=None, no_dist_info=False,
             no_confidence_info=False, filter=None, layout="whole", crosslinkedonly=False,
             filename=None, confidence_classes=None, alphablend=0.1, scale_symbol_size=1.0,
             gap_between_components=0,
             rename_protein_map=None):
        # layout can be:
        #                "lowerdiagonal"  print only the lower diagonal plot
        #                "upperdiagonal"  print only the upper diagonal plot
        #                "whole"  print all
        # crosslinkedonly: plot only components that have crosslinks
        # no_dist_info: if True will plot only the cross-links as grey spots
        # filter = tuple the tuple contains a keyword to be search in the database
        #                a relationship ">","==","<"
        #                and a value
        #                example ("ID_Score",">",40)
        # scale_symbol_size rescale the symbol for the crosslink
        # rename_protein_map is a dictionary to rename proteins

        import matplotlib as mpl
        mpl.use('Agg')
        import matplotlib.pyplot as plt
        import matplotlib.cm as cm

        fig = plt.figure(figsize=(10, 10))
        ax = fig.add_subplot(111)

        ax.set_xticks([])
        ax.set_yticks([])

        # set the list of proteins on the x axis
        if prot_listx is None:
            if crosslinkedonly:
                prot_listx = list(self.crosslinkedprots)
            else:
                prot_listx = list(self.prot_length_dict.keys())
            prot_listx.sort()

        nresx = gap_between_components + \
            sum([self.prot_length_dict[name]
                + gap_between_components for name in prot_listx])

        # set the list of proteins on the y axis

        if prot_listy is None:
            if crosslinkedonly:
                prot_listy = list(self.crosslinkedprots)
            else:
                prot_listy = list(self.prot_length_dict.keys())
            prot_listy.sort()

        nresy = gap_between_components + \
            sum([self.prot_length_dict[name]
                + gap_between_components for name in prot_listy])

        # this is the residue offset for each protein
        resoffsetx = {}
        resendx = {}
        res = gap_between_components
        for prot in prot_listx:
            resoffsetx[prot] = res
            res += self.prot_length_dict[prot]
            resendx[prot] = res
            res += gap_between_components

        resoffsety = {}
        resendy = {}
        res = gap_between_components
        for prot in prot_listy:
            resoffsety[prot] = res
            res += self.prot_length_dict[prot]
            resendy[prot] = res
            res += gap_between_components

        resoffsetdiagonal = {}
        res = gap_between_components
        for prot in IMP.pmi.tools.OrderedSet(prot_listx + prot_listy):
            resoffsetdiagonal[prot] = res
            res += self.prot_length_dict[prot]
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
                ax.plot([res, res], [resy, endy], 'k-', lw=0.4)
                ax.plot([end, end], [resy, endy], 'k-', lw=0.4)
            xticks.append((float(res) + float(end)) / 2)
            if rename_protein_map is not None:
                if prot in rename_protein_map:
                    prot=rename_protein_map[prot]
            xlabels.append(prot)

        yticks = []
        ylabels = []
        for n, prot in enumerate(prot_listy):
            res = resoffsety[prot]
            end = resendy[prot]
            for protx in prot_listx:
                resx = resoffsetx[protx]
                endx = resendx[protx]
                ax.plot([resx, endx], [res, res], 'k-', lw=0.4)
                ax.plot([resx, endx], [end, end], 'k-', lw=0.4)
            yticks.append((float(res) + float(end)) / 2)
            if rename_protein_map is not None:
                if prot in rename_protein_map:
                    prot=rename_protein_map[prot]
            ylabels.append(prot)

        # plot the contact map
        print(prot_listx, prot_listy)

        if not self.contactmap is None:
            import matplotlib.cm as cm
            tmp_array = np.zeros((nresx, nresy))

            for px in prot_listx:
                print(px)
                for py in prot_listy:
                    print(py)
                    resx = resoffsety[px]
                    lengx = resendx[px] - 1
                    resy = resoffsety[py]
                    lengy = resendy[py] - 1
                    indexes_x = self.index_dictionary[px]
                    minx = min(indexes_x)
                    maxx = max(indexes_x)
                    indexes_y = self.index_dictionary[py]
                    miny = min(indexes_y)
                    maxy = max(indexes_y)

                    print(px, py, minx, maxx, miny, maxy)

                    try:
                        tmp_array[
                            resx:lengx,
                            resy:lengy] = self.contactmap[
                            minx:maxx,
                            miny:maxy]
                    except:
                        continue


            ax.imshow(tmp_array,
                      cmap=cm.binary,
                      origin='lower',
                      interpolation='nearest')

        ax.set_xticks(xticks)
        ax.set_xticklabels(xlabels, rotation=90)
        ax.set_yticks(yticks)
        ax.set_yticklabels(ylabels)
        ax.set_xlim(0,nresx)
        ax.set_ylim(0,nresy)


        # set the crosslinks

        already_added_xls = []

        for xl in self.crosslinks:

            (r1, c1, r2, c2, mdist, stdv, confidence,
             unique_identifier, descriptor) = xl

            if confidence_classes is not None:
                if confidence not in confidence_classes:
                    continue

            try:
                pos1 = r1 + resoffsetx[c1]
            except:
                continue
            try:
                pos2 = r2 + resoffsety[c2]
            except:
                continue

            if not filter is None:
                xldb = self.external_csv_data[unique_identifier]
                xldb.update({"Distance": mdist})
                xldb.update({"Distance_stdv": stdv})

                if filter[1] == ">":
                    if float(xldb[filter[0]]) <= float(filter[2]):
                        continue

                if filter[1] == "<":
                    if float(xldb[filter[0]]) >= float(filter[2]):
                        continue

                if filter[1] == "==":
                    if float(xldb[filter[0]]) != float(filter[2]):
                        continue

            # all that below is used for plotting the diagonal
            # when you have a rectangolar plots

            pos_for_diagonal1 = r1 + resoffsetdiagonal[c1]
            pos_for_diagonal2 = r2 + resoffsetdiagonal[c2]

            if layout == 'lowerdiagonal':
                if pos_for_diagonal1 <= pos_for_diagonal2:
                    continue
            if layout == 'upperdiagonal':
                if pos_for_diagonal1 >= pos_for_diagonal2:
                    continue

            already_added_xls.append((r1, c1, r2, c2))

            if not no_confidence_info:
                if confidence == '0.01':
                    markersize = 14 * scale_symbol_size
                elif confidence == '0.05':
                    markersize = 9 * scale_symbol_size
                elif confidence == '0.1':
                    markersize = 6 * scale_symbol_size
                else:
                    markersize = 15 * scale_symbol_size
            else:
                markersize = 5 * scale_symbol_size

            if not no_dist_info:
                color = self.colormap(mdist)
            else:
                color = "gray"

            ax.plot(
                [pos1],
                [pos2],
                'o',
                c=color,
                alpha=alphablend,
                markersize=markersize)



        fig.set_size_inches(0.004 * nresx, 0.004 * nresy)

        [i.set_linewidth(2.0) for i in ax.spines.values()]

        #plt.tight_layout()

        if filename:
            plt.savefig(filename + ".pdf", dpi=300, transparent="False")
        else:
            plt.show()

    def get_frequency_statistics(self, prot_list,
                                 prot_list2=None):

        violated_histogram = {}
        satisfied_histogram = {}
        unique_cross_links = []

        for xl in self.unique_cross_link_list:
            (r1, c1, r2, c2, mdist) = xl

            # here we filter by the protein
            if prot_list2 is None:
                if not c1 in prot_list:
                    continue
                if not c2 in prot_list:
                    continue
            else:
                if c1 in prot_list and c2 in prot_list2:
                    pass
                elif c1 in prot_list2 and c2 in prot_list:
                    pass
                else:
                    continue

            frequency = self.cross_link_frequency[(r1, c1, r2, c2)]

            if (r1, c1, r2, c2) not in unique_cross_links:
                if mdist > 35.0:
                    if frequency not in violated_histogram:
                        violated_histogram[frequency] = 1
                    else:
                        violated_histogram[frequency] += 1
                else:
                    if frequency not in satisfied_histogram:
                        satisfied_histogram[frequency] = 1
                    else:
                        satisfied_histogram[frequency] += 1
                unique_cross_links.append((r1, c1, r2, c2))
                unique_cross_links.append((r2, c2, r1, c1))

        print("# satisfied")

        total_number_of_crosslinks=0

        for i in satisfied_histogram:
            # if i in violated_histogram:
            #   print i, satisfied_histogram[i]+violated_histogram[i]
            # else:
            if i in violated_histogram:
                print(i, violated_histogram[i]+satisfied_histogram[i])
            else:
                print(i, satisfied_histogram[i])
            total_number_of_crosslinks+=i*satisfied_histogram[i]

        print("# violated")

        for i in violated_histogram:
            print(i, violated_histogram[i])
            total_number_of_crosslinks+=i*violated_histogram[i]

        print(total_number_of_crosslinks)


# ------------
    def print_cross_link_binary_symbols(self, prot_list,
                                        prot_list2=None):
        tmp_matrix = []
        confidence_list = []
        for xl in self.crosslinks:
            (r1, c1, r2, c2, mdist, stdv, confidence,
             unique_identifier, descriptor) = xl

            if prot_list2 is None:
                if not c1 in prot_list:
                    continue
                if not c2 in prot_list:
                    continue
            else:
                if c1 in prot_list and c2 in prot_list2:
                    pass
                elif c1 in prot_list2 and c2 in prot_list:
                    pass
                else:
                    continue

            if descriptor != "original":
                continue

            confidence_list.append(confidence)

            dists = self.cross_link_distances_unique[(r1, c1, r2, c2)]
            tmp_dist_binary = []
            for d in dists:
                if d < 35:
                    tmp_dist_binary.append(1)
                else:
                    tmp_dist_binary.append(0)
            tmp_matrix.append(tmp_dist_binary)

        matrix = list(zip(*tmp_matrix))

        satisfied_high_sum = 0
        satisfied_mid_sum = 0
        satisfied_low_sum = 0
        total_satisfied_sum = 0
        for k, m in enumerate(matrix):
            satisfied_high = 0
            total_high = 0
            satisfied_mid = 0
            total_mid = 0
            satisfied_low = 0
            total_low = 0
            total_satisfied = 0
            total = 0
            for n, b in enumerate(m):
                if confidence_list[n] == "0.01":
                    total_high += 1
                    if b == 1:
                        satisfied_high += 1
                        satisfied_high_sum += 1
                elif confidence_list[n] == "0.05":
                    total_mid += 1
                    if b == 1:
                        satisfied_mid += 1
                        satisfied_mid_sum += 1
                elif confidence_list[n] == "0.1":
                    total_low += 1
                    if b == 1:
                        satisfied_low += 1
                        satisfied_low_sum += 1
                if b == 1:
                    total_satisfied += 1
                    total_satisfied_sum += 1
                total += 1
            print(k, satisfied_high, total_high)
            print(k, satisfied_mid, total_mid)
            print(k, satisfied_low, total_low)
            print(k, total_satisfied, total)
        print(float(satisfied_high_sum) / len(matrix))
        print(float(satisfied_mid_sum) / len(matrix))
        print(float(satisfied_low_sum) / len(matrix))
# ------------

    def get_unique_crosslinks_statistics(self, prot_list,
                                         prot_list2=None):

        print(prot_list)
        print(prot_list2)
        satisfied_high = 0
        total_high = 0
        satisfied_mid = 0
        total_mid = 0
        satisfied_low = 0
        total_low = 0
        total = 0
        tmp_matrix = []
        satisfied_string = []
        for xl in self.crosslinks:
            (r1, c1, r2, c2, mdist, stdv, confidence,
             unique_identifier, descriptor) = xl

            if prot_list2 is None:
                if not c1 in prot_list:
                    continue
                if not c2 in prot_list:
                    continue
            else:
                if c1 in prot_list and c2 in prot_list2:
                    pass
                elif c1 in prot_list2 and c2 in prot_list:
                    pass
                else:
                    continue

            if descriptor != "original":
                continue

            total += 1
            if confidence == "0.01":
                total_high += 1
                if mdist <= 35:
                    satisfied_high += 1
            if confidence == "0.05":
                total_mid += 1
                if mdist <= 35:
                    satisfied_mid += 1
            if confidence == "0.1":
                total_low += 1
                if mdist <= 35:
                    satisfied_low += 1
            if mdist <= 35:
                satisfied_string.append(1)
            else:
                satisfied_string.append(0)

            dists = self.cross_link_distances_unique[(r1, c1, r2, c2)]
            tmp_dist_binary = []
            for d in dists:
                if d < 35:
                    tmp_dist_binary.append(1)
                else:
                    tmp_dist_binary.append(0)
            tmp_matrix.append(tmp_dist_binary)

        print("unique satisfied_high/total_high", satisfied_high, "/", total_high)
        print("unique satisfied_mid/total_mid", satisfied_mid, "/", total_mid)
        print("unique satisfied_low/total_low", satisfied_low, "/", total_low)
        print("total", total)

        matrix = list(zip(*tmp_matrix))
        satisfied_models = 0
        satstr = ""
        for b in satisfied_string:
            if b == 0:
                satstr += "-"
            if b == 1:
                satstr += "*"

        for m in matrix:
            all_satisfied = True
            string = ""
            for n, b in enumerate(m):
                if b == 0:
                    string += "0"
                if b == 1:
                    string += "1"
                if b == 1 and satisfied_string[n] == 1:
                    pass
                elif b == 1 and satisfied_string[n] == 0:
                    pass
                elif b == 0 and satisfied_string[n] == 0:
                    pass
                elif b == 0 and satisfied_string[n] == 1:
                    all_satisfied = False
            if all_satisfied:
                satisfied_models += 1
            print(string)
            print(satstr, all_satisfied)
        print("models that satisfies the median satisfied crosslinks/total models", satisfied_models, len(matrix))

    def plot_matrix_cross_link_distances_unique(self, figurename, prot_list,
                                                prot_list2=None):

        import matplotlib as mpl
        mpl.use('Agg')
        import matplotlib.pylab as pl

        tmp_matrix = []
        for kw in self.cross_link_distances_unique:
            (r1, c1, r2, c2) = kw
            dists = self.cross_link_distances_unique[kw]

            if prot_list2 is None:
                if not c1 in prot_list:
                    continue
                if not c2 in prot_list:
                    continue
            else:
                if c1 in prot_list and c2 in prot_list2:
                    pass
                elif c1 in prot_list2 and c2 in prot_list:
                    pass
                else:
                    continue
            # append the sum of dists to order by that in the matrix plot
            dists.append(sum(dists))
            tmp_matrix.append(dists)

        tmp_matrix.sort(key=itemgetter(len(tmp_matrix[0]) - 1))

        # print len(tmp_matrix),  len(tmp_matrix[0])-1
        matrix = np.zeros((len(tmp_matrix), len(tmp_matrix[0]) - 1))

        for i in range(len(tmp_matrix)):
            for k in range(len(tmp_matrix[i]) - 1):
                matrix[i][k] = tmp_matrix[i][k]

        print(matrix)

        fig = pl.figure()
        ax = fig.add_subplot(211)

        cax = ax.imshow(matrix, interpolation='nearest')
        # ax.set_yticks(range(len(self.model_list_names)))
        #ax.set_yticklabels( [self.model_list_names[i] for i in leaves_order] )
        fig.colorbar(cax)
        pl.savefig(figurename, dpi=300)
        pl.show()

    def plot_bars(
        self,
        filename,
        prots1,
        prots2,
        nxl_per_row=20,
        arrangement="inter",
            confidence_input="None"):

        data = []
        for xl in self.cross_link_distances:
            (r1, c1, r2, c2, mdist, confidence) = xl
            if c1 in prots1 and c2 in prots2:
                if arrangement == "inter" and c1 == c2:
                    continue
                if arrangement == "intra" and c1 != c2:
                    continue
                if confidence_input == confidence:
                    label = str(c1) + ":" + str(r1) + \
                        "-" + str(c2) + ":" + str(r2)
                    values = self.cross_link_distances[xl]
                    frequency = self.cross_link_frequency[(r1, c1, r2, c2)]
                    data.append((label, values, mdist, frequency))

        sort_by_dist = sorted(data, key=lambda tup: tup[2])
        sort_by_dist = list(zip(*sort_by_dist))
        values = sort_by_dist[1]
        positions = list(range(len(values)))
        labels = sort_by_dist[0]
        frequencies = list(map(float, sort_by_dist[3]))
        frequencies = [f * 10.0 for f in frequencies]

        nchunks = int(float(len(values)) / nxl_per_row)
        values_chunks = IMP.pmi.tools.chunk_list_into_segments(values, nchunks)
        positions_chunks = IMP.pmi.tools.chunk_list_into_segments(
            positions,
            nchunks)
        frequencies_chunks = IMP.pmi.tools.chunk_list_into_segments(
            frequencies,
            nchunks)
        labels_chunks = IMP.pmi.tools.chunk_list_into_segments(labels, nchunks)

        for n, v in enumerate(values_chunks):
            p = positions_chunks[n]
            f = frequencies_chunks[n]
            l = labels_chunks[n]
            IMP.pmi.output.plot_fields_box_plots(
                filename + "." + str(n), v, p, f,
                valuename="Distance (Ang)", positionname="Unique " + arrangement + " Crosslinks", xlabels=l)

    def crosslink_distance_histogram(self, filename,
                                     prot_list=None,
                                     prot_list2=None,
                                     confidence_classes=None,
                                     bins=40,
                                     color='#66CCCC',
                                     yplotrange=[0, 1],
                                     format="png",
                                     normalized=False):
        if prot_list is None:
            prot_list = list(self.prot_length_dict.keys())

        distances = []
        for xl in self.crosslinks:
            (r1, c1, r2, c2, mdist, stdv, confidence,
             unique_identifier, descriptor) = xl

            if not confidence_classes is None:
                if confidence not in confidence_classes:
                    continue

            if prot_list2 is None:
                if not c1 in prot_list:
                    continue
                if not c2 in prot_list:
                    continue
            else:
                if c1 in prot_list and c2 in prot_list2:
                    pass
                elif c1 in prot_list2 and c2 in prot_list:
                    pass
                else:
                    continue

            distances.append(mdist)

        IMP.pmi.output.plot_field_histogram(
            filename, distances, valuename="C-alpha C-alpha distance [Ang]",
            bins=bins, color=color,
            format=format,
            reference_xline=35.0,
            yplotrange=yplotrange, normalized=normalized)

    def scatter_plot_xl_features(self, filename,
                                 feature1=None,
                                 feature2=None,
                                 prot_list=None,
                                 prot_list2=None,
                                 yplotrange=None,
                                 reference_ylines=None,
                                 distance_color=True,
                                 format="png"):
        import matplotlib as mpl
        mpl.use('Agg')
        import matplotlib.pyplot as plt
        import matplotlib.cm as cm

        fig = plt.figure(figsize=(10, 10))
        ax = fig.add_subplot(111)

        for xl in self.crosslinks:
            (r1, c1, r2, c2, mdist, stdv, confidence,
             unique_identifier, arrangement) = xl

            if prot_list2 is None:
                if not c1 in prot_list:
                    continue
                if not c2 in prot_list:
                    continue
            else:
                if c1 in prot_list and c2 in prot_list2:
                    pass
                elif c1 in prot_list2 and c2 in prot_list:
                    pass
                else:
                    continue

            xldb = self.external_csv_data[unique_identifier]
            xldb.update({"Distance": mdist})
            xldb.update({"Distance_stdv": stdv})

            xvalue = float(xldb[feature1])
            yvalue = float(xldb[feature2])

            if distance_color:
                color = self.colormap(mdist)
            else:
                color = "gray"

            ax.plot([xvalue], [yvalue], 'o', c=color, alpha=0.1, markersize=7)

        if not yplotrange is None:
            ax.set_ylim(yplotrange)
        if not reference_ylines is None:
            for rl in reference_ylines:
                ax.axhline(rl, color='red', linestyle='dashed', linewidth=1)

        if filename:
            plt.savefig(filename + "." + format, dpi=150, transparent="False")

        plt.show()
