import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.rmf
import IMP.container
import RMF
import math



class Tests(IMP.test.TestCase):

    def _create_singleton_particle(self, m, name):
        ''' create a particle for simulation fine level'''
        p = IMP.kernel.Particle(m, name)
        d = IMP.core.XYZR.setup_particle(p)
        d.set_coordinates_are_optimized(True)
        bb = IMP.algebra.BoundingBox3D([0,0,0],[50,50,50])
        d.set_coordinates( IMP.algebra.get_random_vector_in(bb) )
        d.set_radius(2)
        d.set_coordinates_are_optimized(True)
        IMP.atom.Mass.setup_particle(p, 1)
        IMP.atom.Diffusion.setup_particle(p)
        IMP.core.Hierarchy.setup_particle(p)
        return p

    def _create_tamd_image( self, p, name, T_factor, F_factor):
        '''
        Create a TAMD image of centroid particle p

        p - reference particle to be tied by spring
        name - particle name
        T_factor - temeprature factor
        F_factor - friction factor
        k - spring contant

        returns the TAMD image particle
        '''
        # TAMD image of centroid
        pstar = IMP.kernel.Particle(p.get_model(), name)
        pstarD = IMP.core.XYZR.setup_particle(pstar)
        pD = IMP.core.XYZR(p)
        pstarD.set_coordinates( pD.get_coordinates() )
        pstarD.set_radius( pD.get_radius() )
        pstarD.set_coordinates_are_optimized( True )
        pstarH = IMP.core.Hierarchy.setup_particle(pstar)
        IMP.atom.Diffusion.setup_particle( pstar )
        IMP.atom.TAMDParticle.setup_particle( pstar, T_factor, F_factor)
        pMass = IMP.atom.Mass(p)
        IMP.atom.Mass.setup_particle( pstar, pMass.get_mass() )
        return pstar


    def _create_tamd_hierarchy( self, m, nlevels, d, T_factors, F_factors, Ks ):
        """
        Create a TAMD hierarchy of nlevels depth with a core for
        each d centroids in a lower level, with a real centroid and
        restrained centroid realization

        Params:
        -------
        m - model
        nlevels - number of levels in the hierarchy, if 1 -
                  return signleton particles
        d - the out degree of each non-leaf node (# of children)
        T_factors - a list of length nlevels-1 with temeprature at each level
                    from top to bottom
        G_factors - a list of length nlevels-1 with friction factor (G for gamma)
                    at each level from top to bottom
        Ks - spring constants at each level between a particle and its TAMD
             image (a list of length nlevels-1)
        """

        #        IMP.base.set_log_level(IMP.MEMORY)

        # build children hierarchies recursively first
        children=[]
        centroids=[]
        images=[] # all tamd images
        R=[] # all restraints with image particles
        for i in range(d):
            if nlevels==1:
                p =  self._create_singleton_particle(m, "leaf")
                child_centroids = []
                child_images = []
                child_R = []
                return p, child_centroids, child_images, child_R
            else:
                [p, child_centroids, child_images, child_R] \
                    = self._create_tamd_hierarchy(m,
                                                  nlevels - 1,
                                                  d,
                                                  T_factors[1:],
                                                  F_factors[1:],
                                                  Ks[1:])
            children.append(p)
            centroids = centroids + child_centroids
            images = images + child_images
            R = R + child_R

        # Build centroid of lower hierarchies
        p = IMP.kernel.Particle(m, "centroid %d" % nlevels)
        centroids.append(p)
        pH = IMP.core.Hierarchy.setup_particle(p)
        pD = IMP.core.XYZR.setup_particle(p)
        pD.set_radius(2) # math.sqrt(math.pow(d, nlevels))) # TODO: something smarter?
#        pMass = IMP.atom.Mass.setup_particle(p, 1) # math.pow(d, nlevels)) # NOT RELEVANT FOR CENTER OF MASS
        pD.set_coordinates_are_optimized(True) # TODO: very unclear if this is needed or dangerous - to get BD to evaluate on it
        pDiffusion = IMP.atom.Diffusion.setup_particle(p)
        for child in children:
            pH.add_child( IMP.core.Hierarchy( child ) )
        print pH.get_children()
        refiner = IMP.core.ChildrenRefiner(
            IMP.core.Hierarchy.get_default_traits())
        IMP.atom.CenterOfMass.setup_particle(p, refiner)
        m.update() # so center of mass is up to date

        # Add a TAMD image of centroid with a spring
        pstar = self._create_tamd_image(  p,
                                          "TAMD image %d",
                                          T_factors[0],
                                          F_factors[0] )
        images.append(pstar)
        spring = IMP.core.HarmonicDistancePairScore(0, Ks[0])
        r=IMP.core.PairRestraint(spring, (p, pstar))
        R.append(r)

        return p, centroids, images, R

    def _get_ordered_leaves(self, h):
        '''
        get leave particles by certain order that is guaranteed to cluster
        leave particles with a common ancestor together, for hierarchy h
        '''
        if(h.get_number_of_children() == 0):
            return [h.get_particle()]
        leaves = []
        for child in h.get_children():
            leaves = leaves + self._get_ordered_leaves(child)
        return leaves


    def test_bonded(self):
        """Check brownian dynamics with rigid bodies"""
        m = IMP.kernel.Model()
        LOCAL_WELLS = True
