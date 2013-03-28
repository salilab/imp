import sys
import time
import csv
import logging
log = logging.getLogger("DominoModel")

import IMP
import IMP.domino as domino
import IMP.core as core
import IMP.container as container
import IMP.display as display
import IMP.atom as atom
import IMP.algebra as alg
import IMP.em2d as em2d
import IMP.base
import IMP.multifit as multifit

import IMP.em2d.imp_general.comparisons as comparisons
import IMP.em2d.imp_general.alignments as alignments
import IMP.em2d.imp_general.io as io
import IMP.em2d.imp_general.representation as representation

import IMP.em2d.restraints as restraints
import IMP.em2d.buildxlinks as buildxlinks
import IMP.em2d.solutions_io as solutions_io
import IMP.em2d.csv_related as csv_related


field_delim = ","  # separate fields in the output text file
unit_delim = "/" # separate units within a field (eg, reference frames).
                 # It is used in output text files and in databse files



class DominoModel:
    """
        Management of a model using DOMINO
    """
    def __init__(self, name="my model"):
        self.model = IMP.Model()
        self.model.set_name(name)
        self.configuration_sampling_done = False
        self.assignments_sampling_done = False
        self.rb_states_table = domino.ParticleStatesTable()
        self.merge_tree = None
        # measure_models is True only when doing benchmark
        self.measure_models = False
        # The first time that create_coarse is found and its value is True
        # a coarse version of the assembly is built
        self.create_coarse = True
        self.restraints = dict()
        self.xlinks_dict = buildxlinks.XlinksDict()

    def add_restraint(self, r, name, weight, max_score=False):
        """
            Adds a restraint to the model
            @param r An IMP.Restraint object
            @param name Name for the restraint
            @param weight Weight for the restraint
            @param max_score Maximum score allowed for the restraint. If
            max_score is False, there is no limit for the value of the restraint
        """
        log.info("Adding restraint %s weight %s max_score %s",
                                                name, weight, max_score)
        if "-" in name:
            raise ValueError("SQL database does not accept restraint names "\
                            "containing -")
        r.set_name(name)
        if max_score != None and max_score != False:
            r.set_maximum_score(max_score)
        r.set_weight(weight)
        self.restraints[name] = r
        self.model.add_restraint(r)


    def align_rigid_bodies_states(self):
        """
            Aligns the set of structures considered for DOMINO sampling.
            The function:
                1) reads the possible reference frames that the
                    rb_states_table stores for each rigid body. This table
                    must be filled before using this function. Usually it is
                    filled with the results from a previous Monte Carlo sampling.
                    If the solutions from Monte Carlo seem to have the same structure
                    but they are not aligned to each other, this function can
                    help setting better starting positions to use with DOMINO.
                2) Gets the first state for each of the rigid bodies and sets
                   a reference structure using such states.
                3) Aligns all the rest of the structures respect to the reference
                4) Replaces the values of the reference frames stored in the
                   rb_states_table with the new values obtained from the alignments.
                   It does it for all states of a rigid body.
            @note: If this function is applied, the parameters "anchor" and "fixed"
               are ignored, as they are superseded by the use of the aligments
               calculated here.
        """
        log.debug("Align the configurations read from the database before " \
                    "running DOMINO")
        rb_states = []
        n_states = []
        for rb in self.components_rbs:
            ps = self.rb_states_table.get_particle_states(rb)
            n = ps.get_number_of_particle_states()
            if n == 0:
                raise ValueError("There are no particle states for %s"  % rb.get_name())
            n_states.append(n)
            rb_states.append(ps)
        # coordinates of the first configuration (there is at least one for all the rbs)
        for rb, states in zip(self.components_rbs, rb_states):
            states.load_particle_state(0, rb)
        reference_coords = get_coordinates(self.components_rbs)
        aligned_transformations = [[] for rb in self.components_rbs]
        for i in range(1, max(n_states)):
            log.debug("Aligning rigid bodies configuration %s" % i)
            # load the configuration
            for j in range(len(self.components_rbs)):
                # if there are no more particle states for this rigid body, use the last
                state_index = min(i,n_states[j] - 1)
                rb_states[j].load_particle_state(state_index, self.components_rbs[j])
            coords = get_coordinates(self.components_rbs)
            T = alg.get_transformation_aligning_first_to_second(coords, reference_coords)
            for j, rb in enumerate(self.components_rbs):
                t = rb.get_reference_frame().get_transformation_to()
                new_t =  alg.compose(T, t)
                aligned_transformations[j].append(new_t)
        # set the new possible transformations
        for i, rb in enumerate(self.components_rbs):
            self.set_rb_states(rb, aligned_transformations[i])


    def set_xlink_restraint(self, id1, chain1, residue1, id2, chain2, residue2,
                                 distance, weight, stddev, max_score=False):
        """
            Set a restraint on the maximum distance between 2 residues
            @param id1 Name of the first component
            @param residue1 Residue number for the aminoacid in the first
                component.The number is the number in the PDB file, not the
                number relative to the beginning of the chain
            @param id2 Name of the second component
            @param residue2 Residue number for the aminoacid in the second
              component.
            @param distance Maximum distance tolerated
            @param weight Weight of the restraint
            @param stddev Standard deviation used to approximate the
                HarmonicUpperBound function to a Gaussian
            @param max_score See help for add_restraint(). If none is given,
            the maximum score is set to allow a maximum distance of 10 Angstrom
            greater than the parameter "distance".
        """
        xlink = buildxlinks.Xlink(id1, chain1, residue1, id2, chain2, residue2, distance)
        log.info("Setting cross-linking restraint ")
        log.info("%s", xlink.show())
        self.xlinks_dict.add(xlink)
        # setup restraint
        A = representation.get_component(self.assembly, xlink.first_id)
        s1=IMP.atom.Selection(A, chain=xlink.first_chain,
                                residue_index=xlink.first_residue)
        p1 = s1.get_selected_particles()[0]
        B = representation.get_component(self.assembly, xlink.second_id)
        s2=IMP.atom.Selection(B, chain=xlink.second_chain,
                                residue_index=xlink.second_residue)
        p2 = s2.get_selected_particles()[0]
        k = core.Harmonic.get_k_from_standard_deviation(stddev)
        score = core.HarmonicUpperBound(xlink.distance, k)
        pair_score = IMP.core.DistancePairScore(score)
        r = IMP.core.PairRestraint(pair_score, IMP.ParticlePair(p1, p2))
        if not max_score:
            error_distance_allowed = 100
            max_score = weight * score.evaluate(distance + error_distance_allowed)
        log.info("%s, max_score %s", xlink.show(), max_score)
        self.add_restraint(r, xlink.get_name(), weight, max_score)

    def set_complementarity_restraint(self, name1, name2, rname,
                                         max_sep_distance, max_penetration,
                                         weight,max_score=1e10):
        """
            Set a restraint for geometric complementarity between 2 components
            @param name1 name of
            @param name2 - The restraint is applied to this components
            - rname - The name given to the restraint
            - max_sep_distance - maximum distance between molecules
                                tolerated by the restraint
            - max_penetration - Maximum penetration allowd (angstrom)
        """
        log.info("Setting geometric complementarity restraint %s: %s - %s",
                            rname, name1, name2)
        A = representation.get_component(self.assembly, name1)
        B = representation.get_component(self.assembly, name2)
        restraint = multifit.ComplementarityRestraint(atom.get_leaves(A),
                                  atom.get_leaves(B), rname)
        restraint.set_maximum_separation(max_sep_distance)
        restraint.set_maximum_penetration(max_penetration)
        log.debug("Maximum separation: %s Maximum penetration score: %s",
                     max_sep_distance, max_penetration)
        self.add_restraint(restraint, rname, weight, max_score)


    def create_coarse_assembly(self, n_residues, write=False):
        """
            Simplify the assembly with a coarse representation
            @param n_residues Number of residues used for a coarse particle
            @param write If True, write the coarse assembly to a
                format that Chimera can display
        """
        if self.create_coarse:
            log.info("Creating simplified assembly")
            self.coarse_assembly = \
            representation.create_simplified_assembly(self.assembly,
                                            self.components_rbs, n_residues)
        if write:
            io.write_hierarchy_to_chimera(self.assembly, "coarse_assembly.py")
        self.create_coarse = False


    def do_sampling(self, mode="assignments_heap_container", params=None):
        """
            Do Domino sampling
            @param mode The mode used to recover solutions from domino.
                It can have
              the value "configuration", "assignments", or
              "assignments_heap_container". The mode
              "configuration" recovers all possible configurations
              (takes a  while to generate them).
              The mode "assignments" only provides the assignments
              (state numbers) of the solutions. It is faster but requires
              generating the solutions afterwards
              The mode "assignments_heap_container" selects the best solutions
              after ecah merging in DOMINO, discarding the rest.
              In practice I used the mode "assignments_heap_container"
        """
        t0 = time.time()
        if mode == "configuration":
            self.configuration_set = self.sampler.get_sample()
            tf = time.time()
            log.info("found %s configurations. Time %s",
                        self.configuration_set.get_number_of_configurations(),
                        tf-t0)
            self.configuration_sampling_done = True
        elif mode == "assignments":
            subset = self.rb_states_table.get_subset()
            log.info("Do sampling with assignments. Subset has %s elements: %s",
                     len(subset), subset)
            self.solution_assignments = \
                     self.sampler.get_sample_assignments(subset)
            tf = time.time()
            log.info("found %s assignments. Time %s",
                        len(self.solution_assignments), tf-t0)
            self.assignments_sampling_done = True
        elif mode == "assignments_heap_container":
            subset = self.rb_states_table.get_subset()
            log.info("DOMINO sampling  an assignments_heap_container. "\
                    "Subset has %s elements: %s", len(subset), subset)
            # last vertex is the root of the merge tree
            root = self.merge_tree.get_vertices()[-1]
            container = self.get_assignments_with_heap(root,
                                                params.heap_solutions)
            self.solution_assignments = container.get_assignments()
            tf = time.time()
            log.info("found %s assignments. Time %s",
                        len(self.solution_assignments), tf-t0)
            self.assignments_sampling_done = True
        else:
            raise ValueError("Requested wrong mode for the DOMINO sampling")


    def get_assignment_text(self, assignment, subset):
        """
            Get the formatted text for an assignment
            @param subset Subset of components of the assembly
            @param assignment Assignment class with the states for the subset
            @note: The order in assignment and subset is not always the order
            of the rigid bodies in self.components_rbs. The function reorders
            the terms in the assignment so there is correspondence.
        """
        ordered_assignment = []
        for rb in self.components_rbs:
            for i, particle in enumerate(subset):
                if rb.get_particle() == particle:
                    ordered_assignment.append( assignment[i])
        text = "|".join([str(i) for i in ordered_assignment])
        return text


    def get_assignment_and_RFs(self, assignment, subset):
        """
            Return a line with texts for an assignment and the
            the reference frames of the RigidBodies in the subset.
            @param subset Subset of components of the assembly
            @param assignment Assignment class with the states for the subset
            @note: see the get_assignment_assignment_text() note.
        """
        ordered_assignment = []
        RFs = []
        for rb in self.components_rbs:
            for i, particle in enumerate(subset):
                if rb.get_particle() == particle:
                    j = assignment[i]
                    ordered_assignment.append(j)
                    pstates = self.rb_states_table.get_particle_states(rb)
                    pstates.load_particle_state(j, rb.get_particle())
                    RFs.append(rb.get_reference_frame())
        RFs_text = unit_delim.join(
               [io.ReferenceFrameToText(ref).get_text() for ref in RFs])
        assignment_numbers = "|".join([str(i) for i in ordered_assignment])
        return [assignment_numbers, RFs_text]

    def get_assignment_info(self, assignment, subset):
        """
            Info related to an assignment. Returns a list with text for the
            assignment and all the scores for the restraints
            @param subset Subset of components of the assembly
            @param assignment Assignment class with the states for the subset
        """
        text = self.get_assignment_and_RFs(assignment, subset)
        scores, total_score = self.get_assignment_scores(assignment, subset)
        info = text + [total_score] + scores
        return info

    def get_assignment_scores(self, assignment, subset):
        """
            Returns a list with the values for the restraints on a subset of
            the components of the assembly
            @param subset Subset of components of the assembly
            @param assignment Assignment class with the states for the subset
        """
        restraints_to_evaluate = \
                        self.restraint_cache.get_restraints(subset, [])
        scores = [self.restraint_cache.get_score(r, subset, assignment)
                                            for r in restraints_to_evaluate]
        total_score = sum(scores)
        return scores, total_score


    def load_state(self, assignment):
        """
            Load the positions of the components given by the assignment
            @param assignment Assignment class
        """
        subset = self.rb_states_table.get_subset()
        domino.load_particle_states(subset, assignment, self.rb_states_table)


    def get_assignments_with_heap(self, vertex, k=0):
        """
            Domino sampling that recovers the assignments for the root of the
            merge tree, but
            conserving only the best k scores for each vertex of the tree.
            @param vertex Vertex with the root of the current merge tree. This
            function is recursive.
        """
        if(self.sampler.get_number_of_subset_filter_tables() == 0 ):
            raise ValueError("No subset filter tables")
        if(self.merge_tree == None):
            raise ValueError("No merge tree")
        # In the merge tree, the names of the vertices are the subsets.
        # The type of the vertex name is a domino.Subset
        subset = self.merge_tree.get_vertex_name(vertex)
        log.info("computing assignments for vertex %s",subset)
        t0 = time.time()
        assignments_container = domino.HeapAssignmentContainer(subset, k,
            self.restraint_cache, "my_heap_assignments_container" )
        neighbors = self.merge_tree.get_out_neighbors(vertex)
        if len(neighbors) == 0: # A leaf
            # Fill the container with the assignments for the leaf
            self.sampler.load_vertex_assignments(vertex, assignments_container)
        else:
            if neighbors[0] > neighbors[1]:
                # get_vertex_assignments() methods in domino
                # expects the children in sorted order
                neighbors =[neighbors[1], neighbors[0]]
            # recurse on the two children
            container_child0 = self.get_assignments_with_heap(neighbors[0], k)
            container_child1 = self.get_assignments_with_heap(neighbors[1], k)
            self.sampler.load_vertex_assignments(vertex,
                            container_child0,
                            container_child1,
                            assignments_container)
        tf = time.time() - t0
        log.info("Merge tree vertex: %s assignments: %s Time %s sec.", subset,
                        assignments_container.get_number_of_assignments(), tf)
        return assignments_container

    def get_restraint_value_for_assignment(self, assignment, name):
        """
            Recover the value of a restraint
            @param name of the restraint
            @param assignment Assignment class containing the assignment for
                        the solution desired
        """
        if not self.assignments_sampling_done:
            raise ValueError("DominoModel: sampling not done")
        log.debug("Computing restraint value for assignment %s",assignment)
        self.load_state(assignment)
        for rb in self.components_rbs:
            log.debug("rb - %s",
                            rb.get_reference_frame().get_transformation_to())
        val = self.restraints[name].evaulate(False)
        log.debug("restraint %s = %s",restraint.get_name(), val)
        return val

    def set_native_assembly_for_benchmark(self, params):
        """
            Sets the native model for benchmark, by reading the native
            structure and set the rigid bodies.
            @param fn_pdb_native PDB with the native structure
            @param anchored List of True/False values indicating
                if the components of the assembly are anchored. The function
                sets the FIRST anchored component in the (0,0,0) coordinate
                and moves the other components with the same translation
        """
        self.measure_models = True
        self.native_model = IMP.Model()
        if hasattr(params.benchmark, "fn_pdb_native"):
            self.native_assembly = \
                representation.create_assembly_from_pdb(self.native_model,
                              params.benchmark.fn_pdb_native, params.names)
        elif hasattr(params.benchmark, "fn_pdbs_native"):
            self.native_assembly = \
                representation.create_assembly(self.native_model,
                          params.benchmark.fn_pdbs_native, params.names)
        else:
            raise ValueError("set_native_assembly_for_benchmark: Requested " \
                "to set a native assembly for benchmark but did not provide " \
                "its pdb, or the pdbs of the components" )
        self.native_rbs = representation.create_rigid_bodies(self.native_assembly)
        anchor_assembly(self.native_rbs, params.anchor)


    def set_rb_states(self, rb, transformations):
        """
            Add a set of reference frames as possible states for a rigid body
            @param rb The rigid body
            @param transformations Transformations are used to build the
                                      reference frames for the rigid body.
        """
        log.info("Set rigid body states for %s",rb.get_name())
        RFs = [ alg.ReferenceFrame3D(T) for T in transformations ]
        for st in RFs:
            log.debug("%s",st)
        rb_states = domino.RigidBodyStates(RFs)
        self.rb_states_table.set_particle_states(rb, rb_states)
        st = self.rb_states_table.get_particle_states(rb)
        log.info("RigidBodyStates created %s",
                                    st.get_number_of_particle_states())

    def add_restraint_score_filter_table(self):
        """
            Add a RestraintScoreSubsetFilterTable to DOMINO that allows to
            reject assignments that have score worse than the thresholds for
            the restraints
        """
        log.info("Adding RestraintScoreSubsetFilterTable")
        # Restraint Cache is a centralized container of the restraints and
        # their scores
        self.restraint_cache = domino.RestraintCache(self.rb_states_table)
        self.restraint_cache.add_restraints(self.model.get_restraints())
        rssft = domino.RestraintScoreSubsetFilterTable(self.restraint_cache)
        rssft.set_name('myRestraintScoreSubsetFilterTable')
        self.sampler.add_subset_filter_table(rssft)

    def set_assembly_components(self, fn_pdbs, names):
        """
            Sets the components of an assembly, each one given as a separate
            PDB file.
            @param fn_pdbs List with the names of the pdb files
            @param names List with the names of the components of the assembly.
        """
        if len(fn_pdbs) != len(names):
            raise ValueError("Each PDB file needs a name")
        self.names = names
        self.assembly = representation.create_assembly(self.model, fn_pdbs,
                                                                        names)
        self.components_rbs = representation.create_rigid_bodies(self.assembly)
        self.not_optimized_rbs = []

    def set_assembly(self, fn_pdb, names):
        """
            Sets an assembly from a single PDB file. The function assumes that
            the components of the assembly are the chains of the PDB file.
            @param fn_pdb PDB with the file for the asembly
            @param names Names for each of the components (chains)
                of the asembly
        """
        self.assembly = representation.create_assembly_from_pdb(self.model,
                                                             fn_pdb,
                                                             names)
        self.components_rbs = representation.create_rigid_bodies(self.assembly)
        self.not_optimized_rbs = []


    def add_branch_and_bound_assignments_table(self):
        """
            BranchAndBoundAssignmentsTable enumerate states based on provided
            filtered using the provided the subset filter tables
        """
        log.info("Setting BranchAndBoundAssignmentsTable")
        fts = []
        for i in range(self.sampler.get_number_of_subset_filter_tables()):
            ft = self.sampler.get_subset_filter_table(i)
            fts.append(ft)
        at = domino.BranchAndBoundAssignmentsTable(self.rb_states_table, fts)
        self.sampler.set_assignments_table(at)
        self.assignments_table = at

    def add_exclusion_filter_table(self):
        """
           ExclusionSubsetFilterTable ensures that two particles are not given
           the same state at the same time
        """
        log.info("Setting ExclusionSubsetFilterTable")
        ft = domino.ExclusionSubsetFilterTable(self.rb_states_table)
        self.sampler.add_subset_filter_table(ft)

    def setup_domino_sampler(self):
        """
            Creates a DOMINO sampler and adds the required filter tables
        """
        if self.merge_tree == None:
            raise ValueError("Merge tree for DOMINO not set. It is required " \
                              "to setup the sampler")
        log.info("Domino sampler")
        self.sampler = domino.DominoSampler(self.model, self.rb_states_table)
        self.sampler.set_log_level(IMP.base.TERSE)
        self.sampler.set_merge_tree(self.merge_tree)
        self.add_exclusion_filter_table()
        self.add_restraint_score_filter_table()
        self.add_branch_and_bound_assignments_table()

    def read_merge_tree(self, fn):
        """
            Read and create a merge tree from a file.
            The format of the file is the format written by write merge_tree()
            It does not matter the order of the indices in the file, as
            they are sorted by this function.
            The only condition is that the index for the vertex that is the
            root of the tree MUST be the largest. This is guaranteed when
            creating a merge tree with create_merge_tree()
            @param fn File containing the merge tree
        """
        log.info("Reading merge tree from %s", fn)
        ps = self.rb_states_table.get_particles()
        log.debug("particles")
        for p in ps:
            log.debug("%s", p.get_name())
        rows = csv_related.read_csv(fn, delimiter = field_delim)
        for i in range(len(rows)):
            row = rows[i]
            rows[i] = [int(row[0]), int(row[1]), int(row[2]), row[3]]
        # Sort rows by vertice index
        rows.sort()
        subsets = []
        # build subsets from the rows text
        for row in rows:
            names = row[3].split(unit_delim)
            log.debug("row %s names %s", row, names)
            # get particles in the subset
            particles = []
            for name in names:
                l = filter(lambda p: p.get_name() == name, ps)
                if (len(l) > 1):
                    ValueError("More than one particle with same name" % names)
                particles.extend(l)
            s = domino.Subset(particles)
            subset_names = [p.get_name() for p in particles]
            log.debug("Merge tree Subset %s. Particles %s ",s, subset_names)
            subsets.append(s)
        # The vertex with the largest index is the root.
        # trick: get the merge tree object from a tree with only one node ...
        jt = domino.SubsetGraph()
        jt.add_vertex(subsets[0])
        mt = domino.get_merge_tree(jt)
        # ... and add the rest of the vertices
        for i in range(1,len(subsets)):
            mt.add_vertex(subsets[i]) # the name of the vertex is a subset
        # set edges
        for row in rows:
            vid = row[0]
            child_left = row[1]
            child_right = row[2]
            if child_left != -1:
                mt.add_edge(vid, child_left)
            if child_right != -1:
                mt.add_edge(vid, child_right)
        self.merge_tree = mt
        log.info("%s",self.merge_tree.show_graphviz() )


    def create_merge_tree(self):
        """
            Creates a merge tree from the restraints that are set already
        """
        rs = self.model.get_restraints()
        ig = domino.get_interaction_graph(rs, self.rb_states_table)
