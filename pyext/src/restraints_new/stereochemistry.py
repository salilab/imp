#!/usr/bin/env python
from __future__ import print_function
import IMP
import IMP.core
import IMP.atom
import IMP.isd
import itertools
import IMP.pmi

class MyGetRestraint(object):
    def __init__(self,rs):
        self.rs=rs
    def get_restraint_for_rmf(self):
        return self.rs

class CharmmForceFieldRestraint(object):
    """ Enable CHARMM force field """
    def __init__(self,root,
                 ff_temp=300.0,
                 zone_ps=None,
                 zone_size=10.0,
                 enable_nonbonded=True,
                 enable_bonded=True,
                 zone_nonbonded=False):
        """Setup the charmm restraint on a selection. Expecting atoms.
        @param root             The node at which to apply the restraint
        @param ff_temp          The temperature of the force field
        @param zone_ps          Create a zone around this set of particles
                                Automatically includes the entire residue (incl. backbone)
        @param zone_size        The size for looking for neighbor residues
        @param enable_nonbonded Allow the repulsive restraint
        @param enable_bonded    Allow the bonded restraint
        @param zone_nonbonded   EXPERIMENTAL: exclude from nonbonded all sidechains that aren't in zone!
        """

        kB = (1.381 * 6.02214) / 4184.0

        self.mdl = root.get_model()
        self.bonds_rs = IMP.RestraintSet(self.mdl, 1.0 / (kB * ff_temp), 'BONDED')
        self.nonbonded_rs = IMP.RestraintSet(self.mdl, 1.0 / (kB * ff_temp), 'NONBONDED')
        self.weight=1.0
        self.label=""

        ### setup topology and bonds etc
        if enable_bonded:
            ff = IMP.atom.get_heavy_atom_CHARMM_parameters()
            topology = ff.create_topology(root)
            topology.apply_default_patches()
            topology.setup_hierarchy(root)
            if zone_ps is not None:
                limit_to_ps=IMP.pmi.topology.get_particles_within_zone(
                    root,
                    zone_ps,
                    zone_size,
                    entire_residues=True,
                    exclude_backbone=False)
                r = IMP.atom.CHARMMStereochemistryRestraint(root,
                                                            topology,
                                                            limit_to_ps)
                self.ps = limit_to_ps
            else:
                r = IMP.atom.CHARMMStereochemistryRestraint(root, topology)
                self.ps = IMP.core.get_leaves(root)
            self.bonds_rs.add_restraint(r)
            ff.add_radii(root)
            ff.add_well_depths(root)

        atoms = IMP.atom.get_leaves(root)
        ### non-bonded forces
        if enable_nonbonded:
            if (zone_ps is not None) and zone_nonbonded:
                print('stereochemistry: zone_nonbonded is True')
                # atoms list should only include backbone plus zone_ps!
                backbone_types=['C','N','CB','O']
                sel = IMP.atom.Selection(root,atom_types=[IMP.atom.AtomType(n)
                                                          for n in backbone_types])
                backbone_atoms = sel.get_selected_particles()
                #x = list(set(backbone_atoms)|set(limit_to_ps))
                sel_ps=IMP.pmi.topology.get_particles_within_zone(
                    root,
                    zone_ps,
                    zone_size,
                    entire_residues=True,
                    exclude_backbone=True)

                self.nbl = IMP.container.CloseBipartitePairContainer(
                    IMP.container.ListSingletonContainer(backbone_atoms),
                    IMP.container.ListSingletonContainer(sel_ps),
                    4.0)
            else:
                cont = IMP.container.ListSingletonContainer(atoms)
                self.nbl = IMP.container.ClosePairContainer(cont, 4.0)
            if enable_bonded:
                self.nbl.add_pair_filter(r.get_full_pair_filter())
            pairscore = IMP.isd.RepulsiveDistancePairScore(0,1)
            pr=IMP.container.PairsRestraint(pairscore, self.nbl)
            self.nonbonded_rs.add_restraint(pr)

        print('CHARMM is set up')

    def set_label(self, label):
        self.label = label
        self.bonds_rs.set_name(self.bonds_rs.get_name()+label)
        self.nonbonded_rs.set_name(self.nonbonded_rs.get_name()+label)

    def add_to_model(self):
        self.mdl.add_restraint(self.bonds_rs)
        self.mdl.add_restraint(self.nonbonded_rs)

    def get_restraints(self):
        return [self.bonds_rs,self.nonbonded_rs]

    def get_restraint(self):
        return self.bonds_rs

    def get_close_pair_container(self):
        return self.nbl

    def get_ps(self):
        return self.ps

    def set_weight(self, weight):
        self.weight = weight
        self.bonds_rs.set_weight(weight)
        self.nonbonded_rs.set_weight(weight)
        print('CHARMM: weight is',self.weight)

    def get_output(self):
        self.mdl.update()
        output = {}
        bonds_score = self.weight * self.bonds_rs.unprotected_evaluate(None)
        nonbonded_score = self.weight * self.nonbonded_rs.unprotected_evaluate(None)
        score=bonds_score+nonbonded_score
        output["_TotalScore"] = str(score)
        output["CHARMM_BONDS"+str(self.label)] = str(bonds_score)
        output["CHARMM_NONBONDED"+str(self.label)] = str(nonbonded_score)
        return output

