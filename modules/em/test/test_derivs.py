# test that derivatives agree with numerical ones
try:
    import modeller
    import IMP.modeller
except ImportError:
    modeller = None
import IMP
import IMP.test
import IMP.base
import IMP.core
import IMP.algebra
import sys
import IMP.em
import os

def copy_to_modeller(env, particles):
    fh = open("temp_particles.pdb", "w")
    for i in range(len(particles)):
        fh.write("ATOM  %5d  N   ALA     0       0.000   0.000   0.000  "
                 "1.00  0.00           C  \n" % (i))
    fh.close()
    mdl = modeller.model(env, file='temp_particles.pdb')
    os.unlink('temp_particles.pdb')
    return mdl

class Tests(IMP.test.TestCase):
    """check the agreement of numerical and analytical
       derivatives"""

    def test_deriv(self):
        """Test calculated derivatives for a distorted model's map"""
        if modeller is None:
            self.skipTest("modeller module unavailable")
        modeller.log.level= (0,0,0,0,1)
        self.env = modeller.environ()
        self.env.edat.dynamic_sphere = False
        self.env.libs.topology.read(file='$(LIB)/top_heav.lib')
        self.env.libs.parameters.read(file='$(LIB)/par.lib')
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = []
        ## -  create a set of three particles in imp
        for i in range(3):
            self.particles.append(IMP.Particle(self.imp_model))
        #add IMP Restraints into the modeller scoring function
        t = self.env.edat.energy_terms
        t.append(IMP.modeller.IMPRestraints(self.particles))

        # Load the same model into Modeller
        self.modeller_model = copy_to_modeller(self.env, self.particles)

        # - add the particles attributes
        rad = 1.0
        wei = 1.0
        wei_key=IMP.FloatKey("weight")
        prot_key=IMP.IntKey("protein")
        id_key=IMP.IntKey("id")

        for i,p_data in enumerate([[9.0,9.0,9.0,rad,wei,1],[12.0,3.0,3.0,rad,wei,1],[3.0,12.0,12.0,rad,wei,1]]):
            p=self.particles[i]
            center = IMP.algebra.Vector3D(*p_data[0:3])
            sphere = IMP.algebra.Sphere3D(center, p_data[3])
            IMP.core.XYZR.setup_particle(p, sphere)
            p.add_attribute(wei_key,p_data[4])
            p.add_attribute(prot_key,p_data[5])
            p.add_attribute(id_key,i)


        self.atmsel = modeller.selection(self.modeller_model)
        print "initialization done ..."

        resolution=3.
        voxel_size=1.
        model_map = IMP.em.SampledDensityMap(self.particles, resolution, voxel_size,wei_key)
        erw = IMP.em.EMReaderWriter()
        xorigin = model_map.get_header().get_xorigin()
        yorigin = model_map.get_header().get_yorigin()
        zorigin = model_map.get_header().get_zorigin()
        print("x= " + str(xorigin) + " y=" + str(yorigin) + " z=" + str(zorigin) )
        mapfile = IMP.base.create_temporary_file_name('xxx.em')
        IMP.em.write_map(model_map, mapfile, erw)
        # EM restraint
        em_map = IMP.em.read_map(mapfile, erw)
        em_map.get_header_writable().set_xorigin(xorigin)
        em_map.get_header_writable().set_yorigin(yorigin)
        em_map.get_header_writable().set_zorigin(zorigin)
        em_map.get_header_writable().compute_xyz_top()
        em_map.get_header_writable().set_resolution(resolution)
        print "rms_calc",em_map.get_rms_calculated()
        em_map.calcRMS()
        print "rms_calc",em_map.get_rms_calculated()
        ind_emrsr = []
        ind_emrsr.append(IMP.em.FitRestraint(self.particles,
                                             em_map,
                                             [0.,0.],
                                             wei_key,
                                             1.0))
        self.imp_model.add_restraint(ind_emrsr[0])
        print("EM-score score: "+str(self.atmsel.energy()) )
        self.atmsel.randomize_xyz(1.0)
        nviol = self.atmsel.debug_function(debug_function_cutoff=(.010, 0.010, 0.01),
                                      detailed_debugging=True)
        self.assertLess(nviol, 1, "at least one partial derivative is wrong!")
        print " derivs done ..."
        os.unlink(mapfile)


    def test_deriv_to_pull_particles_into_density(self):
        """Test if the derivatives can pull the particles back into the density"""
        """Test calculated derivatives for a distorted model's map"""
        if modeller is None:
            self.skipTest("modeller module unavailable")
        modeller.log.level= (0,0,0,0,1)
        self.env = modeller.environ()
        self.env.edat.dynamic_sphere = False
        self.env.libs.topology.read(file='$(LIB)/top_heav.lib')
        self.env.libs.parameters.read(file='$(LIB)/par.lib')
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = []

        ## -  create a set of three particles in imp
        for i in range(3):
            self.particles.append(IMP.Particle(self.imp_model))

        #add IMP Restraints into the modeller scoring function
        t = self.env.edat.energy_terms
        t.append(IMP.modeller.IMPRestraints(self.particles))

        # Load the same model into Modeller
        self.modeller_model = copy_to_modeller(self.env, self.particles)

        # - add the particles attributes
        rad = 1.0
        wei = 1.0
        wei_key=IMP.FloatKey("weight")
        prot_key=IMP.IntKey("protein")
        id_key=IMP.IntKey("id")

        for i,p_data in enumerate([[9.0,9.0,9.0,rad,wei,1],[12.0,3.0,3.0,rad,wei,1],[3.0,12.0,12.0,rad,wei,1]]):
            p=self.particles[i]
            center = IMP.algebra.Vector3D(*p_data[0:3])
            sphere = IMP.algebra.Sphere3D(center, p_data[3])
            IMP.core.XYZR.setup_particle(p, sphere)
            p.add_attribute(wei_key,p_data[4])
            p.add_attribute(prot_key,p_data[5])
            p.add_attribute(id_key,i)


        self.atmsel = modeller.selection(self.modeller_model)
        print "initialization done ..."

        resolution=3.
        voxel_size=1.
        model_map = IMP.em.SampledDensityMap(self.particles, resolution, voxel_size,wei_key)
        erw = IMP.em.EMReaderWriter()
        xorigin = model_map.get_header().get_xorigin()
        yorigin = model_map.get_header().get_yorigin()
        zorigin = model_map.get_header().get_zorigin()
        print("x= " + str(xorigin) + " y=" + str(yorigin) + " z=" + str(zorigin) )
        mapfile = IMP.base.create_temporary_file_name('xxx.em')
        IMP.em.write_map(model_map, mapfile, erw)
        # EM restraint
        em_map = IMP.em.read_map(mapfile, erw)
        em_map.get_header_writable().set_xorigin(xorigin)
        em_map.get_header_writable().set_yorigin(yorigin)
        em_map.get_header_writable().set_zorigin(zorigin)
        em_map.get_header_writable().compute_xyz_top()
        em_map.get_header_writable().set_resolution(resolution)
        ind_emrsr = []
        ind_emrsr.append(IMP.em.FitRestraint(self.particles,
                                             em_map,
                                             [0,0],
                                             wei_key,
                                             1.0))
        self.imp_model.add_restraint(ind_emrsr[0])
        #move the particles outside of the density
        for p in self.particles:
            xyz=IMP.core.XYZ(p)
            xyz.set_coordinates(xyz.get_coordinates()+IMP.algebra.Vector3D(20,20,20))
        print("EM-score score: "+str(self.atmsel.energy()) )
        self.atmsel.randomize_xyz(1.0)
        nviol = self.atmsel.debug_function(debug_function_cutoff=(.010, 0.010, 0.01),
                                      detailed_debugging=True)
        self.assertLess(nviol, 1, "at least one partial derivative is wrong!")
        print " derivs done ..."
        os.unlink(mapfile)

    def test_fr_deriv(self):
        """Testing FitRestraint derivative magnitudes"""
        use_rigid_bodies=True
        bd= 10
        radius=10
        m= IMP.Model()
        p= IMP.Particle(m)
        IMP.atom.Mass.setup_particle(p, 10000)
        d= IMP.core.XYZR.setup_particle(p)
        d.set_radius(radius)
        fp= d
        to_move=d
        d.set_coordinates_are_optimized(True)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-bd-radius, -bd-radius, -bd-radius),
                                      IMP.algebra.Vector3D( bd+radius,  bd+radius,  bd+radius))

        dheader = IMP.em.create_density_header(bb,1)
        dheader.set_resolution(1)
        dmap = IMP.em.SampledDensityMap(dheader)
        dmap.set_particles([p])

        dmap.resample()
        # computes statistic stuff about the map and insert it in the header
        print "BEFORE calcRMS",dmap.get_rms_calculated()
        dmap.calcRMS()
        print "AFTER calcRMS",dmap.get_rms_calculated()
        IMP.em.write_map(dmap,"map.mrc",IMP.em.MRCReaderWriter())
        rs= IMP.RestraintSet()
        m.add_restraint(rs)

        r= IMP.em.FitRestraint([fp], dmap)
        rs.add_restraint(r)
        for i in range(0,10):
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            self.assertXYZDerivativesInTolerance(m, d, tolerance=.05, percentage=5)


if __name__ == '__main__':
    IMP.test.main()
