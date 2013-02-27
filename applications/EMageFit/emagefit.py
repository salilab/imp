#!/usr/bin/env python

import ConfigParser
import sys
import os
import time
import logging
log = logging.getLogger("domino_model")

import IMP
import IMP.atom as atom
import IMP.algebra as alg
import IMP.em2d as em2d


import IMP.em2d.Database as Database
import IMP.em2d.imp_general.io as io
import IMP.em2d.imp_general.alignments as alignments
import IMP.em2d.imp_general.representation as representation
import IMP.em2d.DominoModel as DominoModel
import IMP.em2d.MonteCarloRelativeMoves as MonteCarloRelativeMoves
import IMP.em2d.sampling as sampling
import IMP.em2d.solutions_io as solutions_io
import IMP.em2d.utility as utility


def setup_sampling_schema(model, params):
    """
       Set the discrete states for sampling for DOMINO.
       @param model A DominoModel object
       @param params See the help for the script
    """
    n_rbs = len(model.components_rbs)
    sampling_schema = sampling.SamplingSchema(n_rbs, params.fixed, params.anchor)

    sampling_schema.read_from_database( params.sampling_positions.read,
                                            ["reference_frames"],
                                            params.sampling_positions.max_number,
                                            params.sampling_positions.orderby)
    for i, rb in enumerate(model.components_rbs):
        model.set_rb_states(rb, sampling_schema.get_sampling_transformations(i))


def write_pdbs_for_solutions(params, fn_database, n=10, orderby="em2d",
                    split_components=False):
    """
        Write PDBs for the solutions in the database
        @param params class with the parameters for the experiment
        @param fn_database File containing the database of solutions
        @param n Number of solutions to write
        @param orderby Restraint used for sorting the solutions
        @param split_components If true, each component of the assembly in a
                            solution is written as a separate PDB file
    """
    m = DominoModel.DominoModel()
    m.set_assembly_components(params.fn_pdbs, params.names)
    db = solutions_io.ResultsDB()
    db.connect(fn_database)
    data = db.get_solutions(["reference_frames"], n, orderby)
    db.close()
    for i, d in enumerate(data):
        texts = d[0].split("/") # each row has only one value
        log.debug("Reference frames to generate pdb %s",texts)
        RFs = [io.TextToReferenceFrame(t).get_reference_frame() for t in texts]
        if not split_components:
            fn = "solution-%03d.pdb" % i
            m.write_pdb_for_reference_frames(RFs,fn)
        else:
            fn = "solution-%03d-" % i
            m.write_pdbs_for_reference_frames(RFs,fn)


def write_nth_largest_cluster(params, fn_database, fn_db_clusters,
                                        position, table_name="clusters"):
    """
        @param params class with the parameters for the experiment
        @param fn_database Database file with the reference frames of the
                    solutions
        @param fn_db_clusters Database file for the clusters.
                        It can the same as fn_database or a different one.
        @param table_name Table with the info for the clusters in
                     fn_db_clusters
        @param position Cluster position (by number of elements) requested.
                                           The index of largest cluster is 1.
    """
    m = DominoModel.DominoModel()
    m.set_assembly_components(params.fn_pdbs, params.names)
    # get cluster
    db_clusters = solutions_io.ResultsDB()
    db_clusters.connect(fn_db_clusters)
    cl_record = db_clusters.get_nth_largest_cluster(position)
    log.info("Size of %s largest cluster: %s",position, cl_record.n_elements)
    # get solutions
    db = solutions_io.ResultsDB()
    db.connect(fn_database)
    fields = ("reference_frames",)
    solutions_ids = map(int, cl_record.solutions_ids.split("|"))
    solutions = db.get_solutions_from_list(fields, solutions_ids)
    db.close()
    for i, d in zip(solutions_ids, solutions):
        texts = d[0].split("/")
        RFs = [io.TextToReferenceFrame(t).get_reference_frame() for t in texts]
        fn = "cluster-%02d-solution-%03d.pdb" % (position, i)
        log.debug(fn)
        m.write_pdb_for_reference_frames(RFs, fn)

