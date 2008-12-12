# test that derivatives agree with numerical ones
import modeller
import IMP
import IMP.test
import sys
import IMP.modeller
import IMP.em
import EM
import unittest
from os import unlink


def init_particle(particles,p_ind_,x_,y_,z_,r_=0.0,w_=1.0,protein_=1):
    """particles     IMP particles
    p_ind         particle indices
    x_            x coord
    y_            y coord
    z_            z coord
    r_            radius
    w_            weight
    protein_      protein identifier (int)"""
    p1 = particles[p_ind_]
    p1.add_attribute(IMP.FloatKey("radius"), r_, False)
    p1.add_attribute(IMP.FloatKey("weight"), w_)
    p1.add_attribute(IMP.IntKey("id"), p_ind_)
    p1.add_attribute(IMP.IntKey("protein"), protein_)
    p1.set_value(IMP.FloatKey("x"), x_)
    p1.set_value(IMP.FloatKey("y"), y_)
    p1.set_value(IMP.FloatKey("z"), z_)

class DerivativesTest(IMP.test.TestCase):
    """check the agreement of numerical and analytical
       derivatives"""
    def setUp(self):
        """initialize IMP and modeller environment
           create particles"""
        IMP.test.TestCase.setUp(self)
        modeller.log.level= (0,0,0,0,1)
        self.env = modeller.environ()
        self.env.io.atom_files_directory = self.get_test_file('../data/')
        self.env.edat.dynamic_sphere = False
        self.env.libs.topology.read(file='$(LIB)/top_heav.lib')
        self.env.libs.parameters.read(file='$(LIB)/par.lib')
        #init IMP model ( the environment)
        self.imp_model = IMP.Model()
        self.particles = []
        #add IMP Restraints into the modeller scoring function
        t = self.env.edat.energy_terms
        t.append(IMP.modeller.IMPRestraints(self.imp_model, self.particles))
        ## -  create a set of three particles in imp
        npart = 3
        self.modeller_model = IMP.modeller.create_particles(npart,
                                  self.env, self.imp_model, self.particles)
        # - add the particles attributes ( other than X,Y,Z)
        rad = 1.0
        wei = 1.0
        init_particle(self.particles,0,9.0,9.0,9.0,rad,wei,1)
        init_particle(self.particles,1,12.0,3.0,3.0,rad,wei,1)
        init_particle(self.particles,2,3.0,12.0,12.0,rad,wei,1)
        IMP.modeller.copy_imp_coords_to_modeller(self.particles,self.modeller_model.atoms)
        #modeller_model.write(file='xxx.pdb')
        self.particle_indexes = IMP.Ints()
        self.particle_indexes.clear()
        for i in xrange(npart):
            self.particle_indexes.push_back(i)
        self.atmsel = modeller.selection(self.modeller_model)
        print "initialization done ..."

    def test_deriv(self):
        """1. create a map
           2. read it back in
           3. calculate score
           4. distort model
           5. calculate derivatives """
        resolution=3.
        voxel_size=1.
        access_p = IMP.em.IMPParticlesAccessPoint(self.imp_model,
                                  self.particle_indexes, "radius", "weight")
        model_map = EM.SampledDensityMap(access_p, resolution, voxel_size)
        erw = EM.EMReaderWriter()
        xorigin = model_map.get_header().get_xorigin()
        yorigin = model_map.get_header().get_yorigin()
        zorigin = model_map.get_header().get_zorigin()
        print("x= " + str(xorigin) + " y=" + str(yorigin) + " z=" + str(zorigin) )
        model_map.Write("xxx.em",erw)
        # EM restraint
        em_map = EM.DensityMap()
        em_map.Read("xxx.em",erw)
        em_map.get_header_writable().set_xorigin(xorigin)
        em_map.get_header_writable().set_yorigin(yorigin)
        em_map.get_header_writable().set_zorigin(zorigin)
        em_map.get_header_writable().set_resolution(resolution)
        ind_emrsr = []
        ind_emrsr.append(IMP.em.EMFitRestraint(self.imp_model,
                                           self.particle_indexes,
                                           em_map,
                                           "radius",
                                           "weight",
                                           1.0))
        self.imp_model.add_restraint(ind_emrsr[0])
        print("EM-score score: "+str(self.atmsel.energy()) )
        self.atmsel.randomize_xyz(1.0)
        nviol = self.atmsel.debug_function(debug_function_cutoff=(.010, 0.010, 0.01),
                                      detailed_debugging=True)
        self.assert_(nviol < 1, "at least one partial derivative is wrong!")
        print " derivs done ..."
        unlink("xxx.em")

if __name__ == '__main__':
    unittest.main()