#        pruned_dep = IMP.get_pruned_dependency_graph(self.model)
#        IMP.base.show_graphviz(pruned_dep)
#        IMP.base.show_graphviz(ig)
        jt = domino.get_junction_tree(ig)
#        IMP.base.show_graphviz(jt)
        self.merge_tree = domino.get_balanced_merge_tree(jt)
#        IMP.base.show_graphviz(self.merge_tree)
        log.info("Balanced merge tree created")
        log.info("%s",self.merge_tree.show_graphviz() )


    def write_merge_tree(self, fn):
        """
            Writes the current merge tree to file. The format is:
            parent | child_left | child_right | labels
            @param fn File for storing the merge tree
        """
        log.info("Writing merge to %s", fn)
        if self.merge_tree == None:
            raise ValueError("No merge tree")
        f = open(fn, "w")
        f.write(
          "# Vertex index | Child0 | Child1 | label (names of the particles)\n")
        w = csv.writer(f, delimiter = field_delim)
        root = self.merge_tree.get_vertices()[-1]
        self.write_subset(root, w)
        f.close()


    def write_subset(self, v, w):
        """
            Writes lines describing a subset (vertex of a merge tree)
            @param w A csv.writer
            @param v Vertex index
        """
        no_child_index = -1
        subset = self.merge_tree.get_vertex_name(v)
        names = [p.get_name() for p in subset]
        reps = filter(lambda x: names.count(x) > 1, names) # repeated names
        if(len(reps)):
            raise ValueError("The names of the "\
                        "particles in the subset %s are not unique" % subset)
        names = unit_delim.join(names)
        neighbors = self.merge_tree.get_out_neighbors(v)
        if len(neighbors) == 0: # A leaf
            # Fill the container with the assignments for the leaf
            w.writerow([v,no_child_index,no_child_index, names])
        else:
            if neighbors[0] > neighbors[1]:
                neighbors =[neighbors[1], neighbors[0]]
            w.writerow([v,neighbors[0],neighbors[1], names])
            self.write_subset(neighbors[0], w)
            self.write_subset(neighbors[1], w)


    def set_connectivity_restraint(self, names, rname,
                                   distance=10,
                                   weight=1.0, max_score=None, n_pairs=1,
                                   stddev=2):
        """
            Set a connectivity restraint between the elements of the assembly
            @param names List with all the elements to be connected
            @param distance  Maximum distance tolerated by the restraint
            @param rname Name for the restraint
            @param weight Weight for the restraint
            @param max_score Maximum score for the restraint
            @param n_pairs Number of pairs of particles used in the
                        KClosePairScore of the connecitivity restraint
            @param stddev Standard deviation used to approximate the
                                HarmonicUpperBound function to a Gaussian
        """
        components = []
        for name in names:
            c = representation.get_component(self.coarse_assembly,name)
            components.append(c)
        log.info("Connectivity restraint for %s",components)
        spring_constant = core.Harmonic.get_k_from_standard_deviation(stddev)
        if max_score == None:
            max_score = weight * spring_constant * n_pairs * (stddev)**2
        r = restraints.get_connectivity_restraint(components, distance, n_pairs,
                                       spring_constant)
        self.add_restraint(r, rname, weight, max_score)

    def set_em2d_restraint(self, name, fn_images_sel, pixel_size, resolution,
                         n_projections, weight, max_score=False,
                        mode="fast", n_optimized=1):
        """
            Set a Restraint computing the em2d score.
            @param name Name for the restraint
            @param fn_images_sel Selection file with the names of the images
                            used for the restraint
            @param pixel_size - pixel size in the images
            @param resolution - resolution used to downsample the projections
                    of the model when projecting
            @param weight Weight of the restraint
            @param max_score - Maximum value tolerated for the restraint
            @param n_projections - Number of projections to generate
                    when projecting the model to do the coarse alignments
            @param mode - Mode used for computing the restraints.
            @param n_optimized - number of results from the coarse
                alignment that are refined with the Simplex algorithm
                to get a more accurate value for the restraint
        """
        log.info("Adding a em2D restraint: %s", fn_images_sel)
        restraint_params = em2d.Em2DRestraintParameters(pixel_size,
                                                           resolution,
                                                           n_projections)
        r = restraints.get_em2d_restraint( self.assembly,
                                     fn_images_sel,
                                     restraint_params,
                                     mode, n_optimized)
        self.add_restraint(r, name, weight, max_score)


    def set_not_optimized(self, name):
        """
            Set a part of the model as not optimized (it does not move during
            the model optimization)
            @param Name of the component to optimized
        """
        if name not in self.names:
            raise ValueError("DominoModel: There is not component " \
                            "in the assembly with this name")
        rb_name = representation.get_rb_name(name)
        self.not_optimized_rbs.append(rb_name)


    def set_close_pairs_excluded_volume_restraint(self, distance=10,
                                   weight=1.0, ratio=0.05, stddev=2,
                            max_score=False ):
        """
            Creates an excluded volume restraint between all the components
            of the coarse assembly.See help for
            @param distance Maximum distance tolerated between particles
            @param weight Weight for the restraint
            @param max_score Maximum value for the restraint. If the parameter
                is None, an automatic value is computed (using the ratio).
            @param ratio Fraction of the number of possible pairs of
                  particles that are tolerated to overlap. The maximum
                  score is modified according to this ratio.
                  I have found that ratios of 0.05-0.1 work well allowing for
                  some interpenetration
            @param stddev Standard deviation used to approximate the
                   HarmonicUpperBound function to a Gaussian
        """
        k = core.Harmonic.get_k_from_standard_deviation(stddev)
        for i, c1 in enumerate(self.coarse_assembly.get_children()):
            for j, c2 in enumerate(self.coarse_assembly.get_children()):
                if j > i:
                    name = "exc_vol_%s_%s" % (c1.get_name(), c2.get_name())

                    ls1 = atom.get_leaves(c1)
                    ls2 = atom.get_leaves(c2)
                    possible_pairs = len(ls1) * len(ls2)
                    n_pairs = possible_pairs * ratio

                    marker1 = IMP.Particle(self.model, "marker1 " + name)
                    marker2 = IMP.Particle(self.model, "marker2 " + name)
                    table_refiner = core.TableRefiner()
                    table_refiner.add_particle(marker1, ls1)
                    table_refiner.add_particle(marker2, ls2)
                    score = core.HarmonicLowerBound(distance, k)
                    pair_score = core.SphereDistancePairScore(score)
                    close_pair_score = core.ClosePairsPairScore(pair_score,
                                                                table_refiner,
                                                                distance)
                    r = core.PairRestraint(close_pair_score,
                                           IMP.ParticlePair(marker1,marker2))

                    if not max_score:
                        minimum_distance_allowed = 0
                        max_score = weight * n_pairs * \
                            score.evaluate(minimum_distance_allowed)
                    log.debug("Setting close_pairs_excluded_volume_restraint(), " \
                          "max_score %s", max_score)
                    self.add_restraint(r, name, weight, max_score)

    def set_pair_score_restraint(self, name1, name2,
                        restraint_name, distance=10,
                        weight=1.0, n_pairs = 1, stddev=2, max_score=None):
        """
            Set a pair_score restraint between the coarse representation
            of two components
            @param name1 Name of the first component
            @param name2 Name of the second component
            @param restraint_name Name for the restraint
            @param distance Maximum distance tolerated between particles
            @param weight. Weight of the restraint
            @param max_score Maximum value tolerated for the restraint
            @param stddev Standard deviation used to approximate the
                HarmonicUpperBound function to a Gaussian
        """
        table_refiner = core.TableRefiner()

        # The particles in A are attached to a marker particle via a refiner.
        # When the refiner gets a request for marker1, it returns the attached
        # particles
        A = representation.get_component(self.coarse_assembly, name1)
        marker1 = IMP.Particle(self.model, "marker1 "+restraint_name)
        table_refiner.add_particle(marker1, atom.get_leaves(A))
        # same for B
        B = representation.get_component(self.coarse_assembly, name2)
        marker2 = IMP.Particle(self.model, "marker2 "+restraint_name)
        table_refiner.add_particle(marker2, atom.get_leaves(B))

        k = core.Harmonic.get_k_from_standard_deviation(stddev)
        score = core.HarmonicUpperBoundSphereDistancePairScore(distance, k)
        # The score is set for the n_pairs closest pairs of particles
        pair_score = core.KClosePairsPairScore(score, table_refiner, n_pairs)
        #  When KClosePairsPairScore is called, the refiner will provide the
        # particles for A and B
        if not max_score:
            # Build a maximum score based on the function type that is used,
            # an HarmonicUpperBound
            temp_score = core.HarmonicUpperBound(distance, k)
            error_distance_allowed = 10
            max_score = weight * n_pairs * \
                        temp_score.evaluate(distance + error_distance_allowed)

        log.info("Setting pair score restraint for %s %s. k = %s, max_score " \
            "= %s, stddev %s", name1, name2, k, max_score,stddev)
        r = core.PairRestraint(pair_score, IMP.ParticlePair( marker1, marker2 ) )
        self.add_restraint(r, restraint_name, weight, max_score)


    def write_solutions_database(self, fn_database, max_number=None):
        """
            Write the results of the DOMINO sampling to a SQLite database.
            @param max_number Maximum number of results to write
        """
        log.info("Creating the database of solutions")
        if self.measure_models == True and not hasattr(self, "native_model"):
            raise ValueError("The native model has not been set")
        db = solutions_io.ResultsDB()
        db.create(fn_database, overwrite=True)
        db.connect(fn_database)
        subset = self.rb_states_table.get_subset()
        restraints_names = self.get_restraints_names_used(subset)
        db.add_results_table(restraints_names, self.measure_models)
        n = len(self.solution_assignments)
        if max_number != None:
            n = min(n, max_number)
        for sol_id, assignment in enumerate(self.solution_assignments[0:n]):
            txt = self.get_assignment_text(assignment, subset)
            # load the positions of the components in the state
            self.load_state(assignment)
            RFs = [rb.get_reference_frame() for rb in self.components_rbs]
            scores, total_score = self.get_assignment_scores(assignment, subset)
            measures = None
            if(self.measure_models):
                measures = measure_model(self.assembly, self.native_assembly,
                        self.components_rbs, self.native_rbs)
            db.add_record(sol_id, txt, RFs, total_score, scores,  measures)
        # Add native state if doing benchmarking
        if self.measure_models:
            assignment = "native"
            RFs = [rb.get_reference_frame() for rb in self.native_rbs]
            scores, total_score = self.get_restraints_for_native(
                                                            restraints_names)
            db.add_native_record(assignment, RFs, total_score, scores)
        db.save_records()
        db.close()

    def get_restraints_names_used(self, subset):
        """ Get the names of the restraints used for a subset
            @param subset Subset of components of the assembly
        """
        return [r.get_name() for r
                            in self.restraint_cache.get_restraints(subset, [])]

    def get_restraints_for_native(self, restraints_names):
        """
            Get values of the restraints for the native structure
            @param restraints_names Names of the restraints requested
            @return a list with the values of all scores, and the total score
        """
        saved = [rb.get_reference_frame() for rb in self.components_rbs]
        # Set the state of the model to native
        for rb,rn  in zip(self.components_rbs, self.native_rbs):
            # remove sub-rigid bodies
            rb_members = filter(
                           lambda m: not core.RigidBody.particle_is_instance(
                                     m.get_particle()), rb.get_members())
            rn_members =  filter(
                           lambda m: not core.RigidBody.particle_is_instance(
                                     m.get_particle()), rn.get_members())
            rb_coords = [m.get_coordinates() for m in rb_members]
            rn_coords = [m.get_coordinates() for m in rn_members]

            # align and put rb in the position of rn
            if len(rn_coords) != len(rb_coords) :
                raise ValueError("Mismatch in the number of members. "
                "Reference %d Aligned %d " % (len(rn_coords), len(rb_coords)))
            T = alg.get_transformation_aligning_first_to_second(rb_coords,
                                                                rn_coords)
            t = rb.get_reference_frame().get_transformation_to()
            new_t =  alg.compose(T, t)
            rb.set_reference_frame(alg.ReferenceFrame3D(new_t))
        scores = []
        for name in restraints_names:
            scores.append( self.restraints[name].evaluate(False))
        total_score = sum(scores)
        representation.set_reference_frames(self.components_rbs, saved)
        return scores, total_score

    def write_pdb_for_assignment(self, assignment, fn_pdb):
        """
            Write the solution in the assignment
            @param assignment Assignment class with the states for the subset
            @param fn_pdb File to write the model
        """
        if not self.assignments_sampling_done:
            raise ValueError("DominoModel: sampling not done")
        log.info("Writting PDB %s for assignment %s",fn_pdb, assignment)
        self.load_state(assignment)
        atom.write_pdb(self.assembly, fn_pdb)

    def write_pdb_for_configuration(self, n, fn_pdb):
        """
            Write a file with a configuration for the model (configuration
            here means a configuration in DOMINO)
            @param n Index of the configuration desired.
        """
        if not self.configuration_sampling_done:
            raise ValueError("DominoModel: sampling not done")
        log.debug("Writting PDB %s for configuration %s", fn_pdb, n)
        configuration_set.load_configuration(n)
        atom.write_pdb(self.assembly, fn_pdb)

    def write_pdb_for_reference_frames(self,RFs, fn_pdb):
        """
            Write a PDB file with a solution given by a set of reference frames
            @param RFs Reference frames for the elements of the complex
            @param fn_pdb File to write the solution
        """
        log.debug("Writting PDB %s for reference frames %s",fn_pdb, RFs)
        representation.set_reference_frames(self.components_rbs,RFs)
        atom.write_pdb(self.assembly, fn_pdb)

    def write_pdbs_for_reference_frames(self,RFs, fn_base):
        """
            Write a separate PDB for each of the elements
            @param RFs Reference frames for the elements of the complex
            @param fn_base base string to buid the names of the PDBs files
        """
        log.debug("Writting PDBs with basename %s",fn_base)
        representation.set_reference_frames(self.components_rbs,RFs)
        for i,ch in enumerate(self.assembly.get_children()):
            atom.write_pdb(ch, fn_base + "component-%02d.pdb" % i )

    def write_pdb_for_component(self, component_index, ref,  fn_pdb):
        """
            Write one component of the assembly
            @param component_index Position of the component in the assembly
            @param ref Reference frame of the component
            @param fn_pdb Output PDB
        """
        ("Writting PDB %s for component %s",fn_pdb, component_index)
        self.components_rbs[component_index].set_reference_frame(ref)
        hierarchy_component = self.assembly.get_child(component_index)
        atom.write_pdb(hierarchy_component, fn_pdb)


    def write_monte_carlo_solution(self, fn_database):
        """
            Write the solution of a MonteCarlo run
            @param fn_database Database file with the solution.
                                The database will contain only one record
        """
        total_score = 0
        rnames = []
        scores = []
        for i in range(self.model.get_number_of_restraints()):
            r = self.model.get_restraint(i)
            score = r.evaluate(False)
            rnames.append(r.get_name())
            scores.append(score)
            total_score += score
        db = solutions_io.ResultsDB()
        db.create(fn_database, overwrite=True)
        db.connect(fn_database)
        db.add_results_table(rnames, self.measure_models)
        RFs = [ rb.get_reference_frame() for rb in self.components_rbs]
        measures = None
        if(self.measure_models):
            measures = measure_model(self.assembly, self.native_assembly,
                                     self.components_rbs, self.native_rbs)
        solution_id = 0
        db.add_record(solution_id, "", RFs, total_score, scores, measures)
        db.save_records()
        db.close()

