"""@namespace IMP.pmi.dof
   Handling of degrees of freedom.
"""

from __future__ import print_function

class dof(object):
    """Degrees of Freedom.
    Tasks:
    define rigid bodies and super rigid bodies
    define symmetries (see representation.py)
    create nuisances and their parameters
    generate lists of MC or MD movers
    """
    def __init__(self,root,
                 maxtrans_rb=None,maxrot_rb=None,
                 maxtrans_srb=None,maxrot_srb=None):
        self.rigid_bodies = []
        self.fixed_rigid_bodies = []
        self.floppy_bodies = []
        self.super_rigid_bodies = []

        self.maxtrans_rb = 2.0
        self.maxrot_rb = 0.04
        self.maxtrans_srb = 2.0
        self.maxrot_srb = 0.2
        self.rigidbodiesarefixed = False
        self.maxtrans_fb = 3.0
        self.mdl = mdl

    def set_rigid_body_from_hierarchies(self, hiers, particles=None):
        '''
        This method allows the construction of a rigid body given a list
        of hierarchies and or a list of particles.

        hiers:         list of hierarchies
        particles:     (optional, default=None) list of particles to add to the rigid body
        '''

        if particles is None:
            rigid_parts = set()
        else:
            rigid_parts = set(particles)

        name = ""
        print("set_rigid_body_from_hierarchies> setting up a new rigid body")
        for hier in hiers:
            ps = IMP.atom.get_leaves(hier)
            for p in ps:
                if IMP.core.RigidMember.get_is_setup(p):
                    rb = IMP.core.RigidMember(p).get_rigid_body()
                    print("set_rigid_body_from_hierarchies> WARNING particle %s already belongs to rigid body %s" % (p.get_name(), rb.get_name()))
                else:
                    rigid_parts.add(p)
            name += hier.get_name() + "-"
            print("set_rigid_body_from_hierarchies> adding %s to the rigid body" % hier.get_name())
        rb = IMP.atom.create_rigid_body(list(rigid_parts))
        rb.set_coordinates_are_optimized(True)
        rb.set_name(name + "rigid_body")
        self.rigid_bodies.append(rb)
        return rb

    def set_rigid_bodies(self, subunits):
        '''
        This method allows the construction of a rigid body given a list
        of tuples, that identify the residue ranges and the subunit names (the names used
        to create the component by using create_component.

        subunits: [(name_1,(first_residue_1,last_residue_1)),(name_2,(first_residue_2,last_residue_2)),.....]
                  or
                  [name_1,name_2,(name_3,(first_residue_3,last_residue_3)),.....]

                   example: ["prot1","prot2",("prot3",(1,10))]

        sometimes, we know about structure of an interaction
        and here we make such PPIs rigid
        '''

        rigid_parts = set()
        for s in subunits:
            if type(s) == type(tuple()) and len(s) == 2:
                sel = IMP.atom.Selection(
                    self.prot,
                    molecule=s[0],
                    residue_indexes=list(range(s[1][0],
                                          s[1][1] + 1)))
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exists")
                for p in sel.get_selected_particles():
                    # if not p in self.floppy_bodies:
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        print("set_rigid_body_from_hierarchies> WARNING particle %s already belongs to rigid body %s" % (p.get_name(), rb.get_name()))
                    else:
                        rigid_parts.add(p)

            elif type(s) == type(str()):
                sel = IMP.atom.Selection(self.prot, molecule=s)
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exists")
                for p in sel.get_selected_particles():
                    # if not p in self.floppy_bodies:
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        print("set_rigid_body_from_hierarchies> WARNING particle %s already belongs to rigid body %s" % (p.get_name(), rb.get_name()))
                    else:
                        rigid_parts.add(p)

        rb = IMP.atom.create_rigid_body(list(rigid_parts))
        rb.set_coordinates_are_optimized(True)
        rb.set_name(''.join(str(subunits)) + "_rigid_body")
        self.rigid_bodies.append(rb)
        return rb

    def set_super_rigid_body_from_hierarchies(
        self,
        hiers,
        axis=None,
            min_size=0):
        # axis is the rotation axis for 2D rotation
        super_rigid_xyzs = set()
        super_rigid_rbs = set()
        name = ""
        print("set_super_rigid_body_from_hierarchies> setting up a new SUPER rigid body")

        for hier in hiers:
            ps = IMP.atom.get_leaves(hier)
            for p in ps:
                if IMP.core.RigidMember.get_is_setup(p):
                    rb = IMP.core.RigidMember(p).get_rigid_body()
                    super_rigid_rbs.add(rb)
                else:
                    super_rigid_xyzs.add(p)
            print("set_rigid_body_from_hierarchies> adding %s to the rigid body" % hier.get_name())
        if len(super_rigid_rbs) < min_size:
            return
        if axis is None:
            self.super_rigid_bodies.append((super_rigid_xyzs, super_rigid_rbs))
        else:
            # these will be 2D rotation SRB
            self.super_rigid_bodies.append(
                (super_rigid_xyzs, super_rigid_rbs, axis))

    def fix_rigid_bodies(self, rigid_bodies):
        self.fixed_rigid_bodies += rigid_bodies

    def set_chain_of_super_rigid_bodies(
        self,
        hiers,
        min_length=None,
        max_length=None,
            axis=None):
        '''
        this function takes a linear list of hierarchies (they are supposed
         to be sequence-contiguous) and
        produces a chain of super rigid bodies with given length range, specified
        by min_length and max_length
        '''
        try:
            hiers = IMP.pmi.tools.flatten_list(hiers)
        except:
            pass
        for hs in IMP.pmi.tools.sublist_iterator(hiers, min_length, max_length):
            self.set_super_rigid_body_from_hierarchies(hs, axis, min_length)

    def set_super_rigid_bodies(self, subunits, coords=None):
        super_rigid_xyzs = set()
        super_rigid_rbs = set()

        for s in subunits:
            if type(s) == type(tuple()) and len(s) == 3:
                sel = IMP.atom.Selection(
                    self.prot,
                    molecule=s[2],
                    residue_indexes=list(range(s[0],
                                          s[1] + 1)))
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exists")
                for p in sel.get_selected_particles():
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        super_rigid_rbs.add(rb)
                    else:
                        super_rigid_xyzs.add(p)
            elif type(s) == type(str()):
                sel = IMP.atom.Selection(self.prot, molecule=s)
                if len(sel.get_selected_particles()) == 0:
                    print("set_rigid_bodies: selected particle does not exists")
                for p in sel.get_selected_particles():
                    # if not p in self.floppy_bodies:
                    if IMP.core.RigidMember.get_is_setup(p):
                        rb = IMP.core.RigidMember(p).get_rigid_body()
                        super_rigid_rbs.add(rb)
                    else:
                        super_rigid_xyzs.add(p)
        self.super_rigid_bodies.append((super_rigid_xyzs, super_rigid_rbs))

    def remove_floppy_bodies(self, hierarchies):
        '''
        give a list of hierarchies, get the leaves and remove the corresponding particles
        from the floppy bodies list. We need this function because sometimes
        we want to constraint the floppy bodies in a rigid body. For instance
        when you want to associate a bead with a density particle.
        '''
        for h in hierarchies:
            ps = IMP.atom.get_leaves(h)
            for p in ps:
                if p in self.floppy_bodies:
                    print("remove_floppy_bodies: removing %s from floppy body list" % p.get_name())
                    self.floppy_bodies.remove(p)

    def set_floppy_bodies(self):
        for p in self.floppy_bodies:
            name = p.get_name()
            p.set_name(name + "_floppy_body")
            if IMP.core.RigidMember.get_is_setup(p):
                print("I'm trying to make this particle flexible although it was assigned to a rigid body", p.get_name())
                rb = IMP.core.RigidMember(p).get_rigid_body()
                try:
                    rb.set_is_rigid_member(p.get_index(), False)
                except:
                    # some IMP versions still work with that
                    rb.set_is_rigid_member(p.get_particle_index(), False)
                p.set_name(p.get_name() + "_rigid_body_member")

    def set_floppy_bodies_from_hierarchies(self, hiers):
        for hier in hiers:
            ps = IMP.atom.get_leaves(hier)
            for p in ps:
                IMP.core.XYZ(p).set_coordinates_are_optimized(True)
                self.floppy_bodies.append(p)

    def get_particles_to_sample(self):
        # get the list of samplable particles with their type
        # and the mover displacement. Everything wrapped in a dictionary,
        # to be used by samplers modules
        ps = {}

        # remove symmetric particles: they are not sampled
        rbtmp = []
        fbtmp = []
        srbtmp = []
        if not self.rigidbodiesarefixed:
            for rb in self.rigid_bodies:
                if IMP.pmi.Symmetric.get_is_setup(rb):
                    if IMP.pmi.Symmetric(rb).get_symmetric() != 1:
                        rbtmp.append(rb)
                else:
                    if rb not in self.fixed_rigid_bodies:
                        rbtmp.append(rb)

        for fb in self.floppy_bodies:
            if IMP.pmi.Symmetric.get_is_setup(fb):
                if IMP.pmi.Symmetric(fb).get_symmetric() != 1:
                    fbtmp.append(fb)
            else:
                fbtmp.append(fb)

        for srb in self.super_rigid_bodies:
            # going to prune the fixed rigid bodies out
            # of the super rigid body list
            rigid_bodies = list(srb[1])
            filtered_rigid_bodies = []
            for rb in rigid_bodies:
                if rb not in self.fixed_rigid_bodies:
                    filtered_rigid_bodies.append(rb)
            srbtmp.append((srb[0], filtered_rigid_bodies))

        self.rigid_bodies = rbtmp
        self.floppy_bodies = fbtmp
        self.super_rigid_bodies = srbtmp

        ps["Rigid_Bodies_SimplifiedModel"] = (
            self.rigid_bodies,
            self.maxtrans_rb,
            self.maxrot_rb)
        ps["Floppy_Bodies_SimplifiedModel"] = (
            self.floppy_bodies,
            self.maxtrans_fb)
        ps["SR_Bodies_SimplifiedModel"] = (
            self.super_rigid_bodies,
            self.maxtrans_srb,
            self.maxrot_srb)
        return ps
