#!/usr/bin/env python

import IMP
import IMP.em2d as em2d
import IMP.em2d.utility as utility
import IMP.em2d.imp_general.io as io
import IMP.em2d.imp_general.representation as representation
import IMP.em2d.solutions_io as solutions_io
import IMP.em2d.Database as Database

import IMP.statistics as stats
import IMP.container as container
import IMP.atom as atom
import IMP.core as core
import IMP.algebra as alg

import sys
import os
import time
import logging
log = logging.getLogger("cluster_solutions")


class AlignmentClustering:
    """
        Clusters solutions present in a database.
        - The solutions are chosen by sorting the database according to the
          parameter orderby
        - The models are aligned and clustered by RMSD
    """
    def __init__(self, exp):
        """
            @param exp an Experiment class containing the names of the pdb files
        """
        self.exp = exp

    def cluster(self, fn_database, n_solutions, orderby, max_rmsd):
        """
            @param fn_database Database of results
            @param n_solutions Number of solutions to use for clustering
            @param orderby Measure used to order solutions
            @param max_rmsd See do_clustering()
        """
        log.debug("Call to cluster()")
#        sys.exit()
        db = solutions_io.ResultsDB()
        db.connect(fn_database)
        fields = ["reference_frames","solution_id" ]
        data =  db.get_solutions( fields, n_solutions,orderby)
        db.close()
        self.solution_ids = [row[1] for row in data]
        # reference frames for each configuration
        confs_RFs = []
        for row in data:
            rs = row[0].split("/")
            RFs = [io.TextToReferenceFrame(r).get_reference_frame() for r in rs]
            confs_RFs.append(RFs)
        self.do_clustering(confs_RFs, max_rmsd)

    def do_clustering(self, confs_RFs, max_rmsd):
        """
            Cluster configurations for a model based on RMSD.
            An IMP.ConfigurationSet is built using the reference frames for
            of the components of the assembly for each solution
            @param confs_RFs A lsit containing a tuples of reference frames.
                Each tuple contains the reference frame for the rigid body
                of one component of the assembly
            @param max_rmsd Maximum RMSD tolerated when clustering
        """
        model = IMP.Model()
        assembly = representation.create_assembly(model, self.exp.fn_pdbs)
        rbs = representation.create_rigid_bodies(assembly)
        configuration_set = IMP.ConfigurationSet(model)
        for RFs in confs_RFs:
            representation.set_reference_frames(rbs, RFs)
            configuration_set.save_configuration()
        particles_container = container.ListSingletonContainer(model)
        particles_container.add_particles(atom.get_leaves(assembly))
        metric = stats.ConfigurationSetRMSDMetric(
                                configuration_set,particles_container, True)
        log.info("Clustering ... ")
        maximum_centrality = 10
        self.pclus = stats.create_centrality_clustering( metric, max_rmsd,
                                                maximum_centrality)
        n = self.pclus.get_number_of_clusters()
        log.info("Number of clusters found: %s", n)


    def store_clusters(self, fn_database, tbl="clusters"):
        """
            Store the clusters in the database.
            The database does not necessarily has to be the same database
            used to read the solutions
            @param fn_database Database where the clusters are written
            @param tbl Table of the database where the clusters are written
        """
        if not hasattr(self, "pclus"):
            raise ValueError("Clustering not performed")
        db = solutions_io.ResultsDB()
        if not os.path.exists(fn_database):
            db.create(fn_database)
        db.connect(fn_database)
        db.add_clusters_table(tbl)
        n_clusters = self.pclus.get_number_of_clusters()
        clusters_data = []
        for i in range(n_clusters):
            # IDs of the solutions *according* to the clustering algorithm
            elements = self.pclus.get_cluster(i)
            r = self.pclus.get_cluster_representative(i)
            n_elements = len(elements)
            # IDs of the solutions as stored in the database
            solution_ids = [self.solution_ids[k] for k in elements]
            elements  = "|".join( map(str, elements) )
            solution_ids  = "|".join( map(str, solution_ids) )
            db.add_cluster_record( i, n_elements, r, elements, solution_ids)
        db.store_cluster_data()
        db.close()

if __name__ == "__main__":

    parser = IMP.OptionParser(imp_module=em2d,
                              description= \
        "Clusters the best solutions contained in the database, and writes a "
        " new table in the database containing the clusters ids and members")
    parser.add_option("--exp",
                     dest="experiment",
                     default=None,
                     help="File describing an experiment ")
    parser.add_option("--db",
                    dest="fn_database",
                    help="Database of results")
    parser.add_option("--o",
                      dest="fn_output_db",
                      default="clusters.db",
                      help="Database file to store the clusters obtained. It "\
                           "can be the same one containing the solutions")
    parser.add_option("--n",
                    dest="n_solutions",
                    type=int,
                    default = 10,
                    help="Number of solutions to cluster")
    parser.add_option("--orderby",
                     dest="orderby",
                     default = None,
                     help="Sor the solutions according to this measure before "\
                            "clustering")
    parser.add_option("--log",
                     dest="log",
                     default=None,
                     help="File for logging")
    parser.add_option("--rmsd",
                     type=float,
                     dest="max_rmsd",
                     default=10,
                     help="Maximum rmsd centroids to define clusters")

    args = parser.parse_args()
    args = args[0]
    if(len(sys.argv) == 1):
        parser.print_help()
        sys.exit()
    if(args.log):
        logging.basicConfig(filename=args.log, filemode="w")
    else:
        logging.basicConfig(stream=sys.stdout)
    logging.root.setLevel(logging.DEBUG)

    if(args.fn_database):
        if(not args.n_solutions or not args.orderby):
            raise ValueError("parameters --n and --orderby required")
        exp = utility.get_experiment_params(args.experiment)
        tc = AlignmentClustering(exp)
        tc.cluster(args.fn_database, args.n_solutions, args.orderby,
                                                                args.max_rmsd)
        tc.store_clusters(args.fn_output_db, "clusters")
