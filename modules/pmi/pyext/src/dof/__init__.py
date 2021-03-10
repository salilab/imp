"""@namespace IMP.pmi.dof
   Create movers and set up constraints for PMI objects.
   See the documentation of the DegreesOfFreedom class for more information.
"""

from __future__ import print_function
import IMP
import IMP.atom
import IMP.algebra
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.samplers
import IMP.pmi.tools
import IMP.pmi.samplers
import warnings


def create_rigid_body_movers(dof, maxtrans, maxrot):
    mvs = []
    for rb in dof.rigid_bodies:
        mvs.append(IMP.core.RigidBodyMover(rb.get_model(), rb,
                                           maxtrans, maxrot))
    return mvs


class DegreesOfFreedom(object):
    """Simplify creation of constraints and movers for an IMP Hierarchy.

       * The various "create X" functions make movers for system components
         as well as set up necessary constraints. For each of these functions,
         you can generally pass PMI objects like
         [Molecule](@ref IMP::pmi::topology::Molecule) or slices thereof.
       * DegreesOfFreedom.create_rigid_body() lets you rigidify a molecule
         (but allows you to also pass "nonrigid" components which move with
         the body and also independently).
       * DegreesOfFreedom.create_super_rigid_body() sets up a special
         "Super Rigid Body" which moves rigidly but is not always constrained
         to be rigid (so you can later move the parts separately). This is
         good for speeding up sampling.
       * DegreesOfFreedom.create_flexible_beads() sets up particles to move
         individually.
       * DegreesOfFreedom.setup_md() sets up particles to move with molecular
         dynamics. Note that this is not (yet) compatible with rigid bodies,
         and only works with some restraints.
       * DegreesOfFreedom.constrain_symmetry() makes a symmetry constraint so
         that clones automatically move with their references. If instead you
         want a softer restraint, check out the
         [SymmetryRestraint](@ref IMP::pmi::restraints::stereochemistry::SymmetryRestraint).
       * When you are done you can access all movers with
         DegreesOfFreedom.get_movers(). If you have set up rigid, super rigid,
         or flexible beads, pass the movers to the `monte_carlo_sample_objects`
         argument of
         [ReplicaExchange0](@ref IMP::pmi::macros::ReplicaExchange0).
       * If you are running MD, you have to separately pass the particles
         (also returned from DegreesOfFreedom.setup_md()) to the
         `molecular_dynamics_sample_objects` argument of
         [ReplicaExchange0](@ref IMP::pmi::macros::ReplicaExchange0). Check
         out an [MD example here](pmi_2atomistic_8py-example.html).
    """  # noqa: E501
    def __init__(self, model):
        self.model = model
        self.movers = []
        self.fb_movers = []  # stores movers corresponding to floppy parts
        self.rigid_bodies = []  # stores rigid body objects
        # stores all beads including nonrigid members of rigid bodies
        self.flexible_beads = []
        self.nuisances = []
        # Keys are the RigidBody objects, values are list of movers
        self._rb2mov = {}
        # the following is needed to keep track of disabled movers
        self.movers_particles_map = IMP.pmi.tools.OrderedDict()
        self.movers_rb_map = {}
        self.movers_xyz_map = {}
        self.disabled_movers = []

    def _get_nonrigid_hiers(self, nonrigid_parts, rigid_hiers, resolution):
        """Get Hierarchy objects for nonrigid parts. Make sure that they are
           a subset of the rigid body Hierarchies."""
        if not nonrigid_parts:
            return
        nr_hiers = IMP.pmi.tools.input_adaptor(nonrigid_parts, resolution,
                                               flatten=True)
        if nr_hiers:
            rb_idxs = set(h.get_particle_index() for h in rigid_hiers)
            for h in nr_hiers:
                p = h.get_particle()
                if p.get_index() not in rb_idxs:
                    raise ValueError(
                                "You tried to create nonrigid members from "
                                "particles that aren't in the RigidBody!")
        return nr_hiers

    def create_rigid_body(self, rigid_parts, nonrigid_parts=None,
                          max_trans=4.0, max_rot=0.5, nonrigid_max_trans=4.0,
                          resolution='all', name=None):
        """Create rigid body constraint and mover
        @param rigid_parts Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue, a list/set
               (of list/set) of them or a RigidBody object.
               Must be uniform input, however. No mixing object types.
        @param nonrigid_parts Same input format as rigid_parts.
               Must be a subset of rigid_parts particles.
        @param max_trans Maximum rigid body translation
        @param max_rot Maximum rigid body rotation
        @param nonrigid_max_trans Maximum step for the nonrigid (bead)
               particles
        @param resolution Only used if you pass PMI objects. Probably you
               want 'all'.
        @param name Rigid body name (if None, use IMP default)
        @return (rb_movers,rb_object)
        @note If you want all resolutions, pass PMI objects because this
              function will get them all. Alternatively you can do your
              selection elsewhere and just pass hierarchies.
        """

        rb_movers = []

        # ADD CHECK: these particles are not already part of some RB or SRB

        # First, is this already a rigid body?
        if type(rigid_parts) is IMP.core.RigidBody:
            warnings.warn("Rigid Body Already Setup", IMP.pmi.ParameterWarning)
            rb = rigid_parts
            model = rb.get_model()
            if name is None:
                name = rb.get_name()
            hiers = [IMP.atom.get_leaves(IMP.atom.Hierarchy(
                model.get_particle(i)))[0]
                for i in rb.get_member_particle_indexes()]
        else:
            # Otherwise, setup RB
            hiers = IMP.pmi.tools.input_adaptor(rigid_parts,
                                                resolution,
                                                flatten=True)

            if not hiers:
                warnings.warn(
                        "No hierarchies were passed to create_rigid_body()",
                        IMP.pmi.ParameterWarning)
                return []

        # Need to do this before rigid body is set up so that we leave the
        # system in a consistent state if a sanity check fails
        nr_hiers = self._get_nonrigid_hiers(nonrigid_parts, hiers, resolution)

        if type(rigid_parts) is not IMP.core.RigidBody:
            model = hiers[0].get_model()

            # we need to use the following constructor because the
            # IMP.core.create_rigid_body seems to construct an arbitrary
            # reference frame, which will be different for all copies.
            # therefore, symmetry won't work all the time

            com = IMP.atom.CenterOfMass.setup_particle(IMP.Particle(model),
                                                       hiers)
            comcoor = IMP.core.XYZ(com).get_coordinates()
            tr = IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(), comcoor)
            rf = IMP.algebra.ReferenceFrame3D(tr)
            rbp = IMP.Particle(model)
            rb = IMP.core.RigidBody.setup_particle(rbp, rf)
            for h in hiers:
                rb.add_member(h.get_particle())

        self.rigid_bodies.append(rb)
        rb.set_coordinates_are_optimized(True)
        rb_mover = IMP.core.RigidBodyMover(rb.get_model(), rb, max_trans,
                                           max_rot)
        if name is not None:
            rb.set_name(name)
            rb_mover.set_name(name)
        rb_movers.append(rb_mover)
        self.movers_particles_map[rb_mover] = []
        self.movers_rb_map[rb_mover] = [rb]
        rb_mover.set_was_used(True)
        for h in hiers:
            self.movers_particles_map[rb_mover] += IMP.atom.get_leaves(h)
        # setup nonrigid parts
        if nr_hiers:
            floatkeys = [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]
            for h in nr_hiers:
                self.flexible_beads.append(h)
                p = h.get_particle()
                rb.set_is_rigid_member(p.get_index(), False)
                for fk in floatkeys:
                    p.set_is_optimized(fk, True)
                fbmv = IMP.core.BallMover(p.get_model(), p,
                                          IMP.FloatKeys(floatkeys),
                                          nonrigid_max_trans)
                self.fb_movers.append(fbmv)
                self.movers_particles_map[fbmv] = IMP.atom.get_leaves(h)
                self.movers_xyz_map[fbmv] = IMP.atom.get_leaves(h)
                fbmv.set_was_used(True)
                rb_movers.append(fbmv)

        self.movers += rb_movers  # probably need to store more info
        self._rb2mov[rb] = rb_movers  # dictionary relating rb to movers

        return rb_movers, rb

    def create_main_chain_mover(self, molecule, resolution=10,
                                lengths=[5, 10]):
        """Create crankshaft moves from a set of SUPER rigid body mover
           from one molecule.
        See http://scfbm.biomedcentral.com/articles/10.1186/1751-0473-3-12
        """
        hiers = IMP.pmi.tools.input_adaptor(molecule, resolution, flatten=True)

        for length in lengths:
            for n in range(len(hiers)-length):
                hs = hiers[n+1:n+length]
                self.create_super_rigid_body(
                    hs, max_trans=0.0, max_rot=0.05,
                    axis=(hiers[n].get_particle(),
                          hiers[n+length].get_particle()))

    def create_super_rigid_body(self, srb_parts, max_trans=1.0, max_rot=0.1,
                                chain_min_length=None, chain_max_length=None,
                                resolution='all', name=None, axis=None):
        """Create SUPER rigid body mover from one or more hierarchies.

           Can also create chain of SRBs. If you don't pass chain min/max,
           it'll treat everything you pass as ONE rigid body.
           If you DO pass chain min/max, it'll expect srb_parts is a list
           and break it into bits.
           @param srb_parts Can be one of the following inputs:
                  IMP Hierarchy, PMI System/State/Molecule/TempResidue,
                  or a list/set (of list/set) of them.
                  Must be uniform input, however. No mixing object types.
           @param max_trans Maximum super rigid body translation
           @param max_rot Maximum super rigid body rotation
           @param chain_min_length Create a CHAIN of super rigid bodies -
                  must provide list; this parameter is the minimum
                  chain length.
           @param chain_max_length Maximum chain length
           @param resolution Only used if you pass PMI objects. Probably you
                  want 'all'.
           @param name The name of the SRB (hard to assign a good one
                  automatically)
           @param axis A tuple containing two particles which are used to
                  compute the rotation axis of the SRB. The default is None,
                  meaning that the rotation axis is random.

           @note If you set the chain parameters, will NOT create an SRB from
                 all of them together, but rather in groups made from the
                 outermost list.
        """

        srb_movers = []

        # organize hierarchies based on chains
        if chain_min_length is None and chain_max_length is None:
            # the "chain" is just everything together
            h = IMP.pmi.tools.input_adaptor(srb_parts, resolution,
                                            flatten=True)
            if len(h) == 0:
                warnings.warn(
                    'No hierarchies were passed to create_super_rigid_body()',
                    IMP.pmi.ParameterWarning)
                return srb_movers
            srb_groups = [h]
        else:
            if not hasattr(srb_parts, '__iter__'):
                raise Exception("You tried to make a chain without a list!")
            srb_groups = [IMP.pmi.tools.input_adaptor(
                h, resolution, flatten=True, warn_about_slices=False)
                for h in srb_parts]

        # create SRBs either from all hierarchies or chain
        if chain_min_length is None and chain_max_length is None:
            mv = self._setup_srb(srb_groups, max_trans, max_rot, axis)
            if mv:
                mv.set_was_used(True)
                srb_movers.append(mv)
        elif chain_min_length is not None and chain_max_length is not None:
            for hs in IMP.pmi.tools.sublist_iterator(
                    srb_groups, chain_min_length, chain_max_length):
                mv = self._setup_srb(hs, max_trans, max_rot, axis)
                if mv:
                    mv.set_was_used(True)
                    srb_movers.append(mv)
        else:
            raise Exception(
                "DegreesOfFreedom: SetupSuperRigidBody: if you want "
                "chain, specify min AND max")
        self.movers += srb_movers
        if name is not None:
            if len(srb_movers) > 1:
                for n, mv in enumerate(srb_movers):
                    mv.set_name(name + '_' + str(n))
            else:
                srb_movers[0].set_name(name)
        return srb_movers

    def _setup_srb(self, hiers, max_trans, max_rot, axis):
        if axis is None:
            srbm = IMP.pmi.TransformMover(
                hiers[0][0].get_model(), max_trans, max_rot)
        else:
            srbm = IMP.pmi.TransformMover(
                hiers[0][0].get_model(), axis[0], axis[1], max_trans, max_rot)
        srbm.set_was_used(True)
        super_rigid_rbs, super_rigid_xyzs \
            = IMP.pmi.tools.get_rbs_and_beads(hiers)
        ct = 0
        self.movers_particles_map[srbm] = []
        for h in hiers:
            self.movers_particles_map[srbm] += IMP.atom.get_leaves(h)
        for xyz in super_rigid_xyzs:
            srbm.add_xyz_particle(xyz)
            ct += 1
        for rb in super_rigid_rbs:
            srbm.add_rigid_body_particle(rb)
            ct += 1
        if ct > 1:
            return srbm
        else:
            return 0

    def create_flexible_beads(self, flex_parts, max_trans=3.0,
                              resolution='all'):
        """Create a chain of flexible beads

           @param flex_parts Can be one of the following inputs:
                  IMP Hierarchy, PMI System/State/Molecule/TempResidue,
                  or a list/set (of list/set) of them.
                  Must be uniform input, however. No mixing object types.
           @param max_trans Maximum flexible bead translation
           @param resolution Only used if you pass PMI objects. Probably
                  you want 'all'.
        """

        fb_movers = []
        hiers = IMP.pmi.tools.input_adaptor(flex_parts,
                                            resolution,
                                            flatten=True)
        if not hiers or len(hiers) == 0:
            warnings.warn(
                'No hierarchies were passed to create_flexible_beads()',
                IMP.pmi.ParameterWarning)
            return fb_movers
        for h in hiers:
            p = h.get_particle()
            IMP.core.XYZ(p).set_coordinates_are_optimized(True)
            if IMP.core.RigidMember.get_is_setup(h) \
                    or IMP.core.NonRigidMember.get_is_setup(h):
                raise Exception(
                    "Cannot create flexible beads from members of rigid body")
            self.flexible_beads.append(h)
            fbmv = IMP.core.BallMover(p.get_model(), p, max_trans)
            fb_movers.append(fbmv)
            fbmv.set_was_used(True)
            self.fb_movers.append(fbmv)
            self.movers_particles_map[fbmv] = IMP.atom.get_leaves(h)
            self.movers_xyz_map[fbmv] = IMP.atom.get_leaves(h)
        self.movers += fb_movers
        return fb_movers

    def create_nuisance_mover(self, nuisance_p, step_size, name=None):
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
        mv.set_was_used(True)
        self.nuisances.append(nuisance_p)
        self.movers.append(mv)
        return [mv]

    def setup_md(self, hspec):
        """Setup particles for MD simulation. Returns all particles, just
        pass this to molecular_dynamics_sample_objects in ReplicaExchange0.
        @param hspec Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue,
               or a list/set (of list/set) of them.
               Must be uniform input, however. No mixing object types.
        """
        vxkey = IMP.FloatKey('vx')
        vykey = IMP.FloatKey('vy')
        vzkey = IMP.FloatKey('vz')
        hiers = IMP.pmi.tools.input_adaptor(hspec, flatten=True)
        model = hiers[0].get_model()
        all_ps = []
        for hl in hiers:
            for h in IMP.core.get_leaves(hl):
                p = h.get_particle()
                pxyz = IMP.core.XYZ(model, p.get_index())
                pxyz.set_coordinates_are_optimized(True)
                model.add_attribute(vxkey, p.get_index(), 0.0)
                model.add_attribute(vykey, p.get_index(), 0.0)
                model.add_attribute(vzkey, p.get_index(), 0.0)
                all_ps.append(p)
        return all_ps

    def constrain_symmetry(self, references, clones, transform,
                           resolution='all', type="AXIAL"):
        """Create a symmetry constraint. Checks:
        same number of particles
        disable ANY movers involving symmetry copies
         (later may support moving them WITH references,
         but requires code to propagate constraint)
        @param references Can be one of the following inputs:
               IMP Hierarchy, PMI System/State/Molecule/TempResidue,
               or a list/set (of list/set) of them
        @param clones Same format as references
        @param transform The transform that moves a clone onto a reference
               IMP.algebra.Transformation3D
        @param resolution Only used if you pass PMI objects.
               If you have a multires system, assuming each are rigid
               bodies you probably only need one resolution.
        @param type of symmetry. Implemented = AXIAL, RIGID_BODY
        """

        # get all RBs and particles
        href = IMP.pmi.tools.input_adaptor(references, resolution,
                                           flatten=True)
        hclones = IMP.pmi.tools.input_adaptor(clones, resolution, flatten=True)

        ref_rbs, ref_beads = IMP.pmi.tools.get_rbs_and_beads(href)
        clones_rbs, clones_beads = IMP.pmi.tools.get_rbs_and_beads(hclones)

        for ref, clone in zip(ref_rbs, clones_rbs):
            IMP.core.Reference.setup_particle(clone, ref)

        for ref, clone in zip(ref_beads, clones_beads):
            IMP.core.Reference.setup_particle(clone, ref)

        # removing movers involved in clones
        self.disable_movers(hclones)

        if type == "AXIAL":
            sm = IMP.core.TransformationSymmetry(transform)

        if type == "RIGID_BODY":
            p = IMP.Particle(self.model)
            p.set_name("RigidBody_Symmetry")
            rb = IMP.core.RigidBody.setup_particle(
                p, IMP.algebra.ReferenceFrame3D(transform))
            for cp in [(10, 0, 0), (0, 10, 0), (0, 0, 10)]:
                p = IMP.Particle(self.model)
                IMP.core.XYZ.setup_particle(p, cp)
                rb.add_member(p)
            sm = IMP.core.TransformationSymmetry(rb.get_particle_index())
            self.rigid_bodies.append(rb)
            rb.set_coordinates_are_optimized(True)
            rb_mover_tr = IMP.core.RigidBodyMover(
                rb.get_model(), rb.get_particle_index(), 0.0, 1.0)
            rb_mover_rt = IMP.core.RigidBodyMover(
                rb.get_model(), rb.get_particle_index(), 10.0, 0.0)

            rb_mover_tr.set_name("RigidBody_Symmetry_Mover_Translate")
            rb_mover_rt.set_name("RigidBody_Symmetry_Mover_Rotate")
            print('Created rigid body symmetry restraint')
            self.movers_particles_map[rb_mover_tr] = []
            self.movers_particles_map[rb_mover_rt] = []
            self.movers_rb_map[rb_mover_tr] = [rb]
            self.movers_rb_map[rb_mover_rt] = [rb]
            for h in hclones:
                self.movers_particles_map[rb_mover_tr] \
                    += IMP.atom.get_leaves(h)
                self.movers_particles_map[rb_mover_rt] \
                    += IMP.atom.get_leaves(h)
            self.movers.append(rb_mover_tr)  # probably need to store more info
            self.movers.append(rb_mover_rt)  # probably need to store more info
            # dictionary relating rb to movers
            self._rb2mov[rb] = [rb_mover_tr, rb_mover_rt]

        lsc = IMP.container.ListSingletonContainer(
            self.model,
            [bead.get_particle().get_index()
             for bead in clones_rbs+clones_beads])
        c = IMP.container.SingletonsConstraint(sm, None, lsc)
        self.model.add_score_state(c)
        print('Created symmetry restraint for', len(ref_rbs),
              'rigid bodies and', len(ref_beads), 'flexible beads')
        self.model.update()

    def __repr__(self):
        # would like something fancy like this:
        # - super-rigid "SRB1"
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
            pts.add_particle(fb, "Floppy_Bodies", 1.0,
                             "Flexible_Bead_" + str(n))
        if len(pts.get_particles_to_sample()) > 0:
            mc = IMP.pmi.samplers.MonteCarlo(self.model, [pts], temperature)
            print("optimize_flexible_beads: optimizing %i flexible beads"
                  % len(self.get_flexible_beads()))
            mc.optimize(nsteps)
        else:
            print("optimize_flexible_beads: no particle to optimize")

    def get_movers(self):
        """Returns Enabled movers"""
        if self.disabled_movers:
            filtered_mover_list = []
            for mv in self.movers:
                if mv not in self.disabled_movers:
                    filtered_mover_list.append(mv)
            return filtered_mover_list
        else:
            return self.movers

    def get_floppy_body_movers(self):
        """Return all movers corresponding to individual beads"""
        return self.fb_movers

    def get_rigid_bodies(self):
        """Return list of rigid body objects"""
        return self.rigid_bodies

    def get_flexible_beads(self):
        "Return all flexible beads, including nonrigid members of rigid bodies"
        return self.flexible_beads

    def disable_movers(self, objects, mover_types=None):
        """Fix the position of the particles by disabling the corresponding
           movers.
        @param objects Can be one of the following inputs:
            IMP Hierarchy, PMI System/State/Molecule/TempResidue, or a
            list/set (of list/set) of them.
            Must be uniform input, however. No mixing object types.
        @param mover_types further filter the mover type that will be
            disabled; it can be a list of IMP.core.RigidBodyMover,
            IMP.core.BallMover etc etc if one wants to fix the corresponding
            rigid body, or the floppy bodies.
            An empty mover_types list is interpreted as all possible movers.
        It returns the list of fixed xyz particles (ie, floppy bodies/beads)
        and rigid bodies whose movers were disabled
        """
        hierarchies = IMP.pmi.tools.input_adaptor(objects,
                                                  pmi_resolution='all',
                                                  flatten=True)
        tmp_set = set()
        fixed_rb = set()
        fixed_xyz = set()
        if mover_types is None:
            mover_types = []

        inv_map = {}
        for mv, ps in self.movers_particles_map.items():
            for p in ps:
                if p in inv_map:
                    inv_map[p].append(mv)
                else:
                    inv_map[p] = [mv]

        for h in hierarchies:
            if h in inv_map:
                for mv in inv_map[h]:
                    if (type(mv) in mover_types or not mover_types):
                        tmp_set.add(mv)
                        if mv in self.movers_rb_map:
                            fixed_rb |= set(self.movers_rb_map[mv])
                        if mv in self.movers_xyz_map:
                            fixed_xyz |= set(self.movers_xyz_map[mv])
        print("Fixing %s movers" % (str(len(list(tmp_set)))))
        self.disabled_movers += list(tmp_set)
        return list(fixed_xyz), list(fixed_rb)

    def enable_all_movers(self):
        """Reenable all movers: previously fixed particles will be released"""
        self.disabled_movers = []

    def get_nuisances_from_restraint(self, r):
        """Extract the nuisances from get_particles_to_sample()"""
        try:
            pslist = r.get_particles_to_sample()
        except AttributeError:
            raise Exception(
                "dof.get_nuisances_from_restraint(): the passed object "
                "does not have a get_particles_to_sample() function")
        for name in pslist:
            is_sampled = True
            if len(pslist[name]) == 3:
                ps, maxtrans, is_sampled = pslist[name]
            else:
                ps, maxtrans = pslist[name]
            if is_sampled:
                self.create_nuisance_mover(ps[0], maxtrans, name)
