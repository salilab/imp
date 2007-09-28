import modeller, unittest
import modeller
import modeller.optimizers
import os

# set the appropriate search path
import sys
sys.path.append("../python_libs/")
import IMP_Modeller_Intf
import IMP_Test, imp2

# Class to test torus restraints
class test_torus(IMP_Test.IMPTestCase):
    """test torus restraints"""

    def setUp(self):
        """set up Modeller with the torus restraints """
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
        self.atmsel = modeller.selection(self.modeller_model)

        self.opt = modeller.optimizers.conjugate_gradients()


    # torus
    def test_torus(self):
        """ all particles should be inside the specified torus """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = imp2.Restraint_Set("torus")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = imp2.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func = imp2.Harmonic_Upper_Bound()
        while p_iter.next():
            r = imp2.RSR_Torus(self.imp_model, p_iter.get(), 50, 10, 0.1, score_func)
            self.rsrs.append(r)
            rs.add_restraint(r)

        self.atmsel.randomize_xyz(deviation=100.0)
        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_torus.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_torus.pdb')
        for coord in coords:
            self.assert_(self.TestInTorus(coord, 50, 10), "in torus condition")
        os.unlink('out_torus.pdb')


if __name__ == '__main__':
    unittest.main()
