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
class test_pair_connectivity(IMP_Test.IMPTestCase):
    """test pair connectivity restraints"""

    def setUp(self):
        """set up Modeller with connectivity restraints """
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
        p1 = self.particles[0]
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 1)

        p1 = self.particles[1]
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 2)

        p1 = self.particles[2]
        p1.add_float("radius", 1.0, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 3)

        p1 = self.particles[3]
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 4)

        p1 = self.particles[4]
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 1)
        p1.add_int("id", 5)

        p1 = self.particles[5]
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 6)

        p1 = self.particles[6]
        p1.add_float("radius", 1.5, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 7)

        p1 = self.particles[7]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 8)

        p1 = self.particles[8]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 9)

        p1 = self.particles[9]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 10)

        p1 = self.particles[10]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 11)

        p1 = self.particles[11]
        p1.add_float("radius", 2.0, False)
        p1.add_int("protein", 2)
        p1.add_int("id", 12)

        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    # connectivity
    def test_connectivity(self):
        """ all particles in a single protein should be connected, and all proteins
        should be connected, either directly or indirectly through other proteins """
        self.atmsel.randomize_xyz(deviation=25.0)

        rs = imp2.Restraint_Set("connect")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        # add connectivity restraints

        particle_indexes1 = imp2.vectori()
        particle_indexes2 = imp2.vectori()
        rsrs = []

        # connect 2 proteins together by two beads
        particle_indexes1.clear()
        for i in range(0, 5):
            particle_indexes1.push_back(i)
        particle_indexes2.clear()
        for i in range(5, 12):
            particle_indexes2.push_back(i)
        num_connects = 3

        # set up exclusion volumes
        score_func_params = imp2.Basic_Score_Func_Params("harmonic_lower_bound", 0.0, 0.1)
        rsrs.append(imp2.RSR_Exclusion_Volume(self.imp_model, particle_indexes1, particle_indexes2, "radius", score_func_params))

        # it should work whether this is True or False
        # However, if it is False, the close pairs should all be between distinct particles
        particle_reuse = False
        score_func_params = imp2.Basic_Score_Func_Params("harmonic_upper_bound", 0.0, 0.1)
        rsrs.append(imp2.RSR_Pair_Connectivity(self.imp_model, particle_indexes1, particle_indexes2, "radius", score_func_params, num_connects, particle_reuse))

        # add restraints
        for i in range(len(rsrs)):
            rs.add_restraint(rsrs[i])

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_pair_connectivity.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_pair_connectivity.pdb')
        os.unlink('out_pair_connectivity.pdb')

        # min distances
        min_dist = []
        for i in range(num_connects):
            min_dist.append(10000000)

        for i in range(0, 5):
            for j in range(5, 12):
                d = self.Distance(coords[i], coords[j]) - self.particles[i].get_float("radius") - self.particles[j].get_float("radius")
                found = False
                for k in range(num_connects):
                    if not found:
                        if d < min_dist[k]:
                            found = True

                    if found:
                        d1 = min_dist[k]
                        min_dist[k] = d
                        d = d1

        for i in range(num_connects):
            self.assert_(min_dist[i] < 0.05, "min distance for any pair condition")

if __name__ == '__main__':
    unittest.main()