def print_restraints_values(model):
    print "Restraints: Name, weight, value, maximum_value"
    total_score = 0
    for i in range(model.get_number_of_restraints()):
        r = model.get_restraint(i)
        score = r.evaluate(False)
#        print "%20s %18f %18f %18f" % (r.get_name(), r.get_weight(),
#                                                score, r.get_maximum_score())
        print "%20s %18f %18f" % (r.get_name(), r.get_weight(), score)
        total_score += score
    print "total_score:",total_score


def anchor_assembly(components_rbs, anchored):
    """
        "Anchor" a set of rigid bodies, by setting the position of one of them
        at the origin (0,0,0).
        @param components_rbs Rigid bodies of the components
        @param anchored - List of True/False values indicating if the components
      of the assembly are anchored. The function sets the FIRST anchored
      component in the (0,0,0) coordinate and moves the other
      components with the same translation. If all the values are False, the
      function does not do anything
    """
    if True in anchored:
        anchored_rb = None
        for a, rb  in zip(anchored, components_rbs) :
            if a:
                anchored_rb = rb
                break
        center = anchored_rb.get_coordinates()
        log.info("Anchoring assembly at (0,0,0)")
        for rb in components_rbs:
            T = rb.get_reference_frame().get_transformation_to()
            t = T.get_translation()
            R = T.get_rotation()
            log.debug("%s: Rerefence frame BEFORE %s",
                                        rb.get_name(),rb.get_reference_frame())
            T2 = alg.Transformation3D(R, t - center)
            rb.set_reference_frame(alg.ReferenceFrame3D(T2))
            log.debug("%s Rerefence frame AFTER %s",
                                        rb.get_name(),rb.get_reference_frame())

