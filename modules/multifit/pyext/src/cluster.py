#!/usr/bin/env python

__doc__ = "Cluster assembly solutions."

from optparse import OptionParser
import itertools
import math
import IMP.multifit
import IMP.container

def get_uniques(seq):
    # Not order preserving
    keys = {}
    counter=[]
    for e in seq:
        keys[e] = 1
    num_keys=len(keys.keys())
    for i in range(num_keys+1):
        counter.append([0,i])
    for e in seq:
        counter[e][0]=counter[e][0]+1
    counter=sorted(counter,reverse=True)
    indexes=[]
    for c,i in counter[:-1]:
        indexes.append(i)
    return indexes


class ClusterData:
    def __init__(self,cluster_ind,cluster_size,rmsd_calculated):
        self.cluster_ind=cluster_ind
        self.cluster_size=cluster_size
        self.rmsd_calculated=rmsd_calculated
    def set_distance_stats(self,avg,std):
        self.distance_avg=avg
        self.distance_std=std
    def set_angle_stats(self,avg,std):
        self.angle_avg=avg
        self.angle_std=std
    def set_rmsd_stats(self,avg,std):
        self.rmsd_avg=avg
        self.rmsd_std=std
    def set_best_sampled_data(self,ind,rmsd,cc,distance,angle):
        self.best_sampled_ind=ind
        self.best_sampled_rmsd=rmsd
        self.best_sampled_cc=cc
        self.best_sampled_distance=distance
        self.best_sampled_angle=angle
    def set_best_scored_data(self,ind,rmsd,cc,distance,angle):
        self.best_scored_ind=ind
        self.best_scored_rmsd=rmsd
        self.best_scored_cc=cc
        self.best_scored_distance=distance
        self.best_scored_angle=angle

