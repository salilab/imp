import modeller, unittest
import modeller
import modeller.optimizers
import os
import IMP.modeller_intf
import IMP.test, IMP

# Class to test proximity restraints
class test_proximity(IMP.test.IMPTestCase):
    """test proximity restraints"""

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
        self.t.append(IMP.modeller_intf.IMP_Restraints(self.imp_model, self.particles))

        self.modeller_model = IMP.modeller_intf.Create_Particles(6, self.env, self.imp_model, self.particles)
        p1 = self.particles[0]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 1)

        p1 = self.particles[1]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 2)

        p1 = self.particles[2]
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 3)

        p1 = self.particles[3]
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 4)

        p1 = self.particles[4]
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 3)
        p1.add_int("id", 5)

        p1 = self.particles[5]
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 3)
        p1.add_int("id", 6)

        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    # proximity
    def test_proximity(self):
        """ all particles should be within a given max distance of each other """
        self.atmsel.randomize_xyz(deviation=100.0)

        self.restraint_sets.append(IMP.RestraintSet("proximity"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint_set(rs)

        # add proximity restraints

        particle_indexes = IMP.vectori()
        rsrs = []

        # set up exclusion volumes
        IMP.modeller_intf.Set_Up_Exclusion_Volumes(self.imp_model, self.particles, "radius", rsrs)

        max_distance = 10.0

        # set max distance for two most distant particles together
        particle_indexes.clear()
        for i in range(6):
            particle_indexes.push_back(i)

        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound", 0.0, 0.1)
        rsrs.append(IMP.ProximityRestraint(self.imp_model, particle_indexes, "radius", max_distance, score_func_params))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=1800, actions=None)
        self.modeller_model.write (file='out_proximity.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_proximity.pdb')
        os.unlink('out_proximity.pdb')

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
