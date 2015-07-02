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
    Call the various create() functions to get started."""
    def __init__(self,mdl):
        self.mdl = mdl
        self.movers = []

    def create_rigid_body(self,
                          hspec,
                          max_trans=1.0,
                          max_rot=0.1,
                          name=None):
        """Create rigid body constraint and mover
        @param hspec          Can be one of the following inputs:
                              IMP Selection, Hierarchy,
                              PMI Molecule, Residue, or a list/set
        @param max_trans     Maximum rigid body translation
        @param max_rot       Maximum rigid body rotation
        @param name          Rigid body name (if None, use IMP default)
        \note if you pass a PMI::Molecule, Residue, or list/set of them,
              will automatically get all representations and resolutions
        """
        hs = IMP.pmi.tools.get_hierarchies_from_spec(hspec)
        setup_rb = SetupRigidBody(hs,max_trans,max_rot,name)
        self.movers.append(setup_rb)
        return setup_rb

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
        if chain_min_length is None and chain_max_length is None:
            hiers = [IMP.pmi.tools.get_hierarchies_from_spec(hspec)]
        else:
            if not hasattr(hspec,'__iter__'):
                raise Exception("You tried to make a chain without a list!")
            hiers = [IMP.pmi.tools.get_hierarchies_from_spec(h) for h in hspec]
        setup_srb = SetupSuperRigidBody(hiers,max_trans,max_rot,chain_min_length,chain_max_length)
        self.movers.append(setup_srb)
        return setup_srb

    def create_flexible_beads(self,
                              hspec,
                              max_trans):
        """Create a chain of flexible beads
        @param hspec             Can be one of the following inputs:
                                 IMP Selection, Hierarchy,
                                 PMI Molecule, Residue, or a list/set
        @param max_trans         Maximum flexible bead translation
        """
        hiers = IMP.pmi.tools.get_hierarchies_from_spec(hspec)
        setup_fb = SetupFlexibleBeads(hiers,max_trans)
        self.movers.append(setup_fb)
        return setup_fb


    def show(self):
        """Make it pretty"""

    def get_all_movers(self):
        # possibly organize in some way?
        all_movers = []
        for m in self.movers:
            all_movers+=m.get_movers()
        return all_movers

class SetupRigidBody(object):
    """Sets up and stores RigidBodyMover and BallMovers for NonRigidMembers"""
    def __init__(self,hiers,max_trans,max_rot,name):
        self.rb = IMP.atom.create_rigid_body(hiers)
        self.rb.set_coordinates_are_optimized(True)
        self.rb_mover = IMP.core.RigidBodyMover(self.rb,max_trans,max_rot)
        self.flexible_movers = []
        if name is not None:
            self.rb.set_name(name)

    def create_non_rigid_members(self,spec,max_trans=1.0):
        hiers = IMP.pmi.tools.get_hierarchies_from_spec(spec)
        floatkeys = [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]
        idxs = set(self.rb.get_member_indexes())
        for h in hiers:
            p = h.get_particle()
            if not p.get_index() in idxs:
                raise Exception("You tried to create nonrigid members from "
                                 "particles that aren't in the RigidBody!")

            self.rb.set_is_rigid_member(p.get_index(),False)
            for fk in floatkeys:
                p.set_is_optimized(fk,True)
            self.flexible_movers.append(IMP.core.BallMover([p],
                                                  IMP.FloatKeys(floatkeys),
                                                  max_trans))
    def get_movers(self):
        return [self.rb_mover]+self.flexible_movers

    def get_rigid_body(self):
        return self.rb

class SetupDiscreteRigidBody(SetupRigidBody):
    pass

class SetupSuperRigidBody(object):
    def __init__(self,hiers,max_trans,max_rot,chain_min_length,chain_max_length):
        """Expecting a list of lists of hiers"""
        self.movers = []
        if chain_min_length is None and chain_max_length is None:
            self._setup_srb(hiers,max_trans,max_rot)
        elif chain_min_length is not None and chain_max_length is not None:
            for hs in IMP.pmi.tools.sublist_iterator(hiers, chain_min_length, chain_max_length):
                self._setup_srb(hs,max_trans,max_rot)
        else:
            raise Exception("DegreesOfFreedom: SetupSuperRigidBody: if you want chain, specify min AND max")

    def _setup_srb(self,hiers,max_trans,max_rot):
        srbm = IMP.pmi.TransformMover(hiers[0][0].get_model(), max_trans, max_rot)
        super_rigid_xyzs = set()
        super_rigid_rbs = set()

        for p in IMP.pmi.tools.get_all_leaves(hiers):
            if IMP.core.RigidMember.get_is_setup(p):
                super_rigid_rbs.add(IMP.core.RigidMember(p).get_rigid_body())
            elif IMP.core.NonRigidMember.get_is_setup(p):
                super_rigid_rbs.add(IMP.core.NonRigidMember(p).get_rigid_body())
            else:
                super_rigid_xyzs.add(p)

        for xyz in super_rigid_xyzs:
            srbm.add_xyz_particle(xyz)
        for rb in super_rigid_rbs:
            srbm.add_rigid_body_particle(rb)
        self.movers.append(srbm)

    def get_movers(self):
        return self.movers

class SetupFlexibleBeads(object):
    def __init__(self,hiers,max_trans):
        self.movers = []
        for p in IMP.pmi.tools.get_all_leaves(hiers):
            if IMP.core.RigidMember.get_is_setup(p) or IMP.core.NonRigidMember.get_is_setup(p):
                raise Exception("Cannot create flexible beads from members of rigid body")
            self.movers.append(IMP.core.BallMover([p],max_trans))
    def get_movers(self):
        return self.movers
