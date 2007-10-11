import modeller, unittest
import modeller.optimizers
import os
import IMP.modeller_intf
import IMP.test, IMP

# Class to test various coordinate restraints
class test_coordinate(IMP.test.IMPTestCase):
    """Test various absolute position restraints"""

    def setUp(self):
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


    # x min
    def test_x_min(self):
        """ all x values should be greater than the set minimum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("x-min")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 8.0, 0.1)
        while p_iter.next():
            self.rsrs.append(IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "X_AXIS", score_func_params))
            r = self.rsrs[len(self.rsrs)-1]
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_x_min.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_x_min.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '>=', 8.0, 1, 0, 0), "x-min condition")
        rs.set_is_active(False)
        os.unlink('out_x_min.pdb')

    # x max
    def test_x_max(self):
        """ all x values should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("x-max")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", -8.0, 0.1)
        while p_iter.next():
            self.rsrs.append(IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "X_AXIS", score_func_params))
            r = self.rsrs[len(self.rsrs)-1]
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_x_max.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_x_max.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', -8.0, 1, 0, 0), "x-max condition")
        rs.set_is_active(False)
        os.unlink('out_x_max.pdb')


    # y min
    def test_y_min(self):
        """ all y values should be greater than the set minimum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("y-min")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 8.0, 0.1)
        while p_iter.next():
            self.rsrs.append(IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "Y_AXIS", score_func_params))
            r = self.rsrs[len(self.rsrs)-1]
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_y_min.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_y_min.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '>=', 8.0, 0, 1, 0), "y-min condition")
        rs.set_is_active(False)
        os.unlink('out_y_min.pdb')

    # y max
    def test_y_max(self):
        """ all y values should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("y-max")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", -8.0, 0.1)
        while p_iter.next():
            self.rsrs.append(IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "Y_AXIS", score_func_params))
            r = self.rsrs[len(self.rsrs)-1]
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_y_max.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_y_max.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', -8.0, 0, 1, 0), "y-max condition")
        rs.set_is_active(False)
        os.unlink('out_y_max.pdb')

    # z min
    def test_z_min(self):
        """ all z values should be greater than the set minimum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("z-min")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_lower_bound", 8.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "Z_AXIS", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_z_min.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_z_min.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '>=', 8.0, 0, 0, 1), "z-min condition")
        rs.set_is_active(False)
        os.unlink('out_z_min.pdb')

    # z max
    def test_z_max(self):
        """ all z values should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("z-max")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", -8.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "Z_AXIS", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_z_max.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_z_max.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', -8.0, 0, 0, 1), "z-max condition")
        rs.set_is_active(False)
        os.unlink('out_z_max.pdb')

    # xy radial
    def test_xy_radial(self):
        """ all xy distances should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("xy_radial")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 8.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "XY_RADIAL", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_xy_radial.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_xy_radial.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', 8.1, 1, 1, 0), "xy_radial condition")
        rs.set_is_active(False)
        os.unlink('out_xy_radial.pdb')

    # xz radial
    def test_xz_radial(self):
        """ all xz distances should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        rs = IMP.Restraint_Set("xz_radial")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 8.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "XZ_RADIAL", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_xz_radial.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_xz_radial.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', 8.1, 1, 0, 1), "xz_radial condition")
        rs.set_is_active(False)
        os.unlink('out_xz_radial.pdb')

    # yz radial
    def test_yz_radial(self):
        """ all yz distances should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        self.restraint_sets.append(IMP.Restraint_Set("yz_radial"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 8.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "YZ_RADIAL", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_yz_radial.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_yz_radial.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', 8.1, 0, 1, 1), "yz_radial condition")
        rs.set_is_active(False)
        os.unlink('out_yz_radial.pdb')

    # xyz sphere
    def test_xyz_sphere(self):
        """ all xyz distances should be less than the set maximum """
        self.atmsel.randomize_xyz(deviation=100.0)

        self.restraint_sets.append(IMP.Restraint_Set("xyz_sphere"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.Particle_Iterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.Basic_Score_Func_Params("harmonic_upper_bound", 8.0, 0.1)
        while p_iter.next():
            r = IMP.RSR_Coordinate(self.imp_model, p_iter.get(), "XYZ_SPHERE", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        new_mdl = self.opt.optimize (self.atmsel, max_iterations=55, actions=None)
        self.modeller_model.write (file='out_xyz_sphere.pdb', model_format='PDB')

        coords = self.LoadCoordinates('out_xyz_sphere.pdb')
        for coord in coords:
            self.assert_(self.TestAbsPos(coord, '<=', 8.1, 1, 1, 1), "xyz_sphere condition")
        rs.set_is_active(False)
        os.unlink('out_xyz_sphere.pdb')

if __name__ == '__main__':
    unittest.main()
