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
import itertools

def get_hierarchies(spec):
    """ given PMI Molecule/Residue or IMP objects, return hierarchies """
    def get_h(s):
        if type(s) is IMP.atom.Selection:
            hs = [IMP.atom.Hierarchy(p) for p in s.get_selected_particles()]
        elif type(s) is IMP.atom.Hierarchy:
            hs = [s]
        elif type(s) is IMP.pmi.topology.Molecule:
            hs = [s.get_hierarchy()]
        elif type(s) is IMP.pmi.topology._Residue:
            hs = [s.get_hierarchy()]
        else:
            raise Exception("Cannot process type "+str(type(s)))
        return hs

    if hasattr(spec,'__iter__'):
        hhs = list(itertools.chain.from_iterable(get_h(item) for item in spec))
    else:
        hhs = get_h(spec)
    return hhs

def get_all_leaves(list_of_hs):
    """ Just get the leaves from a list of hierarchies """
    lvs = list(itertools.chain.from_iterable(IMP.atom.get_leaves(item) for item in list_of_hs))
    return lvs

class DegreesOfFreedom(object):
    """A class to create constraints and movers for an IMP Hierarchy"""
    def __init__(self,mdl):
        self.mdl = mdl
        self.movers = []

    def create_rigid_body(self,
                          hspec,
                          max_trans=1.0,
                          max_rot=0.1):
        """Create rigid body constraint and mover
        @param hspec          Can be one of the following inputs:
                              IMP Selection, Hierarchy,
                              PMI Molecule, Residue, or a list/set
        @param max_trans     Maximum rigid body translation
        @param max_rot     Maximum rigid body rotation
        """
        hs = get_hierarchies(hspec)
        setup_rb = SetupRigidBody(hs,max_trans,max_rot)
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
        """
        hiers = get_hierarchies(hspec)
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
        hiers = get_hierarchies(hspec)
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

class SetupRigidBody(object):
    """Sets up and stores RigidBodyMover and BallMovers for NonRigidMembers"""
    def __init__(self,hiers,max_trans,max_rot):
        self.rb = IMP.atom.create_rigid_body(hiers) #should we check first?
        self.rb_mover = IMP.core.RigidBodyMover(self.rb,max_trans,max_rot)
        self.flexible_movers = []

    def create_non_rigid_members(self,spec,max_trans=1.0):
        hiers = get_hierarchies(spec)
        floatkeys = [IMP.FloatKey(4), IMP.FloatKey(5), IMP.FloatKey(6)]
        idxs = set(self.rb.get_member_indexes())
        for h in hiers:
            p = h.get_particle()
            # check that p is a RigidBodyMember of self.rb
            if not p.get_index() in idxs:
                raise Exception("You tried to create nonrigid members from "
                                 "particles that aren't in the RigidBody!")

            self.rb.add_non_rigid_member(p)
            # rb.set_is_rigid_member(p.get_index(), False) #And/Or do this?
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
    def __init__(self,hiers,transformations):
        if not IMP.atom.RigidBody.get_is_setup(ps):
            self.rb = IMP.atom.create_rigid_body(ps)
        self.rb_mover = DiscreteRigidBodyMover(self.rb,max_trans,max_rot) ### not implemented

class SetupSuperRigidBody(object):
    def __init__(self,hiers,max_trans,max_rot,chain_min_length,chain_max_length):
        self.movers = []
        if chain_min_length is None and chain_max_length is None:
            self._setup_srb(hiers,max_trans,max_rot)
        elif chain_min_length is not None and chain_max_length is not None:
            print('will create chain')
            print(list(IMP.pmi.tools.sublist_iterator(hiers, chain_min_length, chain_max_length)))
            for hs in IMP.pmi.tools.sublist_iterator(hiers, chain_min_length, chain_max_length):
                self._setup_srb(hs,max_trans,max_rot)
        else:
            raise Exception("DegreesOfFreedom: SetupSuperRigidBody if you want chain, specify min AND max")

    def _setup_srb(self,hiers,max_trans,max_rot):
        srbm = IMP.pmi.TransformMover(hiers[0].get_model(), max_trans, max_rot)
        for p in get_all_leaves(hiers):
            if IMP.core.RigidMember.get_is_setup(p) or IMP.core.NonRigidMember.get_is_setup(p):
                srbm.add_rigid_body_particle(p)
            else:
                srbm.add_xyz_particle(p)
        self.movers.append(srbm)

    def get_movers(self):
        return self.movers

class SetupFlexibleBeads(object):
    def __init__(self,hiers,max_trans):
        self.movers = []
        for h in hiers:
            self.movers.append(IMP.core.BallMover([h],maxtrans))
    def get_movers(self):
        return self.movers