#        rmf_fname = self.get_tmp_file_name("bd_rb_no_tamd_multi_lw_50.rmf")
        RMF_FNAME = self.get_tmp_file_name("bd_rb_no_tamd_multi_lw.rmf")
        root, all_centroids, all_images, R \
            = self._create_tamd_hierarchy(m,
                                          5, 2,
#                                          T_factors = [3,2.5,2,1.5],
                                          T_factors = [12,6,3,1.5],
                                          F_factors = [225*225,225*15,225,15],
#                                          Ks = [60,40,20,10] ) # TAMD multi on
                                          Ks = [1e-12,1e-12,1e-12, 10] # TAMD singular on
#                                         Ks = [5e-12,5e-12,5e-12,5e-12] ) # TAMD off
            # = self._create_tamd_hierarchy(m,
            #                               5, 2,
            #                               [1.1,1.1,1.1,1.1],
            #                               [16,8,4,2],
            #                               [5,5,5,5] )
        # Add more restraints to R:
        excluded_vol = IMP.core.SoftSpherePairScore(100)
        attraction = IMP.core.HarmonicSphereDistancePairScore(5, 5)
        # Repulsion between fine particles
        rootH = IMP.core.Hierarchy(root)
        leaves= self._get_ordered_leaves(rootH)
        cpc = IMP.container.ClosePairContainer( leaves, 2, 1 )
        r = IMP.container.PairsRestraint(excluded_vol, cpc, "Excluded_vol")
        R.append( r )
        # Attraction between consecutive leaves
        for i in range(len(leaves)-1):
            r = IMP.core.PairRestraint(attraction,
                                       (leaves[i], leaves[i+1]),
                                       "Chain")
            R.append(r)
        print R

        # LOCAL WELLS:
        if(LOCAL_WELLS):
            thb1 = IMP.core.TruncatedHarmonicBound(27.5, 0.175, 8, 20)
            thb2 = IMP.core.TruncatedHarmonicBound(52.5, 0.175, 8, 20)
            dr1 = IMP.core.DistanceRestraint(thb1, leaves[0], leaves[-1])
            #                                        "local_well_1")
            dr2 = IMP.core.DistanceRestraint(thb2, leaves[0], leaves[-1])
            #                                        "local_well_2")
            R = R + [dr2] #, dr2]

        # Define BD
        bd = IMP.atom.BrownianDynamicsTAMD(m)
        sf = IMP.core.RestraintsScoringFunction(R)
        bd.set_maximum_time_step(5)


        # Attach RMF
#        RMF.set_log_level("Off")
        rmf = RMF.create_rmf_file(RMF_FNAME)
        print "RMF: ", RMF_FNAME
        # add all hierarchies
        queue = [rootH]
        while len(queue)>0:
            h=queue[0]
            queue=queue[1:] # pop first
            queue = queue + h.get_children()
            IMP.rmf.add_hierarchies(rmf, [h] )
        for p in all_images:
            IMP.rmf.add_hierarchies(rmf, [IMP.core.Hierarchy(p)])
        IMP.rmf.add_restraints(rmf, R)
        sf.set_log_level(IMP.base.SILENT)
        os = IMP.rmf.SaveOptimizerState(m, rmf)
        os.set_log_level(IMP.base.SILENT)
        os.set_period(10)
        bd.set_scoring_function(sf)
#        bd.add_optimizer_state(os)

        IMP.base.set_log_level(IMP.base.VERBOSE)
        os.update_always("Init position")
        IMP.base.set_log_level(IMP.base.SILENT)

        for i in range(100):
            bd.optimize(10)
            m.update()
            os.update_always("Init opt %d" % i)
            for centroid, image in zip(all_centroids, all_images):
                print centroid, image
                centroid_coords =  IMP.core.XYZ(centroid).get_coordinates()
                IMP.core.XYZ(image).set_coordinates( centroid_coords )

        # Add seconds restraint
        bd.optimize(100000)
        sf = IMP.core.RestraintsScoringFunction(R + [dr1])
        bd.set_scoring_function(sf)

        os.set_period(2500)
        max_cycles = 1000000000
        round_cycles = 1000
        total_cycles = 0
        e_threshold = 2
        for i in range(max_cycles / round_cycles):
            bd.optimize(round_cycles)
            energy = sf.evaluate(False)
            total_cycles += round_cycles
            ter1 = IMP.core.XYZ( leaves[0] )
            ter2 = IMP.core.XYZ( leaves[-1] )
            dTer = IMP.core.get_distance(ter1, ter2)
            print "energy after %d cycles = %.2f  dTer %.2f" \
                % (total_cycles, energy, dTer)
#            if(energy < e_threshold):
#                break
#        self.assertLess(energy, e_threshold)

if __name__ == '__main__':
    IMP.test.main()
