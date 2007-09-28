import modeller, unittest
import modeller
import modeller.optimizers

# set the appropriate search path
import sys
sys.path.append("../")
sys.path.append("../../")
import IMP_Modeller_Intf
import IMP_Test, imp2

# Class to test proximity restraints
class test_proximity(IMP_Test.IMPTestCase):
    """test proximity restraints"""

    def setUp(self):
        """set up Modeller with proximity restraints """
        modeller.log.level(0,0,0,0,0)

        self.env = modeller.environ()
        self.env.io.atom_files_directory = '../data/'
        self.env.edat.dynamic_sphere = False
        self.env.libs.topology.read(file='$(LIB)/top_heav.lib')
        self.env.libs.parameters.read(file='$(LIB)/par.lib')

        self.imp_model = imp2.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        self.t = self.env.edat.energy_terms
        self.t.append(IMP_Modeller_Intf.IMP_Restraints(self.imp_model, self.particles))

        self.modeller_model = IMP_Modeller_Intf.Create_Particles(4, self.env, self.imp_model, self.particles)
        p1 = self.particles[0].imp()
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 1)

        p1 = self.particles[1].imp()
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 2)

        p1 = self.particles[2].imp()
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 3)

        p1 = self.particles[3].imp()
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 4)

        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    # proximity
    def test_proximity(self):
        """ all particles should be within a given max distance of each other """
        self.atmsel.randomize_xyz(deviation=100.0)

        self.restraint_sets.append(imp2.Restraint_Set("proximity"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint_set(rs)

        # add proximity restraints

        particle_indexes = imp2.vectori()
        rsrs = []
        score_func_ub = imp2.Harmonic_Upper_Bound()
        score_func_lb = imp2.Harmonic_Lower_Bound()

        # set up exclusion volumes
        IMP_Modeller_Intf.Set_Up_Exclusion_Volumes(self.imp_model, self.particles, "radius", rsrs, score_func_lb, 0.1)

        max_distance = 4.0

        # set max distance for two most distant particles together
        particle_indexes.clear()
        for i in range(4):
            particle_indexes.push_back(i)
        rsrs.append(imp2.RSR_Proximity(self.imp_model, particle_indexes, "radius", max_distance, 0.1, score_func_ub))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.imp_model.set_up_trajectory()
        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=155, actions=None)
        self.modeller_model.write (file='out_proximity.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_proximity.pdb')

        # min distances
        for i in range(len(coords)):
            for j in range(i+1,len(coords)):
                self.assert_(self.TestMinDistance(coords[i], coords[j], self.particles[i].get_float("radius") + self.particles[j].get_float("radius") - 0.1), "min distance for any pair condition (" + str(i) + ", " + str(j) + ")")

        # max distances
        for i in range(len(coords)):
            for j in range(i+1,len(coords)):
                self.assert_(self.TestMaxDistance(coords[i], coords[j], max_distance - self.particles[i].get_float("radius") - self.particles[j].get_float("radius") + 0.1), "max distance for any pair condition (" + str(i) + ", " + str(j) + ")")

if __name__ == '__main__':
    unittest.main()