def generate_domino_model(params, fn_database, fn_log = None):
    """
        Generate a model for an assembly using DOMINO.
        @param params Class with the parameters for the experiment
        @param fn_database Databse file that will contain the solutions
                    SQLite format
    """
    log.info(io.imp_info([IMP, em2d]))
    t0 = time.time()
    m = DominoModel.DominoModel()
    if hasattr(params, "test_opts") and params.test_opts.do_test:
        m.set_assembly(params.test_opts.test_fn_assembly, params.names)
    else:
        m.set_assembly_components(params.fn_pdbs, params.names)
    setup_sampling_schema(m, params)
    if hasattr(params.sampling_positions, "align_before_domino") and \
                params.sampling_positions.align_before_domino:
        m.align_rigid_bodies_states()

    if hasattr(params, "benchmark"):
        m.set_native_assembly_for_benchmark(params)
    set_pair_score_restraints(params, m)
    set_xlink_restraints(params, m)
    set_geometric_complementarity_restraints(params, m)
    if hasattr(params.domino_params,"fn_merge_tree"):
        fn = params.domino_params.fn_merge_tree
        if not os.path.exists(fn):
            raise IOError("merge tree file not found: %s" % fn)
        m.read_merge_tree(fn)
    else:
        m.create_merge_tree()
    set_connectivity_restraints(params, m)
    set_pairs_excluded_restraint(params, m)
    set_em2d_restraints(params, m)
    m.setup_domino_sampler()
    m.do_sampling("assignments_heap_container", params.domino_params)
    tf = time.time()
    log.info("Total time for the sampling (non-parallel): %s",tf-t0)
    m.write_solutions_database(fn_database, params.n_solutions)



def print_restraints(params):
    """
        Generate a model for an assembly using DOMINO.
        @param params Class with the parameters for the experiment
        @param fn_database Databse file that will contain the solutions
                    SQLite format
    """
    log.info(io.imp_info([IMP, em2d]))
    t0 = time.time()
    m = DominoModel.DominoModel()
    if hasattr(params, "test_opts") and params.test_opts.do_test:
        m.set_assembly(params.test_opts.test_fn_assembly, params.names)
    else:
        m.set_assembly_components(params.fn_pdbs, params.names)
    if hasattr(params, "benchmark"):
        m.set_native_assembly_for_benchmark(params)
    set_pair_score_restraints(params, m)
    set_xlink_restraints(params, m)
    set_geometric_complementarity_restraints(params, m)
    set_connectivity_restraints(params, m)
    set_pairs_excluded_restraint(params, m)
    set_em2d_restraints(params, m)
    log.debug("RESTRAINTS FOR THE INPUT CONFIGURATION")
    DominoModel.print_restraints_values(m.model)


def set_pair_score_restraints(params, model):
    """ Set the pair score restraints in the DominoModel
        @param model DominoModel class
        @param params See the help for the script
    """
    if hasattr(params, "pair_score_restraints"):
        model.create_coarse_assembly(params.n_residues)
        for r_params in params.pair_score_restraints:
            model.set_pair_score_restraint(*r_params)

def set_xlink_restraints(params, model):
    """ Set the cross-linking restraints in the DominoModel
        @param model DominoModel class
        @param params See the help for the script
    """
    if hasattr(params, "xlink_restraints"):
        for params in params.xlink_restraints:
            model.set_xlink_restraint(*params)

def set_geometric_complementarity_restraints(params, model):
    """ Set the geometric complementarity restraints in the DominoModel
        @param model DominoModel class
        @param params See the help for the script
    """
    if hasattr(params, "complementarity_restraints"):
        model.create_coarse_assembly(params.n_residues)
        for r_params in params.complementarity_restraints:
            model.set_complementarity_restraint(*r_params)

def set_connectivity_restraints(params, model):
    """ Set the connectivity restraints in the DominoModel
        @param model DominoModel class
        @param params See the help for the script
    """
    if hasattr(params,"connectivity_restraints"):
        model.create_coarse_assembly(params.n_residues)
        for r_params in params.connectivity_restraints:
            model.set_connectivity_restraint(*args)

def set_pairs_excluded_restraint(params, model) :
    """
        All against all excluded volume restraints
        @param model DominoModel class
        @param params See the help for the script
    """
    if hasattr(params,"pairs_excluded_restraint"):
        model.create_coarse_assembly(params.n_residues)
        model.set_close_pairs_excluded_volume_restraint(
                                            *params.pairs_excluded_restraint)

