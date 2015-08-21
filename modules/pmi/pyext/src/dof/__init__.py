"""@namespace IMP.pmi.dof
   Setup constraints and create movers for an IMP Hierarchy
"""

from __future__ import print_function
import IMP
import IMP.atom
import IMP.algebra
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.samplers
import IMP.pmi.tools
import itertools

class DegreesOfFreedom(object):
    """A class to simplify create of constraints and movers for an IMP Hierarchy.
    Call the various create() functions to get started.
    Can get all enabled movers with get_movers(). Pass this to ReplicaExchange0.
    """
    def __init__(self,mdl):
        self.mdl = mdl
        self.movers = []       # should store mover, underlying bodies, enabled?
        self.particle_map = {} # map from particles/rb objects to relevant movers+constraints
        # internal mover  = [mover obj, list of particles, enabled?] ?
        # mover map = {particles/rbs : movers}

    def create_rigid_body(self,
                          body,
                          nonrigid_parts = None,
                          max_trans=1.0,
                          max_rot=0.1,
                          nonrigid_max_trans = 0.1,
                          name=None):
        """Create rigid body constraint and mover
        @param body           Can be one of the following inputs:
                              IMP Selection, Hierarchy,
                              PMI Molecule, Residue, or a list/set
        @param nonrigid_parts Same input format as body.
                              Must be a subset of body particles.
        @param max_trans      Maximum rigid body translation
        @param max_rot        Maximum rigid body rotation
        @param name           Rigid body name (if None, use IMP default)
        \note if you pass a PMI::Molecule, PMI::Residue, or list/set of them,
        will automatically get all representations and resolutions
        """

        rb_movers = []

        # ADD CHECK: these particles are not already part of some RB or SRB

        ### setup RB
        hiers = IMP.pmi.tools.get_hierarchies_from_spec(body)
        if not hiers:
            print("WARNING: No hierarchies were passed to create_rigid_body()")
            return []
        rb = IMP.atom.create_rigid_body(hiers)
        rb.set_coordinates_are_optimized(True)
        rb_mover = IMP.core.RigidBodyMover(rb,max_trans,max_rot)
        if name is not None:
            rb.set_name(name)
        rb_movers.append(rb_mover)

        ### setup nonrigid parts
        if nonrigid_parts:
            nr_hiers = IMP.pmi.tools.get_hierarchies_from_spec(nonrigid_parts)
            if nr_hiers:
                floatkeys = [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]
                rb_idxs = set(rb.get_member_indexes())
                for h in nr_hiers:
                    p = h.get_particle()
                    if not p.get_index() in rb_idxs:
                        raise Exception("You tried to create nonrigid members from "
                                         "particles that aren't in the RigidBody!")

                    rb.set_is_rigid_member(p.get_index(),False)
                    for fk in floatkeys:
                        p.set_is_optimized(fk,True)
                    rb_movers.append(IMP.core.BallMover([p],
                                                        IMP.FloatKeys(floatkeys),
                                                        nonrigid_max_trans))
        self.movers += rb_movers # probably need to store more info
        return rb_movers

    def create_super_rigid_body(self,
                                hspec,
                                max_trans=1.0,
                                max_rot=0.1,
                                chain_min_length=None,
                                chain_max_length=None):
        """Create SUPER rigid body mover from one or more hierarchies. Can also create chain of SRBs.
        @param hspec             Can be one of the following inputs:
                                 IMP Selection, Hierarchy,
                                 PMI Molecule, Residue, or a list/set
        @param max_trans         Maximum super rigid body translation
        @param max_rot           Maximum super rigid body rotation
        @param chain_min_length  Create a CHAIN of super rigid bodies - must provide list
                                 This parameter is the minimum chain length.
        @param chain_max_length  max chain length
        \note if you pass a PMI::Molecule, Residue, or list/set of them,
              will automatically get all representations and resolutions
        """

        srb_movers = []

        ## organize hierarchies based on chains
        if chain_min_length is None and chain_max_length is None:
            # the "chain" is just everything together
            h = IMP.pmi.tools.get_hierarchies_from_spec(hspec)
            if len(h)==0:
                print('WARNING: No hierarchies were passed to create_super_rigid_body()')
                return srb_movers
            hiers = [h]
        else:
            if not hasattr(hspec,'__iter__'):
                raise Exception("You tried to make a chain without a list!")
            hiers = [IMP.pmi.tools.get_hierarchies_from_spec(h) for h in hspec]

        ## create SRBs either from all hierarchies or chain
        if chain_min_length is None and chain_max_length is None:
            srb_movers.append(self._setup_srb(hiers,max_trans,max_rot))
        elif chain_min_length is not None and chain_max_length is not None:
            for hs in IMP.pmi.tools.sublist_iterator(hiers, chain_min_length, chain_max_length):
                srb_movers.append(self._setup_srb(hs,max_trans,max_rot))
        else:
            raise Exception("DegreesOfFreedom: SetupSuperRigidBody: if you want chain, specify min AND max")
        self.movers += srb_movers
        return srb_movers

    def _setup_srb(self,hiers,max_trans,max_rot):
        srbm = IMP.pmi.TransformMover(hiers[0][0].get_model(), max_trans, max_rot)
        super_rigid_rbs,super_rigid_xyzs = IMP.pmi.tools.get_rbs_and_beads(hiers)
        for xyz in super_rigid_xyzs:
            srbm.add_xyz_particle(xyz)
        for rb in super_rigid_rbs:
            srbm.add_rigid_body_particle(rb)
        return srbm


    def create_flexible_beads(self,
                              hspec,
                              max_trans):
        """Create a chain of flexible beads
        @param hspec             Can be one of the following inputs:
                                 IMP Selection, Hierarchy,
                                 PMI Molecule, Residue, or a list/set
        @param max_trans         Maximum flexible bead translation
        """

        fb_movers = []
        hiers = IMP.pmi.tools.get_hierarchies_from_spec(hspec)
        if not hiers:
            print('WARNING: No hierarchies were passed to create_flexible_beads()')
            return fb_movers
        for p in IMP.pmi.tools.get_all_leaves(hiers):
            if IMP.core.RigidMember.get_is_setup(p) or IMP.core.NonRigidMember.get_is_setup(p):
                raise Exception("Cannot create flexible beads from members of rigid body")
            fb_movers.append(IMP.core.BallMover([p],max_trans))
        self.movers += fb_movers
        return fb_movers

    def create_nuisance_mover(self,
                              ps = None,
                              pmi_restraint = None):
        """either create nuisance or pass ISD restraint with fixed interface"""

    def constrain_symmetry(self,
                           references,
                           clones,
                           transform):
        """Create a symmetry constraint. Checks:
        same number of particles
        disable ANY movers involving symmetry copies
         (later may support moving them WITH references,
         but requires code to propagate constraint)
        @param references    Can be one of the following inputs:
                                 IMP Selection, Hierarchy,
                                 PMI Molecule, Residue, or a list/set
        @param clones        Same format as references
        @param transform     How to transform clones onto references
                             IMP.algebra.Transformation3D
        """

        # get all RBs and particles
        href    = IMP.pmi.tools.get_hierarchies_from_spec(references)
        hclones = IMP.pmi.tools.get_hierarchies_from_spec(clones)
        ref_rbs,ref_beads = IMP.pmi.tools.get_rbs_and_beads(href)
        clones_rbs,clones_beads = IMP.pmi.tools.get_rbs_and_beads(hclones)

        # dumb check for matching numbers of particles
        if len(ref_rbs)!=len(clones_rbs) or len(ref_beads)!=len(clones_beads):
            raise Exception("ERROR: Your references don't match your clones")

        # symmetry RBs
        for ref,clone in zip(ref_rbs+ref_beads,clones_rbs+clones_beads):
            IMP.core.Reference.setup_particle(clone,ref)
        sm = IMP.core.TransformationSymmetry(transform)
        lsc = IMP.container.ListSingletonContainer(
            self.mdl,[p.get_particle_index() for p in clones_rbs+clones_beads])
        c = IMP.container.SingletonsConstraint(sm, None, lsc)
        self.mdl.add_score_state(c)
        print('Created symmetry restraint for',len(ref_rbs),'rigid bodies and',
              len(ref_beads),'flexible beads')

    def __repr__(self):
        # would like something fancy like this:
        #- super-rigid "SRB1"
        #  - rigid "Mol1" (8 rigid, 3 nonrigid)
        #  - rigid "Mol2" (8 rigid, 3 nonrigid)
        #  - rigid "Mol3" (8 rigid, 3 nonrigid)
        return 'DegreesOfFreedom: ' + \
          "\n".join(repr(m) for m in self.movers)

    def get_movers(self):
        """Should only return Enabled movers?"""
        return self.movers