class AlignmentClustering:
    """
        Clusters assembly models
        - The solutions are chosen by sorting the database according to the
          parameter orderby
        - The models are aligned and clustered by RMSD
    """
    def __init__(self, asmb_fn, prot_fn, map_fn, align_fn, combs_fn):
        self.asmb_fn = asmb_fn
        self.prot_fn = prot_fn
        self.map_fn = map_fn
        self.align_fn = align_fn
        self.combs_fn = combs_fn

        self.asmb=IMP.multifit.read_settings(self.asmb_fn)
        self.asmb.set_was_used(True)
        self.prot_data=IMP.multifit.read_proteomics_data(self.prot_fn)
        self.alignment_params = IMP.multifit.AlignmentParams(self.align_fn)
        self.mapping_data=IMP.multifit.read_protein_anchors_mapping(self.prot_data,self.map_fn)

        self.align=IMP.multifit.ProteomicsEMAlignmentAtomic(self.mapping_data,self.asmb,
                                                             self.alignment_params)

        self.combs=IMP.multifit.read_paths(self.combs_fn)
        self.ensmb=IMP.multifit.Ensemble(self.asmb,self.mapping_data)
        self.mhs=self.align.get_molecules()
        for i,mh in enumerate(self.mhs):
            self.ensmb.add_component_and_fits(mh,
                                      IMP.multifit.read_fitting_solutions(self.asmb.get_component_header(i).get_transformations_fn()))
        #load the density map
        self.dmap=IMP.em.read_map(self.asmb.get_assembly_header().get_dens_fn())
        self.dmap.get_header().set_resolution(
                         self.asmb.get_assembly_header().get_resolution())
        threshold=self.asmb.get_assembly_header().get_threshold()
        self.dmap.update_voxel_size(self.asmb.get_assembly_header().get_spacing())
        self.dmap.set_origin(self.asmb.get_assembly_header().get_origin())
        self.dmap.calcRMS()
    def do_clustering(self,max_comb_ind,max_rmsd):
        """
            Cluster configurations for a model based on RMSD.
            An IMP.ConfigurationSet is built using the reference frames for
            all of the components of the assembly for each solution
            @param max_comb_ind Maximum number of components to consider
            @param max_rmsd Maximum RMSD tolerated when clustering
        """
        import fastcluster
        import scipy.cluster.hierarchy

        self.mdl = self.align.get_model()
        self.all_ca=[]
        for mh in self.mhs:
            mh_res=IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE)
            s1=IMP.atom.Selection(mh_res);
            s1.set_atom_types([IMP.atom.AtomType("CA")])
            self.all_ca.append(s1.get_selected_particles())
        configuration_set = IMP.ConfigurationSet(self.mdl)
        #load configurations
        self.coords=[]
        print "load configurations"
        for combi,comb in enumerate(self.combs[:max_comb_ind]):
            self.ensmb.load_combination(comb)
            configuration_set.save_configuration()
            c1=[]
            for mol_ca in self.all_ca:
                mol_xyz=[]
                for ca in mol_ca:
                    mol_xyz.append(IMP.core.XYZ(ca).get_coordinates())
                c1.append(mol_xyz)
            self.coords.append(c1)
            self.ensmb.unload_combination(comb)
        self.distances=[]
        print "calculate distances"
        for i in range(len(self.coords)):
            for j in range(i+1,len(self.coords)):
                self.distances.append(IMP.atom.get_rmsd(list(itertools.chain.from_iterable(self.coords[i])),
                                                        list(itertools.chain.from_iterable(self.coords[j]))))
        print "cluster"
        Z=fastcluster.linkage(self.distances)
        self.cluster_inds=scipy.cluster.hierarchy.fcluster(Z,max_rmsd,criterion='distance')
        self.uniques=get_uniques(self.cluster_inds)
        print "number of clusters",len(self.uniques)

        #return clusters by their size
        return self.uniques


    def get_placement_score_from_coordinates(self,model_coords, native_coords):
        """
        Computes the position error (placement distance) and the orientation                                                                      error (placement angle) of the coordinates in model_coords respect to the coordinates in native_coords.
        placement distance - translation between the centroids of the
                           coordinates                                                                                                           placement angle - Angle in the axis-angle formulation of the rotation
        aligning the two rigid bodies.
        """
        native_centroid = IMP.algebra.get_centroid(native_coords)
        model_centroid = IMP.algebra.get_centroid(model_coords)
        translation_vector = native_centroid - model_centroid
        distance = translation_vector.get_magnitude()
        if(len(model_coords) != len(native_coords) ):
            raise ValueError(
                "Mismatch in the number of members %d %d " % (
                    len(model_coords),
                    len(native_coords)) )
        TT = IMP.algebra.get_transformation_aligning_first_to_second(model_coords,
                                                                     native_coords)
        P = IMP.algebra.get_axis_and_angle( TT.get_rotation() )
        angle = P.second*180./math.pi
        return distance, angle


    def get_cc(self,ps):
        '''
        bb_native = self.dmap.get_bounding_box()
        bb_solution = IMP.core.get_bounding_box(IMP.core.XYZs(ps))
        # bounding box enclosing both the particles of the native assembly
        #  and the particles of the model
        bb_union = IMP.algebra.get_union(bb_native, bb_solution)
        # add border of 4 voxels
        border = 4*voxel_size
        bottom = bb_union.get_corner(0)
        bottom += IMP.algebra.Vector3D(-border, -border, -border)
        top = bb_union.get_corner(1)
        top += IMP.algebra.Vector3D(border, border, border)
        bb_union = IMP.algebra.BoundingBox3D(bottom, top)
        '''

        resolution = self.dmap.get_header().get_resolution()
        voxel_size = self.dmap.get_spacing()

        map_solution = IMP.em.SampledDensityMap(self.dmap.get_header())
        map_solution.set_particles(ps)
        map_solution.resample()

        map_solution.calcRMS()
        coarse_cc = IMP.em.CoarseCC()
        # base the calculation of the cross_correlation coefficient on the threshold
        # for the native map, because the threshold for the map of the model changes
        # with each model
        #map_solution.get_header().show()
        threshold = 0.01 # threshold AFTER normalization using calcRMS()
        ccc = coarse_cc.cross_correlation_coefficient(map_solution,
                                                      self.dmap, threshold)
        return ccc


    def get_cluster_representative_combination(self,query_cluster_ind):
        return self.combs[self.clusters_data[query_cluster_ind].cluster_ind]

    def get_cluster_stats(self,query_cluster_ind):
        return self.clusters_data[query_cluster_ind]

    def do_analysis(self,max_comb_ind):
        self.clusters_data={}
        for cluster_ind in self.uniques:
            self.clusters_data[cluster_ind]=self.analyze_cluster(cluster_ind,max_comb_ind)

    def analyze_cluster(self,query_cluster_ind,max_comb_ind):
        import numpy
        #load reference
        mhs_native=[]
        mhs_native_ca=[]
        mhs_native_ca_ps=[]
        calc_rmsd=True
        for i in range(len(self.mhs)):
            if self.asmb.get_component_header(i).get_reference_fn() == "":
                calc_rmsd=False
                continue
            mhs_native.append(IMP.atom.read_pdb(self.asmb.get_component_header(i).get_reference_fn(),self.mdl))
            s1=IMP.atom.Selection(mhs_native[-1]);
            s1.set_atom_types([IMP.atom.AtomType("CA")])
            mhs_native_ca.append([])
            mhs_native_ca_ps.append([])
            for p in s1.get_selected_particles():
                mhs_native_ca[-1].append(IMP.core.XYZ(p).get_coordinates())
                mhs_native_ca_ps[-1].append(IMP.core.XYZ(p))

        rmsds=[]
        distances=[]
        angles=[]
        for i in range(len(self.mhs)):
            distances.append([])
            angles.append([])
        best_sampled_ind=-1
        best_scored_ind=-1
        voxel_size=3 #check with javi
        resolution=20 #check with javi
        counter=-1
        for elem_ind1,cluster_ind1 in enumerate(self.cluster_inds):
            if cluster_ind1 != query_cluster_ind:
                continue
            counter=counter+1
            if calc_rmsd:
                rmsds.append(IMP.atom.get_rmsd(list(itertools.chain.from_iterable(mhs_native_ca)),
                                               list(itertools.chain.from_iterable(self.coords[elem_ind1]))))
            if best_scored_ind==-1:
                self.ensmb.load_combination(self.combs[elem_ind1])
                best_scored_ind=counter
                best_scored_cc=self.get_cc(
                   list(itertools.chain.from_iterable(self.all_ca)))
                if calc_rmsd:
                    best_scored_rmsd = rmsds[-1]
                    best_sampled_ind=counter
                    best_sampled_cc=best_scored_cc
                    best_sampled_rmsd=rmsds[-1]
                self.ensmb.unload_combination(self.combs[elem_ind1])
            #print rmsds[-1],best_scored_rmsd
            if calc_rmsd:
                if rmsds[-1]<best_scored_rmsd:
                    self.ensmb.load_combination(self.combs[elem_ind1])
                    best_sampled_ind=counter
                    best_sampled_cc=self.get_cc(
                       list(itertools.chain.from_iterable(self.all_ca)))
                    best_sampled_rmsd=rmsds[-1]
                    self.ensmb.unload_combination(self.combs[elem_ind1])
                sum_d=0;
                sum_a=0
                for i in range(len(self.mhs)):
                    [d,a]=self.get_placement_score_from_coordinates(self.coords[elem_ind1][i], \
                                                                        mhs_native_ca[i])
                    sum_d=sum_d+d
                    sum_a=sum_a+a
                distances[i].append(sum_d/len(self.mhs))
                angles[i].append(sum_a/len(self.mhs))
        d = numpy.array(list(itertools.chain.from_iterable(distances)))
        a = numpy.array(list(itertools.chain.from_iterable(angles)))
        r = numpy.array(rmsds)
        cd=ClusterData(query_cluster_ind,counter+1,calc_rmsd)
        cd.set_distance_stats(d.mean(),d.std())
        cd.set_angle_stats(a.mean(),a.std())
        if calc_rmsd:
            cd.set_best_scored_data(best_scored_ind, best_scored_rmsd,best_scored_cc,d[0],a[0])
            cd.set_rmsd_stats(r.mean(),r.std())
            cd.set_best_sampled_data(best_sampled_ind,best_sampled_rmsd,best_sampled_cc,d[best_sampled_ind],a[best_sampled_ind])
        else:
            cd.set_best_scored_data(best_scored_ind,-1,best_scored_cc,-1,-1)
        return cd