class ElasticNetworkRestraint(object):
    def __init__(self,root,
                 subsequence=None,
                 label='',
                 strength=10.0,
                 dist_cutoff=10.0,
                 **kwargs):
        """ Add harmonic restraints between all pairs below a specified distance
        @param root             Root hierarchy for applying selections
        @param subsequence      A PMI Subsequence class
        @param strength         The elastic bond strength
        @param dist_cutoff      Create bonds when below this distance
        \note any additional keyword arguments are passed to the Selection
        E.g. you could pass atom_type=IMP.atom.AtomType("CA")c
        """
        self.mdl = root.get_model()
        self.rs = IMP.RestraintSet(self.mdl, "ElasticNetwork")
        self.weight = 1
        self.pairslist = []
        self.label=label
        if subsequence is not None:
            self.ps = subsequence.get_selection(root,**kwargs).get_selected_particles()
        else:
            self.ps = IMP.atom.Selection(root,**kwargs).get_selected_particles()
        if len(self.ps)==0:
            print('ERROR: Did not select any particles!')
            exit()
        self.rs = IMP.pmi.create_elastic_network(self.ps,dist_cutoff,strength)
        #self.rs = IMP.isd_emxl.create_lognormal_elastic_network(ps,dist_cutoff,strength)
        print('created elastic network',self.label,'with',self.rs.get_number_of_restraints(),'restraints')

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def get_ps(self):
        return self.ps

    def add_to_model(self):
        self.mdl.add_restraint(self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_restraints_for_rmf(self):
        return [MyGetRestraint(self.rs)]

    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["ElasticNetworkRestraint_" + self.label] = str(score)
        output["_TotalScore"] = str(score)
        return output

class SymmetryRestraint(object):
    def __init__(self,root,
                 symmetry_transforms,
                 selection_dicts,
                 extra_sel={"atom_type":IMP.atom.AtomType("CA")},
                 label='',
                 strength=10.0):
        """ Add harmonic restraints to the mean positions between multiple selections.
        @param root                Root hierarchy for applying selections
        @param symmetry_transforms Transforms moving each selection to the first selection
        @param selection_dicts     Selection kwargs for each symmetry unit
        @param extra_sel           Additional selection arguments (e.g., CA only!)
        @param strength            The elastic bond strength

        \note the length of symmetry_transforms should be one more than the length of selection_dicts
        Because all transforms are WRT the first selection
        """
        self.mdl = root.get_model()
        self.rs = IMP.RestraintSet(self.mdl, "Symmetry")
        self.weight = 1
        self.label=label

        if len(selection_dicts)!=len(symmetry_transforms)+1:
            print('Error: There should be one more symmetry transform than selection dict')
            exit()
        harmonic = IMP.core.Harmonic(0.,strength)
        sel0 = hierarchy_tools.combine_dicts(selection_dicts[0],extra_sel)
        ps0 = IMP.atom.Selection(root,**sel0).get_selected_particles()
        if len(ps0)==0:
            print("Error: did not select any particles!")
            exit()
        for sdict,trans in zip(selection_dicts[1:],symmetry_transforms):
            sel = hierarchy_tools.combine_dicts(sdict,extra_sel)
            ps=IMP.atom.Selection(root,**sel).get_selected_particles()
            if len(ps0)!=len(ps):
                print("Error: did not select the same number of particles!")
                exit()
            pair_score = IMP.core.TransformedDistancePairScore(harmonic,trans)
            for p0,p1 in zip(ps0,ps):
                r = IMP.core.PairRestraint(pair_score,(p0,p1))
                self.rs.add_restraint(r)

        print('created symmetry network with',self.rs.get_number_of_restraints(),'restraints')

    def set_label(self, label):
        self.label = label
        self.rs.set_name(label)
        for r in self.rs.get_restraints():
            r.set_name(label)

    def add_to_model(self):
        self.mdl.add_restraint(self.rs)

    def get_restraint(self):
        return self.rs

    def set_weight(self, weight):
        self.weight = weight
        self.rs.set_weight(weight)

    def get_excluded_pairs(self):
        return self.pairslist

    def get_output(self):
        self.mdl.update()
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["SymmetryRestraint_" + self.label] = str(score)
        output["_TotalScore"] = str(score)
        return output

class CoarseCARestraint(object):
    """ Add bonds, angles, and dihedrals for a CA-only model """
    def __init__(self,root,
                 strength=10.0,
                 bond_distance=3.78,
                 bond_jitter=None,
                 angle_min=100.0,
                 angle_max=140.0,
                 dihedral_seq=None):
        pass
