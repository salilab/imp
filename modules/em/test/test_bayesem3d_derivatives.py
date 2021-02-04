from __future__ import print_function
import IMP

import IMP.em
import IMP.core
import IMP.atom
import IMP.test

import random
import math
import sys

IMP.set_log_level(IMP.SILENT)
IMP.set_check_level(IMP.NONE)

#Set up the restraints
import numpy as np

try:
    import julia
    from julia import FiniteDifferences as FD
except ImportError:
    julia=None

class EMFitRestraint(IMP.Restraint):
    def __init__(self, model, ps, dmap, weight_keys, resolution, sigma):

        IMP.Restraint.__init__(self, model, "EMFitRestraintBayesEM3D%1%")
        self.ps = ps
        self.dmap = dmap
        self.weight_keys = weight_keys
        self.model = model
        self.sigma = sigma

        dh = self.dmap.get_header()
        dh.set_resolution(resolution)
        fr = IMP.em.FitRestraintBayesEM3D(self.ps, self.dmap, weight_keys, True, self.sigma)

        self.rs = IMP.RestraintSet(self.model, "EMFitRestraintBayesEM3D")
        self.rs.add_restraint(fr)

    def get_restraint_set(self):
        return self.rs

    def unprotected_evaluate(self,da):
        return self.rs.unprotected_evaluate(da)

    def do_get_inputs(self):
        return self.rs.do_get_inputs()

    def get_setup(self):
        sf = IMP.core.RestraintsScoringFunction(self.rs)
        return self.rs, sf

def get_particles(m, input_pdb):
    ### Read input fragment
    sel0 = IMP.atom.BackbonePDBSelector()
    sel1 = IMP.atom.CBetaPDBSelector()
    mh = IMP.atom.read_pdb(input_pdb, m, IMP.atom.OrPDBSelector(sel0, sel1))
    IMP.atom.add_radii(mh)
    return mh, IMP.core.get_leaves(mh)

class Tests(IMP.test.TestCase):
    """Derivative score stability Test"""

    def setUp(self):

        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()

        self.mh, self.ps = get_particles(self.m, "input/input_atom0.pdb")

        ##Read and setup EM
        res = 5.
        voxel_size = 1.5

        dmap = IMP.em.read_map("input/sampled_all.mrc", IMP.em.MRCReaderWriter())
        dmap.get_header_writable().set_resolution(res)
        dmap.update_voxel_size(voxel_size)
        self.m.update()

        ################################
        #Convert fragment to rigid body#
        ################################
        IMP.atom.create_rigid_body(self.mh)
        self.prot_rb = IMP.core.RigidMember(
            IMP.core.get_leaves(self.mh)[0]).get_rigid_body()
        self.m.update()

        #print(self.prot_rb.get_rigid_members())

        #Set up the restraints
        ## Normalize EM map
        mass = IMP.atom.get_mass_from_number_of_residues(157)
        t = IMP.em.get_threshold_for_approximate_mass(dmap, mass)
        self.fmap = IMP.em.get_threshold_map(dmap, t)
        bayesem3d = IMP.em.BayesEM3D()
        bayesem3d.get_normalized_intensities(self.fmap, self.ps, res);

        r = EMFitRestraint(self.m, self.ps, self.fmap,
            IMP.atom.Mass.get_mass_key(), res, 0.5)

        self.rs, self.sf = r.get_setup()
        score = self.rs.evaluate(True)

    ##@IMP.test.skipIf(julia is None, "Requires julia")
    def test_derivatives(self):
        fdm = FD.central_fdm(5, 1)

        total_counter = 0
        identical_counter = 0

        self.m.update()
        for i in range(1):
            IMP.core.transform(self.prot_rb, IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(), (1.0 * i, 0, 0)))
            self.m.update()

            dr = IMP.core.XYZ(self.prot_rb).get_derivatives()
            q = np.array(IMP.core.RigidBody(self.prot_rb).get_rotation().get_quaternion())
            dq = np.array(IMP.core.RigidBody(self.prot_rb).get_rotational_derivatives())
            dq -= np.dot(dq, q) * q

            def myfun(x, y, z, qs, qx, qy, qz):
                q = np.array([qs, qx, qy, qz])
                rot = IMP.algebra.Rotation3D(q)
                trans = IMP.algebra.Transformation3D(rot, (x, y, z))
                rf = IMP.algebra.ReferenceFrame3D(trans)

                IMP.core.RigidBody(self.prot_rb).set_reference_frame(rf)
                self.m.update()
                return self.rs.evaluate(False)

            deriv_fd = FD.grad(fdm, myfun,
                *IMP.core.XYZ(self.prot_rb).get_coordinates(), *q)

            ddr = np.array(dr - deriv_fd[:3]) < 1e-7
            ddq = np.array(dq - deriv_fd[-4:]) < 1e-7

            identical_counter = sum(ddr) + sum(ddq)

            print("Translation Derivatives")
            print(dr)
            print(deriv_fd[:3])

            print("Quaternion Derivatives")
            print(dq)
            print(deriv_fd[-4:])

            IMP.core.transform(self.prot_rb, IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(), (-1.0 * i, 0, 0)))
            self.m.update()

            total_counter += 1
        self.assertEqual(total_counter * 7, identical_counter)


if __name__ == '__main__':
    IMP.test.main()