def measure_model(assembly, native_assembly, rbs, native_rbs):
    """
        Get the drms, cdrms and crmsd for a model
        @param assembly Hierachy for an assembly
        @param native_assembly Hierarchy of the native structure
        @param rbs Rigid bodies for the components of the assembly
        @param native_rbs Rigid bodies for the components of the native
                    structure
        @return cdrms, cdrms, crmsd
        - drms - typical drms measure
        - cdrms - drms for the centroids of the rigid bodies of the components
          of the assembly
        - crmsd - rmsd for the centroids
    """
    log.debug("Measure model")
    drms = comparisons.get_drms_for_backbone(assembly, native_assembly)
    RFs = [rb.get_reference_frame() for rb in rbs]
    vs = [r.get_transformation_to().get_translation() for r in RFs]
    nRFs = [rb.get_reference_frame() for rb in native_rbs]
    nvs = [r.get_transformation_to().get_translation()  for r in nRFs]
    cdrms = atom.get_drms(vs, nvs)
    crmsd, RFs =  alignments.align_centroids_using_pca(RFs, nRFs)
    log.debug("drms %s cdrms %s crmsd %s",drms,cdrms,crmsd)
    return [drms, cdrms, crmsd]



def get_coordinates(rigid_bodies):
    """
        Return a list of the coordinates of all the members of the rigid bodies
    """
    if len(rigid_bodies) == 0:
        raise ValueError("get_coordinates: There are not rigid bodies to get coordinates from")
    coords = []
    for rb in rigid_bodies:
            # remove sub-rigid bodies
        rb_members = filter(
                       lambda m: not core.RigidBody.particle_is_instance(
                                 m.get_particle()), rb.get_members())
        coords.extend([m.get_coordinates() for m in rb_members])
    return coords
