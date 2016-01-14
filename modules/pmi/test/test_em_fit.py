from __future__ import print_function
import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import IMP.container

import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.proteomics
import IMP.pmi.restraints.crosslinking
import IMP.pmi.restraints.em
import IMP.pmi.representation
import IMP.pmi.tools
import IMP.pmi.macros

import numpy as np


def get_random_gaussian_3d(center):
    std = np.random.random_sample(3,) * 10
    var = [s ** 2 for s in std]
    rot = IMP.algebra.get_random_rotation_3d()
    trans = IMP.algebra.Transformation3D(rot, center)
    return IMP.algebra.Gaussian3D(IMP.algebra.ReferenceFrame3D(trans), var)

class TestEMRestraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()
        self.p0 = IMP.Particle(self.m)
        self.g0 = IMP.core.Gaussian.setup_particle(
            self.p0, get_random_gaussian_3d([0, 0, 0]))
        IMP.atom.Mass.setup_particle(self.p0, np.random.rand() * 10)
        self.p1 = IMP.Particle(self.m)
        self.g1 = IMP.core.Gaussian.setup_particle(
            self.p1, get_random_gaussian_3d([0, 0, 0]))
        IMP.atom.Mass.setup_particle(self.p1, np.random.rand() * 10)

    @IMP.test.skip("EM interface is changing")
    def test_move_to_center(self):
        target_h = [IMP.atom.Fragment.setup_particle(IMP.Particle(self.m))]
        target_h[0].add_child(self.p0)
        gem = IMP.pmi.restraints.em.GaussianEMRestraint(target_h,
                                                        target_ps=[self.p1],
                                                        cutoff_dist_for_container=10000.0,
                                                        target_radii_scale=10.0,
                                                        model_radii_scale=10.0)
        gem.add_to_model()
        self.m.update()
        print('eval 0')
        s0 = self.m.evaluate(False)
        print('>s0', s0, '\n')

        trans = IMP.algebra.Transformation3D(
            IMP.algebra.Vector3D(np.random.random_sample(3,) * 10))
        print('random trans', trans)
        IMP.core.transform(IMP.core.RigidBody(self.p0), trans)
        self.m.update()
        s1 = self.m.evaluate(False)
        print('>s1', s1, '\n')

        gem.center_model_on_target_density()
        self.m.update()
        s2 = self.m.evaluate(False)
        print('>s2', s2)
        self.assertAlmostEqual(s0, s2)

class TestPMI(IMP.test.TestCase):
    def test_em_pmi(self):
        outputobjects = []
        sampleobjects = []

        # setting up topology

        m = IMP.Model()
        simo = IMP.pmi.representation.Representation(m,upperharmonic=True,disorderedlength=False)

        fastadirectory = self.get_input_file_name("mediator/")
        pdbdirectory=self.get_input_file_name("mediator/")
        gmmdirectory=self.get_input_file_name("mediator/")
        midpdb="cr_mid_fullmed10.pdb"

        # compname  hier_name    color         fastafile              fastaid          pdbname      chain    resrange      read    "BEADS"ize rigid_body super_rigid_body emnum_components emtxtfilename  emmrcfilename chain of super rigid bodies

        domains_middle= [("med4",  "med4_1",    0.10,  fastadirectory+"med4.fasta",  "med4",   pdbdirectory+midpdb,   "D",    (1,131,0),    True,       20,      1,         [19,1,2],     2,   gmmdirectory+"med4_1.txt",  gmmdirectory+"med4_1.mrc",   [0]),
                         ("med4",  "med4_2",    0.10,  fastadirectory+"med4.fasta",  "med4",   "BEADS",               None,   (132,284,0),  True,       20,      2,         [19,1,2],     0,   None,  None,   [0])]

        domains=domains_middle

        bm=IMP.pmi.macros.BuildModel1(simo)
        bm.build_model(domains)
        bm.scale_bead_radii(40,0.8)

        resdensities_middle=bm.get_density_hierarchies([t[1] for t in domains_middle])

        # randomize the initial configuration
        simo.shuffle_configuration(100)

        # defines the movers
        simo.set_rigid_bodies_max_rot(1.0)
        simo.set_floppy_bodies_max_trans(0.1)
        simo.set_rigid_bodies_max_trans(0.1)
        outputobjects.append(simo)
        sampleobjects.append(simo)

        # scoring function
        #simo.optimize_floppy_bodies(200)

        # middle module em density
        middle_mass=sum((IMP.atom.Mass(p).get_mass() for h in resdensities_middle for p in IMP.atom.get_leaves(h)))
        gemh = IMP.pmi.restraints.em.GaussianEMRestraint(
            resdensities_middle,
            gmmdirectory+'target_gmm.txt',
            target_mass_scale=middle_mass,
            slope=0.000001,
            target_radii_scale=3.0)

if __name__ == '__main__':
    IMP.test.main()
