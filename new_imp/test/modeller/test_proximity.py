import modeller, unittest
import modeller
import modeller.optimizers
import os
import IMP
import IMP.modeller_intf
import IMP.utils
import IMP.test

radius = IMP.FloatKey("radius")

class ProximityRestraintTests(IMP.test.IMPTestCase):
    """Test proximity restraints"""

    def setUp(self):
        """set up Modeller with proximity restraints """
        modeller.log.level(0,0,0,0,0)

        self.env = modeller.environ()
        self.env.io.atom_files_directory = '../data/'
        self.env.edat.dynamic_sphere = False
        self.env.libs.topology.read(file='$(LIB)/top_heav.lib')
        self.env.libs.parameters.read(file='$(LIB)/par.lib')

        self.imp_model = IMP.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        self.t = self.env.edat.energy_terms
        self.t.append(IMP.modeller_intf.IMPRestraints(self.imp_model,
                                                      self.particles))

        self.modeller_model = IMP.modeller_intf.create_particles(6, self.env,
                                                                 self.imp_model,
                                                                 self.particles)
        p1 = self.particles[0]
        p1.add_attribute(radius, 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 1)
        p1.add_attribute(IMP.IntKey("id"), 1)

        p1 = self.particles[1]
        p1.add_attribute(radius, 2.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 1)
        p1.add_attribute(IMP.IntKey("id"), 2)

        p1 = self.particles[2]
        p1.add_attribute(radius, 1.5, False)
        p1.add_attribute(IMP.IntKey("protein"), 2)
        p1.add_attribute(IMP.IntKey("id"), 3)

        p1 = self.particles[3]
        p1.add_attribute(radius, 1.5, False)
        p1.add_attribute(IMP.IntKey("protein"), 2)
        p1.add_attribute(IMP.IntKey("id"), 4)

        p1 = self.particles[4]
        p1.add_attribute(radius, 1.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 5)

        p1 = self.particles[5]
        p1.add_attribute(radius, 1.0, False)
        p1.add_attribute(IMP.IntKey("protein"), 3)
        p1.add_attribute(IMP.IntKey("id"), 6)

        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    def test_proximity(self):
        """Test proximity restraints using Modeller optimizers"""
        self.atmsel.randomize_xyz(deviation=100.0)

        self.restraint_sets.append(IMP.RestraintSet("proximity"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint(rs)

        # add proximity restraints

        particle_indexes = IMP.Ints()
        rsrs = []

        # set up exclusion volumes
        IMP.utils.set_up_exclusion_volumes(self.imp_model, self.particles,
                                           radius, rsrs)

        max_distance = 10.0

        # set max distance for two most distant particles together
        particle_indexes.clear()
        for i in range(6):
            particle_indexes.push_back(i)

        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound", 0.0, 0.1)
        rsrs.append(IMP.ProximityRestraint(self.imp_model, particle_indexes,
                                           radius, max_distance,
                                           score_func_params))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=1800, actions=None)
        self.modeller_model.write (file='out_proximity.pdb', model_format='PDB')

        coords = self.load_coordinates('out_proximity.pdb')
        os.unlink('out_proximity.pdb')

        # min distances
        model_data = self.imp_model.get_model_data()
        for i in range(len(coords)):
            irad = self.particles[i].get_value(radius)
            for j in range(i+1,len(coords)):
                jrad = self.particles[j].get_value(radius)
                self.assert_(self.check_min_distance(coords[i], coords[j], irad + jrad - 0.1), "min distance for any pair condition (" + str(i) + ", " + str(j) + ")")

        # max distances
        for i in range(len(coords)):
            irad = self.particles[i].get_value(radius)
            for j in range(i+1,len(coords)):
                jrad = self.particles[j].get_value(radius)
                self.assert_(self.check_max_distance(coords[i], coords[j], max_distance - irad - jrad + 0.1), "max distance for any pair condition (" + str(i) + ", " + str(j) + ")")

if __name__ == '__main__':
    unittest.main()