def usage():
    usage =  """%prog [options] <asmb> <asmb.proteomics> <asmb.mapping>
           <alignment.params> <combinations> <output: clustered combinations>

Clustering of assembly solutions.

This program uses the Python 'fastcluster' module, which can be obtained from
http://math.stanford.edu/~muellner/fastcluster.html
"""
    parser = OptionParser(usage)
    parser.add_option("-m", "--max", type="int", dest="max", default=999999999,
                      help="maximum solutions to consider")
    parser.add_option("-r", "--rmsd", type="float", dest="rmsd", default=5,
                      help="maximum rmsd within a cluster")
    options, args = parser.parse_args()
    if len(args) !=6:
        parser.error("incorrect number of arguments")
    return options,args

def main():
    IMP.base.set_log_level(IMP.WARNING)
    options,args = usage()
    asmb_fn = args[0]
    prot_fn = args[1]
    map_fn = args[2]
    align_fn = args[3]
    combs_fn = args[4]
    output_fn = args[5]

    clust_engine = AlignmentClustering(asmb_fn,prot_fn,map_fn,align_fn,combs_fn)
    clusters=clust_engine.do_clustering(options.max,options.rmsd)
    cluster_representatives=[]
    print "clustering completed"
    print "start analysis"
    clust_engine.do_analysis(options.max)
    repr_combs=[]
    for cluster_ind in clust_engine.uniques:
        repr_combs.append(clust_engine.get_cluster_representative_combination(cluster_ind))
    IMP.multifit.write_paths(repr_combs,output_fn)
    #print the clusters data
    for cluster_ind in clust_engine.uniques:
        info=clust_engine.get_cluster_stats(cluster_ind)
        repr_combs.append(clust_engine.get_cluster_representative_combination(cluster_ind))
        print "==========Cluster index:",info.cluster_ind,"size:",info.cluster_size
        if info.rmsd_calculated:
            print "best sampled in cluster (index,cc,distance,angle,rmsd):",info.best_sampled_ind,info.best_sampled_cc,info.best_sampled_distance,info.best_sampled_angle,info.best_sampled_rmsd
        if info.rmsd_calculated:
            print "cluster representative (index,cc,distance,angle,rmsd):",info.best_scored_ind,info.best_scored_cc,info.best_scored_distance,info.best_scored_angle,info.best_scored_rmsd
        else:
            print "cluster representative (index,cc):",info.best_scored_ind,info.best_scored_cc


if __name__ == "__main__":
    main()
