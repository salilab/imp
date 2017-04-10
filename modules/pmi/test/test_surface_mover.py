from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.pmi.restraints
import IMP.pmi.tools
import IMP.pmi.samplers


class SurfaceRestraint(IMP.pmi.restraints.RestraintBase):

    def __init__(self, m, surface, sphere, weight=1, label=None):
        super(SurfaceRestraint, self).__init__(m, weight=weight, label=label)
        self.surf = surface
        ss = IMP.core.HarmonicSurfaceHeightPairScore(0, 1)
        r = IMP.core.PairRestraint(self.m, ss, (surface.get_particle_index(),
                                                sphere.get_particle_index()))
        self.rs.add_restraint(r)

    def get_particles_to_sample(self):
        ps = super(SurfaceRestraint, self).get_particles_to_sample()
        ps["Surfaces" + self._label_suffix] = ([self.surf], 1., .1, .1)
        return ps


class Tests(IMP.test.TestCase):

    """Test correct setup and usage of ``IMP.core.SurfaceMover``"""

    def test_surface_mover(self):
        """Test surface is moved closer to sphere."""
        m = IMP.Model()
        surf = IMP.pmi.tools.SetupSurface(m, (0, 0, 0), (0, 0, 1),
                                          True).get_particle()

        d = IMP.core.create_xyzr_particles(m, 1, 1.)[0]
        d.set_coordinates((0, 0, 10))
        rs = SurfaceRestraint(m, surf, d)
        rs.add_to_model()

        init_height = IMP.core.get_height(surf, d)
        sample_objects = [rs]
        mc = IMP.pmi.samplers.MonteCarlo(m, sample_objects, 1.0)
        for i in range(100):
            mc.optimize(1)
            print(surf.get_coordinates(), surf.get_normal())
        fin_height = IMP.core.get_height(surf, d)
        self.assertLess(fin_height, init_height)


if __name__ == '__main__':
    IMP.test.main()