def set_em2d_restraints(params, model):
    """
        Restraints related to the match to EM images
        @param model DominoModel class
        @param params See the help for the script
    """
    if hasattr(params,"em2d_restraints"):
        for r_params in params.em2d_restraints:
            model.set_em2d_restraint(*r_params)


def generate_monte_carlo_model(params, fn_database, seed,
                               write_solution=False, fn_log = None):
    """
        Generate a model for an assembly using MonteCarlo optimization
        @param params Class with the parameters for the experiment
        @param fn_database Datbase file where the solutions will be written.
                            SQLite format.
        @param write_solution If True, writes a PDB with the final model
        @param fn_log File for logging. If the value is None, no file is written
    """
    log.info(io.imp_info([IMP, em2d]))
    m = DominoModel.DominoModel()
    m.set_assembly_components(params.fn_pdbs, params.names)
    set_pair_score_restraints(params, m)
    set_xlink_restraints(params, m)
    set_geometric_complementarity_restraints(params, m)
    set_connectivity_restraints(params, m)
    set_pairs_excluded_restraint(params, m)
    set_em2d_restraints(params, m )
    if hasattr(params, "benchmark"):
        m.set_native_assembly_for_benchmark(params)

    MonteCarloRelativeMoves.set_random_seed(seed)
    mc = MonteCarloRelativeMoves.MonteCarloRelativeMoves(m.model,
                                                m.components_rbs, params.anchor)
    mc.set_temperature_pattern(params.monte_carlo.temperatures,
                                  params.monte_carlo.iterations,
                                  params.monte_carlo.cycles)
    mc.set_moving_parameters(params.monte_carlo.max_translations,
                                params.monte_carlo.max_rotations)
    if not hasattr(params,"dock_transforms"):
        mc.dock_transforms = []
    else:
        mc.dock_transforms = params.dock_transforms

    # Probability for a component of the assembly of doing random movement
    # of doing a relative movement respect to another component
    mc.non_relative_move_prob = params.monte_carlo.non_relative_move_prob
    mc.run_monte_carlo_with_relative_movers()
    m.write_monte_carlo_solution(fn_database)
    if write_solution:
        atom.write_pdb(m.assembly, fn_database + ".pdb")



