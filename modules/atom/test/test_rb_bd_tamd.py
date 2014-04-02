import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.rmf
import RMF


class Tests(IMP.test.TestCase):

    def _create_fine_particle(self, m, name):
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
        return p


    def _create_tamd_centroid(self, m):
        """Create a TAMD centroid for a bunch of particles,
        with a real centroid and restrained centroid realization"""

#        IMP.base.set_log_level(IMP.MEMORY)
        n = 3
        p = IMP.kernel.Particle(m, "body") # "tamd_centroid")
        pH = IMP.core.Hierarchy.setup_particle(p)
        pD = IMP.core.XYZR.setup_particle(p)
        pD.set_radius(3)
        IMP.atom.Mass.setup_particle(p, n)
        pD.set_coordinates_are_optimized(True) # TODO: very unclear if this is needed or dangerous - to get BD to evaluate on it
        pDiffusion = IMP.atom.Diffusion.setup_particle(p)
#        pDiffusion.set_diffusion_coefficient(1000000)
        print pDiffusion
        fine_particles = []
        for i in range(0, n):
            fine_p = self._create_fine_particle(m, "fine particle %d" % i)
            fine_particles.append(fine_p)
            fine_pH = IMP.core.Hierarchy.setup_particle(fine_p)
            pH.add_child( fine_pH )
        print pH.get_children()
        refiner = IMP.core.ChildrenRefiner(
            IMP.core.Hierarchy.get_default_traits())
        IMP.core.Centroid.setup_particle(p, refiner)
        m.update() # so c is up to date

        # TAMD realization of centroid (S for star)
        pstar = IMP.kernel.Particle(m, "centroid*")
        pstarD = IMP.core.XYZR.setup_particle(pstar)
        pstarD.set_coordinates( pD.get_coordinates() )
        pstarD.set_radius( pD.get_radius() )
        pstarD.set_coordinates_are_optimized( True )
        pstarH = IMP.core.Hierarchy.setup_particle(pstar)
        IMP.atom.Diffusion.setup_particle( pstar )
        IMP.atom.TAMDParticle.setup_particle( pstar, 100.0, 4.0)
        IMP.atom.Mass.setup_particle( pstar, 2)

        return (p, fine_particles, pstar)



    def test_bonded(self):
        """Check brownian dynamics with rigid bodies"""
        m = IMP.kernel.Model()
        m.set_log_level(IMP.base.SILENT)
        cores=[]
        for i in range(10):
            cores.append( self._create_tamd_centroid(m) )
#        p2, fparticles2, p2S = self._create_tamd_centroid(m)
#        p3, fparticles3, p3S = self._create_tamd_centroid(m)

        for p in ([cores[0][0]] + cores[0][1]+ [cores[0][2]] ):
            IMP.display.Colored.setup_particle( p, IMP.display.Color(1, 0, 0) )
        for p in ([cores[1][0]] + cores[1][1]+ [cores[1][2]] ):
            IMP.display.Colored.setup_particle( p, IMP.display.Color(0, 1, 0) )
        for p in ([cores[2][0]] + cores[2][1]+ [cores[2][2]] ):
            IMP.display.Colored.setup_particle( p, IMP.display.Color(0, 0, 1) )
        for p in ([cores[3][0]] + cores[3][1]+ [cores[3][2]] ):
            IMP.display.Colored.setup_particle( p, IMP.display.Color(1, 1, 0) )

        # Add Restraints:
        spring = IMP.core.HarmonicDistancePairScore(0, 0.00000001)
        excluded_vol = IMP.core.SoftSpherePairScore(100)
        attraction = IMP.core.HarmonicSphereDistancePairScore(5, 5)
        R=[] # restraints
        # TAMD:
        for core in cores:
            r=IMP.core.PairRestraint(spring, (core[0], core[2]))
            R.append(r)
        # Repulsion between fine particles
        for core1 in cores:
            for p1 in core1[1]:
                for core2 in cores:
                    for p2 in core2[1]:
                        if(p1 == p2): continue
                        r = IMP.core.PairRestraint(excluded_vol, (p1, p2), "Excluded_vol")
                        R.append(r)
        # Attraction between consecutive particles (within or between cores)
        for core in cores:
            for p1,p2 in zip(core[1][0:-1],core[1][1:]):
                r = IMP.core.PairRestraint(attraction, (p1, p2), "Chain")
                R.append(r)
        for core1,core2 in zip(cores[0:-1],cores[1:]):
            r = IMP.core.PairRestraint(attraction, (core1[1][-1], core2[1][0]), "Interchain")
            R.append(r)
        print R

        # Define BD
        bd = IMP.atom.BrownianDynamicsTAMD(m)
        sf = IMP.core.RestraintsScoringFunction(R)
        bd.set_maximum_time_step(5)


        # Attach RMF
#        RMF.set_log_level("Off")
        rmf_fname = self.get_tmp_file_name("bd_rb_NOtamd.rmf")
        rmf = RMF.create_rmf_file(rmf_fname)
        print "RMF: ", rmf_fname
        all_particles=[]
        for core in cores:
            for p in [core[0]] + core[1] + [core[2]]:
                IMP.rmf.add_hierarchy(rmf, p)
        IMP.rmf.add_restraints(rmf, R)
        sf.set_log_level(IMP.base.SILENT)
        os = IMP.rmf.SaveOptimizerState(m, rmf)
        os.set_log_level(IMP.base.SILENT)
        os.set_period(10)
        bd.set_scoring_function(sf)
        bd.add_optimizer_state(os)

        IMP.base.set_log_level(IMP.base.VERBOSE)
        os.update_always("Init position")
        IMP.base.set_log_level(IMP.base.SILENT)

        for i in range(100):
            bd.optimize(5)
            m.update()
            os.update_always("Init opt %d" % i)
            for core in cores:
                centroid =  IMP.core.XYZ(core[0]).get_coordinates()
                IMP.core.XYZ(core[2]).set_coordinates( centroid )

        max_cycles = 100000
        round_cycles = 250
        total_cycles = 0
        e_threshold = 2
        for i in range(max_cycles / round_cycles):
            bd.optimize(round_cycles)
            energy = sf.evaluate(False)
            total_cycles += round_cycles
            print "energy after %d cycles = %.2f" \
                % (total_cycles, energy)
#            if(energy < e_threshold):
#                break
#        self.assertLess(energy, e_threshold)

if __name__ == '__main__':
    IMP.test.main()
