from __future__ import print_function
import IMP
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.pmi.tools
import IMP.pmi.restraints

import math
import numpy as np

class ResidueProteinProximityRestraint(IMP.pmi.restraints.RestraintBase):

    """ Restraint residue/residues to bind to unknown location in a target"""

    def __init__(self,
                 hier,
                 selection,
                 cutoff = 6.,
                 sigma = 3.,
                 xi = 0.01,
                 resolution=1.0,
                 weight = 1.0,
                 label = None):
        """
        Constructor
        @ param hier        Hierarchy of the system
        @ param section     Selection of residues and target
                            syntaxis (prot, r1, r2, target_prot)  or  
                            (prot1, r1, r2, target_prot, target_r1, target_r2) 
        @ param cutoff      Distance cutoff between selected segment and target
                            protein 
        @ param sigma       Distance variance between selected fragments
        @ param xi          Slope of a distance-linear scoring function that funnels 
                            the score when the particles are too far away
        @ param resolution  Resolution at which apply restraint
        @ param weight      Weight of the restraint
        @ param label       Extra text to label the restraint so that it is
                            searchable in the output 
        """
        self.hier = hier
        m = self.hier.get_model()

        rname = "ResidueProteinProximityRestraint"
        super(ResidueProteinProximityRestraint, self).__init__(
            m, name="ResidueProteinProximityRestraint", label=label, weight=weight)

        self.cutoff = cutoff
        self.sigma = sigma
        self.xi = xi
        self.resolution = resolution

        # Check selection
        print('selection', selection, isinstance(selection, tuple))
        if (not isinstance(selection, tuple)) and (not isinstance(selection, list)):
            raise ValueError("Selection should be a tuple or list")
        if len(selection) < 4:
            raise ValueError("Selection should be (prot, r1, r2, target_prot) or \
                             (prot1, r1, r2, target_prot, target_r1, target_r2) ")

        # Selection
        self.prot1 = selection[0]
        self.r1 = int(selection[1])
        self.r2 = int(selection[2])

        self.prot2 = selection[3]
        if len(selection) == 6:
            self.tr1 = int(selection[4])
            self.tr2 = int(selection[5])        
 
        if self.r1 == self.r2:
            sel_resi = IMP.atom.Selection(self.hier,
                                          molecule = self.prot1,
                                          residue_index = self.r1,
                                          resolution = self.resolution).get_selected_particles()
        else:
            sel_resi = IMP.atom.Selection(self.hier,
                                          molecule = self.prot1,
                                          residue_indexes = range(self.r1,self.r2+1,1),
                                          resolution = self.resolution).get_selected_particles()

        if len(selection) == 4:
            sel_target = IMP.atom.Selection(self.hier,
                                            molecule=self.prot2,
                                            resolution=self.resolution).get_selected_particles()

        elif len(selection) == 6:
            sel_target = IMP.atom.Selection(self.hier,
                                            molecule=self.prot2,
                                            residue_indexes = range(self.tr1,self.tr2+1,1),
                                            resolution=self.resolution).get_selected_particles()

            
        self.included_ps = sel_resi + sel_target
        
        # Setup restraint
        distance = 0.0
        slack = cutoff*2      
  
        br = IMP.isd.ResidueProteinProximityRestraint(m,
                                                      self.cutoff,
                                                      self.sigma,
                                                      self.xi, 
                                                      True,
                                                      'ResidueProteinProximityRestraint')

        print('Selected fragment and target lengths:', len(sel_resi), len(sel_target))
        
        # Setup close pair container
        # Find close pair within included_resi and included_target
        lsa_target = IMP.container.ListSingletonContainer(m)
        lsa_target.add(IMP.get_indexes(sel_target))
        
        lsa_resi = IMP.container.ListSingletonContainer(m)
        lsa_resi.add(IMP.get_indexes(sel_resi))
        
        self.cpc = IMP.container.CloseBipartitePairContainer(lsa_resi,
                                                             lsa_target,
                                                             distance,
                                                             slack)
                
        br.add_pairs_container(self.cpc)

        # Add particle indexes
        pis1 = [p.get_index() for p in sel_resi]
        pis2 = [p.get_index() for p in sel_target]
        
        br.add_contribution_particles(sel_resi, sel_target)

        # Compute interpolation paramaters
        yi = (cutoff**2/(2*sigma**2)-math.log(1/math.sqrt(2*math.pi*sigma*sigma))+cutoff*xi/2.)/(cutoff/2.)
        interpolation_factor = -(cutoff/2.)*(xi-yi)
        max_p = math.exp(-((distance+slack)**2)/(2*sigma**2))/math.sqrt(2*math.pi*sigma*sigma)
        max_score = -math.log(max_p)
        
        # Add interpolation parameters
        br.set_yi(yi)
        br.set_interpolation_factor(interpolation_factor)
        br.set_max_score(max_score)
        
        self.rs.add_restraint(br)

        self.restraint_sets = [self.rs] + self.restraint_sets[1:]

    def get_container_pairs(self):
        """ Get particles in the close pair container """
        m = self.hier.get_model()
        return self.cpc.get_indexes()

    def get_output(self):
        output = {}
        score = self.weight * self.rs.unprotected_evaluate(None)
        output["ResidueProteinProximityRestraint_score_" + self.label] = str(score)
        
        return output

        
