"""@namespace IMP.pmi.dof
   Create movers and setup constraints for PMI objects.
* Start by creating the DegreesOfFreedom class with `dof = IMP::pmi::dof::DegreesOfFreedom(model)`
* The various "create X" functions make movers for system components as well as setup necessary constraints.
For each of these functions, you can generally pass PMI objects like [Molecule](@ref IMP::pmi::topology::Molecule) or slices thereof.
 * DegreesOfFreedom.create_rigid_body() lets you rigidify a molecule (but allows you to also pass "nonrigid" components which move with the body and also independently).
 * DegreesOfFreedom.create_super_rigid_body() sets up a special "Super Rigid Body" which moves
rigidly but is not always constrained to be rigid (so you can later move the parts separately). Good for speeding up sampling.
 * DegreesOfFreedom.create_flexible_beads() sets up particles to move individually.
 * DegreesOfFreedom.setup_md() sets up particles to move with molecular dynamics. Note that this is not (yet) compatible with rigid bodies, and only some restraints.
 * DegreesOfFreedom.constrain_symmetry() makes a symmetry constraint so that clones automatically move with their references. If instead you want a softer restraint, check out the [SymmetryRestraint](@ref IMP::pmi::restraints::stereochemistry::SymmetryRestraint).
* When you are done you can access all movers with DegreesOfFreedom.get_movers(). If you have set up rigid, super rigid, or flexible beads, pass the movers to the `monte_carlo_sample_objects` argument of [ReplicaExchange0](@ref IMP::pmi::macros::ReplicaExchange0).
* If you are running MD, you have to separately pass the particles (also returned from DegreesOfFreedom.setup_md()) to the `molecular_dynamics_sample_objects` argument of [ReplicaExchange0](@ref IMP::pmi::macros::ReplicaExchange0). Check out an [MD example here](pmi_2atomistic_8py-example.html).
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
import IMP.pmi.samplers

class DegreesOfFreedom(object):
    """A class to simplify create of constraints and movers for an IMP Hierarchy.
    Call the various create() functions to get started.
    Can get all enabled movers with get_movers(). Pass this to ReplicaExchange0.
    """
    def __init__(self,mdl):
        self.mdl = mdl
        self.movers = []
        self.rigid_bodies = [] #stores rigid body objects
        self.flexible_beads = [] # stores all beads including nonrigid members of rigid bodies
        self.nuisances = []

        #self.particle_map = {} # map from particles/rb objects to relevant movers+constraints
        # internal mover  = [mover obj, list of particles, enabled?] ?
        # mover map = {particles/rbs : movers}

    def create_rigid_body(self,
                          rigid_parts,
                          nonrigid_parts=None,
                          max_trans=1.0,
                          max_rot=0.1,
                          nonrigid_max_trans = 0.1,
                          resolution='all',
                          name=None):
        """Create rigid body constraint and mover
        @param rigid_parts Can be one of the following inputs:
           IMP Hierarchy, PMI System/State/Molecule/TempResidue, a list/set (of list/set) of them
           or a RigidBody object.
           Must be uniform input, however. No mixing object types.
        @param nonrigid_parts Same input format as rigid_parts.
               Must be a subset of rigid_parts particles.
        @param max_trans Maximum rigid body translation
        @param max_rot Maximum rigid body rotation
        @param nonrigid_max_trans Maximum step for the nonrigid (bead) particles
        @param resolution Only used if you pass PMI objects. Probably you want 'all'.
        @param name Rigid body name (if None, use IMP default)
        \note If you want all resolutions, pass PMI objects because this function will get them all.
        Alternatively you can do your selection elsewhere and just pass hierarchies.
        Returns tuple (rb_movers,rb_object)
        """

        rb_movers = []

        # ADD CHECK: these particles are not already part of some RB or SRB

        # First, is this already a rigid body?
        if type(rigid_parts) is IMP.core.RigidBody:
            rb = rigid_parts
            if name is not None:
                name = rb.get_name()
        else:
            ### Otherwise, setup RB
            hiers = IMP.pmi.tools.input_adaptor(rigid_parts,
                                                resolution,
                                                flatten=True)
            if not hiers:
                print("WARNING: No hierarchies were passed to create_rigid_body()")
                return []
            rb = IMP.atom.create_rigid_body(hiers)
        self.rigid_bodies.append(rb)
        rb.set_coordinates_are_optimized(True)
        rb_mover = IMP.core.RigidBodyMover(rb,max_trans,max_rot)
        if name is not None:
            rb.set_name(name)
            rb_mover.set_name(name)
        rb_movers.append(rb_mover)

        ### setup nonrigid parts
        if nonrigid_parts:
            nr_hiers = IMP.pmi.tools.input_adaptor(nonrigid_parts,
                                                   resolution,
                                                   flatten=True)
            if nr_hiers:
                floatkeys = [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]
                rb_idxs = set(rb.get_member_indexes())
                for h in nr_hiers:
                    self.flexible_beads.append(h)
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
        return rb_movers,rb

    def create_super_rigid_body(self,
                                srb_parts,
                                max_trans=1.0,
                                max_rot=0.1,
                                chain_min_length=None,
                                chain_max_length=None,
                                resolution='all',
                                name=None):
        """Create SUPER rigid body mover from one or more hierarchies. Can also create chain of SRBs.
        If you don't pass chain min/max, it'll treat everything you pass as ONE rigid body.
        If you DO pass chain min/max, it'll expect srb_parts is a list and break it into bits.
        @param srb_parts Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set (of list/set) of them.
               Must be uniform input, however. No mixing object types.
        @param max_trans Maximum super rigid body translation
        @param max_rot Maximum super rigid body rotation
        @param chain_min_length Create a CHAIN of super rigid bodies - must provide list
               This parameter is the minimum chain length.
        @param chain_max_length Maximum chain length
        @param resolution Only used if you pass PMI objects. Probably you want 'all'.
        @param name The name of the SRB (hard to assign a good one automatically)
        \note If you set the chain parameters, will NOT create an SRB from all of them together,
        but rather in groups made from the outermost list.
        """

        srb_movers = []

        ## organize hierarchies based on chains
        if chain_min_length is None and chain_max_length is None:
            # the "chain" is just everything together
            h = IMP.pmi.tools.input_adaptor(srb_parts,resolution,flatten=True)
            if len(h)==0:
                print('WARNING: No hierarchies were passed to create_super_rigid_body()')
                return srb_movers
            srb_groups = [h]
        else:
            if not hasattr(srb_parts,'__iter__'):
                raise Exception("You tried to make a chain without a list!")
            srb_groups = [IMP.pmi.tools.input_adaptor(h,resolution,flatten=True,
                                                      warn_about_slices=False) for h in srb_parts]

        ## create SRBs either from all hierarchies or chain
        if chain_min_length is None and chain_max_length is None:
            mv = self._setup_srb(srb_groups,max_trans,max_rot)
            if mv:
                srb_movers.append(mv)
        elif chain_min_length is not None and chain_max_length is not None:
            for hs in IMP.pmi.tools.sublist_iterator(srb_groups, chain_min_length, chain_max_length):
                mv = self._setup_srb(hs,max_trans,max_rot)
                if mv:
                    srb_movers.append(mv)
        else:
            raise Exception("DegreesOfFreedom: SetupSuperRigidBody: if you want chain, specify min AND max")
        self.movers += srb_movers
        if name is not None:
            if len(srb_movers)>1:
                for n,mv in enumerate(srb_movers):
                    mv.set_name(name+'_'+str(n))
            else:
                srb_movers[0].set_name(name)
        return srb_movers

    def _setup_srb(self,hiers,max_trans,max_rot):
        srbm = IMP.pmi.TransformMover(hiers[0][0].get_model(), max_trans, max_rot)
        super_rigid_rbs,super_rigid_xyzs = IMP.pmi.tools.get_rbs_and_beads(hiers)
        ct = 0
        for xyz in super_rigid_xyzs:
            srbm.add_xyz_particle(xyz)
            ct+=1
        for rb in super_rigid_rbs:
            srbm.add_rigid_body_particle(rb)
            ct+=1
        if ct>1:
            return srbm
        else:
            return 0


    def create_flexible_beads(self,
                              flex_parts,
                              max_trans=0.1,
                              resolution='all'):
        """Create a chain of flexible beads
        @param flex_parts Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set (of list/set) of them.
               Must be uniform input, however. No mixing object types.
        @param max_trans Maximum flexible bead translation
        @param resolution Only used if you pass PMI objects. Probably you want 'all'.
        """

        fb_movers = []
        hiers = IMP.pmi.tools.input_adaptor(flex_parts,
                                            resolution,
                                            flatten=True)
        if not hiers or len(hiers)==0:
            print('WARNING: No hierarchies were passed to create_flexible_beads()')
            return fb_movers
        for h in hiers:
            p = h.get_particle()
            IMP.core.XYZ(p).set_coordinates_are_optimized(True)
            if IMP.core.RigidMember.get_is_setup(h) or IMP.core.NonRigidMember.get_is_setup(h):
                raise Exception("Cannot create flexible beads from members of rigid body")
            self.flexible_beads.append(h)
            fb_movers.append(IMP.core.BallMover([p],max_trans))
        self.movers += fb_movers
        return fb_movers

    def create_nuisance_mover(self,
                              nuisance_p,
                              step_size,
                              name=None):
        """Create MC normal mover for nuisance particles.
        We will add an easier interface to add all of them from a PMI restraint
        @param nuisance_p The Nuisance particle (an ISD::Scale)
        @param step_size The maximum step size for Monte Carlo
        @param name The name of the mover, useful for better output reading.
        """
        mv = IMP.core.NormalMover([nuisance_p],
                                  IMP.FloatKeys([IMP.FloatKey("nuisance")]),
                                  step_size)
        if name is not None:
            mv.set_name(name)
        self.nuisances.append(nuisance_p)
        self.movers.append(mv)
        return [mv]

    def setup_md(self,
                 hspec):
        """Setup particles for MD simulation. Returns all particles, just
        pass this to molecular_dynamics_sample_objects in ReplicaExchange0.
        @param hspec Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set (of list/set) of them.
               Must be uniform input, however. No mixing object types.
        """
        vxkey = IMP.FloatKey('vx')
        vykey = IMP.FloatKey('vy')
        vzkey = IMP.FloatKey('vz')
        hiers = IMP.pmi.tools.input_adaptor(hspec,flatten=True)
        mdl = hiers[0].get_model()
        all_ps = []
        for h in hiers:
            p = h.get_particle()
            IMP.core.XYZ(mdl,p.get_index()).set_coordinates_are_optimized(True)
            mdl.add_attribute(vxkey,p.get_index(),0.0)
            mdl.add_attribute(vykey,p.get_index(),0.0)
            mdl.add_attribute(vzkey,p.get_index(),0.0)
            all_ps.append(p)
        return all_ps

    def constrain_symmetry(self,
                           references,
                           clones,
                           transform,
                           resolution=1):
        """Create a symmetry constraint. Checks:
        same number of particles
        disable ANY movers involving symmetry copies
         (later may support moving them WITH references,
         but requires code to propagate constraint)
        @param references Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a list/set (of list/set) of them
        @param clones Same format as references
        @param transform The transform that moves a clone onto a reference
               IMP.algebra.Transformation3D
        @param resolution Only used if you pass PMI objects.
               If you have a multires system, assuming each are rigid
               bodies you probably only need one resolution.
        """

        # get all RBs and particles
        href    = IMP.pmi.tools.input_adaptor(references,resolution,flatten=True)
        hclones = IMP.pmi.tools.input_adaptor(clones,resolution,flatten=True)

        ref_rbs,ref_beads = IMP.pmi.tools.get_rbs_and_beads(href)
        clones_rbs,clones_beads = IMP.pmi.tools.get_rbs_and_beads(hclones)

        # dumb check for matching numbers of particles
        if len(ref_rbs)!=len(clones_rbs) or len(ref_beads)!=len(clones_beads):
            raise Exception("ERROR: Your references don't match your clones")

        # symmetry RBs
        for ref,clone in zip(ref_rbs+ref_beads,clones_rbs+clones_beads):
            IMP.core.Reference.setup_particle(clone,ref)
        sm = IMP.core.TransformationSymmetry(transform.get_inverse())
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

    def optimize_flexible_beads(self, nsteps, temperature=1.0):
        '''Set up MC run with just flexible beads.
        Optimization works much better when restraints
        are already set up.'''
        pts = IMP.pmi.tools.ParticleToSampleList()
        for n, fb in enumerate(self.get_flexible_beads()):
            pts.add_particle(fb, "Floppy_Bodies", 1.0, "Flexible_Bead_" + str(n))
        if len(pts.get_particles_to_sample()) > 0:
            mc = IMP.pmi.samplers.MonteCarlo(self.mdl, [pts], temperature)
            print("optimize_flexible_beads: optimizing %i flexible beads" % len(self.get_flexible_beads()))
            mc.optimize(nsteps)
        else:
            print("optimize_flexible_beads: no particle to optimize")

    def get_movers(self):
        """Should only return Enabled movers?"""
        return self.movers

    def get_rigid_bodies(self):
        """Return list of rigid body objects"""
        return self.rigid_bodies

    def get_flexible_beads(self):
        """Return all flexible beads, including nonrigid members of rigid bodies"""
        return self.flexible_beads

    def get_nuisances_from_restraint(self,r):
        """Extract the nuisances from get_particles_to_sample()"""
        try:
            pslist = r.get_particles_to_sample()
        except:
            raise Exception("dof.get_nuisances_from_restraint(): the passed object does not have a "
                            "get_particles_to_sample() function")
        for name in pslist:
            is_sampled = True
            if len(pslist[name])==3:
                ps,maxtrans,is_sampled = pslist[name]
            else:
                ps,maxtrans = pslist[name]
            if is_sampled:
                self.create_nuisance_mover(ps[0],maxtrans,name)
