import modeller, unittest
import modeller
import modeller.optimizers
import os

# set the appropriate search path
import sys
sys.path.append("../python_libs/")
import IMP_Modeller_Intf
import IMP_Test, imp2

# Class to test pair connectivity restraints
class test_exclusion_volumes(IMP_Test.IMPTestCase):
    """test exclusion volumes restraints"""

    def setUp(self):
        """set up Modeller with exclusion volumes restraints """
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

        self.modeller_model = IMP_Modeller_Intf.Create_Particles(12, self.env, self.imp_model, self.particles)
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
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 4)

        p1 = self.particles[4].imp()
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 5)

        p1 = self.particles[5].imp()
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 6)

        p1 = self.particles[6].imp()
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 7)

        p1 = self.particles[7].imp()
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 8)

        p1 = self.particles[8].imp()
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 9)

        p1 = self.particles[9].imp()
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 10)

        p1 = self.particles[10].imp()
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 11)

        p1 = self.particles[11].imp()
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 12)

        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    # intra-protein exclusion volumes
    def test_exclusion_volumes_one_list(self):
        """ all particles in a single protein should be connected but should
        not be closer than allowed by their vw radii"""
        rs = imp2.Restraint_Set("one_list")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        # add connectivity restraints

        particle_indexes1 = imp2.vectori()
        particle_indexes2 = imp2.vectori()
        rsrs = []
        score_func_ub = imp2.Harmonic_Upper_Bound()
        score_func_lb = imp2.Harmonic_Lower_Bound()

        # connect 2 proteins together by two beads
        particle_indexes1.clear()
        for i in range(0, 5):
            particle_indexes1.push_back(i)
        particle_indexes2.clear()
        for i in range(5, 12):
            particle_indexes2.push_back(i)

        # connect the beads within the two proteins together
        for i in range(0, 4):
            rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[i].imp(), self.particles[i+1].imp(), "radius", 0.1, score_func_ub))

        for i in range(5, 11):
            rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[i].imp(), self.particles[i+1].imp(), "radius", 0.1, score_func_ub))

        # create the exclusion volume for each protein
        rsrs.append(imp2.RSR_Exclusion_Volume(self.imp_model, particle_indexes1, "radius", 0.1))
        rsrs.append(imp2.RSR_Exclusion_Volume(self.imp_model, particle_indexes2, "radius", 0.1))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=100, actions=None)
        self.modeller_model.write (file='out_exclusion_volume_one_list.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_exclusion_volume_one_list.pdb')
        os.unlink('out_exclusion_volume_one_list.pdb')

        # check min distances for intra-protein pairs
        for i in range(0, 4):
            for j in range(i+1, 5):
                d = self.Distance(coords[i], coords[j]) - self.particles[i].get_float("radius") - self.particles[j].get_float("radius")
                self.assert_(d > -0.05, "particles "+str(i)+" and "+str(j)+" are too close together.")

        for i in range(5, 11):
            for j in range(i+1, 12):
                d = self.Distance(coords[i], coords[j]) - self.particles[i].get_float("radius") - self.particles[j].get_float("radius")
                self.assert_(d > -0.05, "particles "+str(i)+" and "+str(j)+" are too close together.")


    # intra- and inter- protein exclusion volumes
    def test_exclusion_volumes_two_lists(self):
        """ all particles in a single protein should be connected and the two
        proteins should be connected in four locations but beads should
        not be closer than allowed by their vw radii"""
        rs = imp2.Restraint_Set("two_lists")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        # add connectivity restraints

        particle_indexes1 = imp2.vectori()
        particle_indexes2 = imp2.vectori()
        rsrs = []
        score_func_ub = imp2.Harmonic_Upper_Bound()
        score_func_lb = imp2.Harmonic_Lower_Bound()

        # connect 2 proteins together by two beads
        particle_indexes1.clear()
        for i in range(0, 5):
            particle_indexes1.push_back(i)
        particle_indexes2.clear()
        for i in range(5, 12):
            particle_indexes2.push_back(i)

        # connect the beads within the two proteins together
        for i in range(0, 4):
            rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[i].imp(), self.particles[i+1].imp(), "radius", 0.1, score_func_ub))

        for i in range(5, 11):
            rsrs.append(imp2.RSR_Distance(self.imp_model, self.particles[i].imp(), self.particles[i+1].imp(), "radius", 0.1, score_func_ub))

        # create the exclusion volume for each protein
        rsrs.append(imp2.RSR_Exclusion_Volume(self.imp_model, particle_indexes1, "radius", 0.1))
        rsrs.append(imp2.RSR_Exclusion_Volume(self.imp_model, particle_indexes2, "radius", 0.1))

        # connect the beads within the two proteins together
        # get 4 distinct pairs
        num_connects = 4
        particle_reuse = False
        rsrs.append(imp2.RSR_Pair_Connectivity(self.imp_model, particle_indexes1, particle_indexes2, "radius", 0.1, score_func_ub, num_connects, particle_reuse))

        # create the exclusion volume for each protein
        rsrs.append(imp2.RSR_Exclusion_Volume(self.imp_model, particle_indexes1, particle_indexes2, "radius", 0.1))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=200, actions=None)
        self.modeller_model.write (file='out_exclusion_volume_two_lists.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_exclusion_volume_two_lists.pdb')
        os.unlink('out_exclusion_volume_two_lists.pdb')

        # check min distances for intra-protein and inter-protein pairs
        for i in range(0, 11):
            for j in range(i+1, 12):
                d = self.Distance(coords[i], coords[j]) - self.particles[i].get_float("radius") - self.particles[j].get_float("radius")
                self.assert_(d > -0.05, "particles "+str(i)+" and "+str(j)+" are too close together.")

if __name__ == '__main__':
    unittest.main()
