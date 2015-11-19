import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.npc

class Tests(IMP.test.TestCase):

    def evaluate_config(self, m, particles):
        particles_by_type = {}
        for particle_data in particles:
            name, x, y = particle_data
            p = IMP.Particle(m, name)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(x,y,0.))
            particle_type = name[0]
            if particle_type in particles_by_type:
                particles_by_type[particle_type].append(p)
            else:
                particles_by_type[particle_type] = [p]
        print particles_by_type
        ps = IMP.core.DistancePairScore(IMP.core.Linear(0.0, 1.0))
        r = IMP.npc.CompositeRestraint(m, ps)
        r.set_maximum_score(10)
        for val in particles_by_type.values():
            r.add_type(val)
        return r.evaluate(False)

    def test_bond_parameters(self):
        m = IMP.Model()
        s = self.evaluate_config(m,
                             [('A1', 0,0),
                              ('C1', 1,0),
                              ('B1', 0,-1),
                              ('A2', -10,-10),
                              ('B2', 10,10),
                              ('C2', 10,-10)])
        print(s)

if __name__ == '__main__':
    IMP.test.main()