def create_dockings_from_xlinks(params):
    """
        Perform dockings that satisfy the cross-linking restraints.
        1) Based on the number of restraints, creates an order for the
           docking between pairs of subunits, favouring the subunits with
           more crosslinks to be the "receptors"
        2) Moves the subunits that are going to be docked to a position that
           satisfies the x-linking restraints. There is no guarantee that this
           position is correct. Its purpose is to help the docking
           algorithm with a clue of the proximity/orientation between subunits
        3) Performs docking between the subunits
        4) Filters the results of the docking that are not consistent with
           the cross-linking restraints
        5) Computes the relative transformations between the rigid bodies
           of the subunits that have been docked
        @param params Class with the parameters for the experiment
    """
    log.info("Creating initial assembly from xlinks and docking")
    import emagefit_dock as dock
    import IMP.em2d.buildxlinks as bx
    m = DominoModel.DominoModel()
    m.set_assembly_components(params.fn_pdbs, params.names)
    set_xlink_restraints(params, m)
    order = bx.DockOrder()
    order.set_xlinks(m.xlinks_dict)
    docking_pairs = order.get_docking_order()

    if hasattr(params, "have_hexdock"):
        if not params.have_hexdock:
            return

    for rec, lig in docking_pairs:
        xlinks_list = m.xlinks_dict.get_xlinks_for_pair((rec,lig))
        log.debug("Xlinks for the pair %s %s",rec, lig)
        for xl in xlinks_list:
            log.debug("%s", xl.show())

        h_receptor = representation.get_component(m.assembly, rec)
        h_ligand = representation.get_component(m.assembly, lig)
        rb_receptor = representation.get_rigid_body(m.components_rbs,
                                         representation.get_rb_name(rec))
        rb_ligand = representation.get_rigid_body(m.components_rbs,
                                         representation.get_rb_name(lig))
        initial_ref = rb_ligand.get_reference_frame()
        # move to the initial docking position
        mv = bx.InitialDockingFromXlinks()
        mv.set_xlinks(xlinks_list)
        mv.set_hierarchies(h_receptor, h_ligand)
        mv.set_rigid_bodies(rb_receptor, rb_ligand)
        mv.move_ligand()
        fn_initial_docking = "%s-%s_initial_docking.pdb" % (rec,lig)
        mv.write_ligand(fn_initial_docking)
        # dock
        dock.check_for_hexdock()
        hex_docking = dock.HexDocking()
        receptor_index = params.names.index(rec)
        fn_transforms = "hex_solutions_%s-%s.txt" % (rec, lig)
        fn_docked = "%s-%s_hexdock.pdb" % (rec, lig)
        hex_docking.dock(params.fn_pdbs[receptor_index],
                         fn_initial_docking, fn_transforms, fn_docked, False)

        sel = atom.ATOMPDBSelector()
        new_m = IMP.Model()
        # After reading the file with the initial solution, the reference frame
        # for the rigid body of the ligand is not necessarily the  same one
        # that it had when saved.
        # Thus reading the file again ensures  consisten results when
        # using the HEXDOCK transforms
        new_h_ligand =  atom.read_pdb(fn_initial_docking, new_m, sel)
        new_rb_ligand = atom.create_rigid_body(new_h_ligand)
        Tlig = new_rb_ligand.get_reference_frame().get_transformation_to()
        fn_filtered = "hex_solutions_%s-%s_filtered.txt" % (rec, lig)
        # new_h_ligand contains the coordinates of the ligand after moving it
        # to the initial position for the docking
        dock.filter_docking_results(h_receptor, new_h_ligand, xlinks_list,
                                            fn_transforms, fn_filtered)
        # transforms to apply to the ligand as it is in the file
        # fn_initial_docking
        transformations_hexdock = dock.read_hex_transforms(fn_filtered)

        # The initial position the ligand, by construction, is a "docking"
        # solution, very bad probably, but is an starting point for Monte Carlo
        # refinement. Hexdock therefore would "apply" an identity transformation
        log.info("Adding the initial approximate transformation to the " \
                " set of possible transformations")
        transformations_hexdock.append(alg.get_identity_transformation_3d())

        Trec = rb_receptor.get_reference_frame().get_transformation_to()
        Tinternal = []
        for i,T in enumerate(transformations_hexdock):
            Tdock = alg.compose(T, Tlig)
            ref = alg.ReferenceFrame3D(Tdock)
            new_rb_ligand.set_reference_frame(ref)
            # internal transformation. The relationship is Tdock = Trec * Ti
            Ti = alg.compose(Trec.get_inverse(), Tdock)
            Tinternal.append(Ti)
        fn_relative = "relative_positions_%s-%s.txt" % (rec, lig)
        io.write_transforms(Tinternal, fn_relative)
        rb_ligand.set_reference_frame(initial_ref)
        #os.remove(fn_initial_docking)

