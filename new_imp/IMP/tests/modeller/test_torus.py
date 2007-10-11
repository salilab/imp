import modeller, unittest
import modeller.optimizers
import os
import IMP.modeller_intf
import IMP.test, IMP

# Class to test torus restraints
class test_torus(IMP.test.IMPTestCase):
    """test torus restraints"""

    def setUp(self):
        """set up Modeller with the torus restraints """
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

        self.modeller_model = IMP.modeller_intf.Create_Particles(12, self.env, self.imp_model, self.particles)
        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    # torus
    def test_torus(self):
        """ all particles should be inside the specified torus """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("torus")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 0.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Torus(self.imp_model, p_iter.get(), 50, 10, score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_torus.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_torus.pdb')
        for coord in coords:
            self.assert_(self.TestInTorus(coord, 50, 10.001),
                         "in torus condition")
        os.unlink('out_torus.pdb')


if __name__ == '__main__':
    unittest.main()