if __name__ == "__main__":
    import ConfigParser
    cparser = ConfigParser.SafeConfigParser()

    parser = IMP.OptionParser(description="Domino grid sampling",
                              imp_module=em2d)
    parser.add_option("--o",
                    dest="fn_database",
                    default = False,
                    help="SQLite file. This file is an output when modeling. " \
                        " It is an input parameter when querying the database.")
    parser.add_option("--exp",
                     dest="fn_params",
                     default=None,
                     help="Experiment file. This should be a Python file "
                          "containing a class called Experiment containing "
                          "all parameters required to do the modeling.")
    parser.add_option("--w",
                    type=int,
                     dest="write",
                      default=None,
                     help="write the best scored solutions to PDB files.")
    parser.add_option("--wcl",
                     dest="write_cluster",
                     metavar="fn_db_clusters position",
                     nargs = 2,
                     default=None,
                     help="Write the n-th largest cluster. This option has " \
                            "two arguments. The first one is the database file " \
                            "containing the clusters. The second one is the " \
                            "number (position) of the cluster to write. " \
                            "The index of the first cluster is 1.")
    parser.add_option("--orderby",
                      dest="orderby",
                      default=None,
                      help="Measure used to sort solutions when saving them " \
                      "to PDB")
    parser.add_option("--monte_carlo", dest="monte_carlo",
                      metavar = "number",
                      type=int,
                      default=False,
                      help="Run a MonteCarlo optimization. There is one " \
                       "parameter passed to the option. When using this " \
                       "script in a cluster and sending a lot of them at the "\
                       "same time, the effective random number obtained by " \
                       "seeding using the time is the same for all scripts. " \
                       "To avoid that situation, you can pass an integer. " \
                       "If the number is -1, the time is " \
                       "used as the seed (useful for one core)")
    parser.add_option("--dock", dest="dock",
                    action="store_true",
                    default=False,
                    help="Perform docking of the components of the assembly "\
                         "guided by cross-linking restraints")
    parser.add_option("--merge", dest="merge",
                  action="callback",
                    callback=utility.vararg_callback,
                    help="Merge the database files")
    parser.add_option("--gather", dest="gather",
                  action="callback",
                    callback=utility.vararg_callback,
                    help="Gather database files and store then in a single " \
                         "file. The arguments are the names of the databases "\
                         "to gather.")
    parser.add_option("--log",
                     dest="log",
                     default=None,
                     help="File for logging.")
    parser.add_option("--cdrms",
                     dest="cdrms",
                     type=int,
                     default=False,
                     metavar="number",
                     help="Align the best solutions to the " \
                            "native using the DRMS of the centroids.")

    parser.add_option("--pr",
                    action="store_true",
                     dest="pr_rest",
                     default=False,
                     help="Print restraints of model and native")


    args = parser.parse_args()
    args = args[0]
    if len(sys.argv) == 1:
        parser.print_help()
        quit()
    if args.log:
        logging.basicConfig(filename=args.log, filemode="w")
    else:
        logging.basicConfig(stream=sys.stdout)
    logging.root.setLevel(logging.INFO)

    if args.merge:
        max_number_of_results = 100000
        t0 = time.time()
        if not args.fn_database or not args.orderby:
            raise ValueError("Merging databases requires the output database " \
                          "results and the name of the restraint to order by")
        solutions_io.gather_best_solution_results(args.merge, args.fn_database,
                max_number=max_number_of_results, orderby=args.orderby)
        log.info("Merging done. Time %s", time.time() - t0)
        quit()
    if args.gather:
        t0 = time.time()
        solutions_io.gather_solution_results(args.gather, args.fn_database)
        log.info("Gathering done. Time %s", time.time() - t0)
        quit()

    if args.write:
        if not args.fn_database or not args.orderby:
            raise ValueError("Writing solutions requires the database of " \
                         "results and the name of the restraint to order by")
        if args.orderby == "False":
            args.orderby = False
        params = utility.get_experiment_params(args.fn_params)
        write_pdbs_for_solutions(params, args.fn_database,
                                 args.write, args.orderby, )
        quit()

    if args.write_cluster:
        if not args.fn_database:
            raise ValueError("Writing clusters requires the database file")
        fn_db_clusters = args.write_cluster[0]
        position = int(args.write_cluster[1])
        params = utility.get_experiment_params(args.fn_params)
        write_nth_largest_cluster(params,  args.fn_database,
                                  fn_db_clusters, position )
        quit()

    if args.cdrms:
        if not args.fn_database or not args.orderby:
            raise ValueError("Writing models requires the database of " \
                            "results and the name of the restraint to order by")
        params = utility.get_experiment_params(args.fn_params)

        write_pdbs_aligned_by_cdrms(params, args.fn_database,
                                    args.cdrms, args.orderby)
        quit()

    if args.monte_carlo:
        params = utility.get_experiment_params(args.fn_params)
        generate_monte_carlo_model(params,
                                   args.fn_database, args.monte_carlo)
        quit()

    if args.dock:
        params = utility.get_experiment_params(args.fn_params)
        create_dockings_from_xlinks(params)
        quit()


    if(args.pr_rest):
        params = utility.get_experiment_params(args.fn_params)
        print_restraints(params)
        quit()


    params = utility.get_experiment_params(args.fn_params)
    generate_domino_model(params, args.fn_database)
