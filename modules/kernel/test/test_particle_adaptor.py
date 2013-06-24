import IMP.kernel

m = IMP.kernel.Model()
pi = m.add_particle("p")
p = m.get_particle(pi)
td = IMP.kernel._TrivialDecorator.setup_particle(m, pi)
tdd = IMP.kernel._TrivialDerivedDecorator.setup_particle(m, pi)
tdt = IMP.kernel._TrivialTraitsDecorator(m, pi)
assert(IMP.kernel._take_particle_adaptor(p) == pi)
assert(IMP.kernel._take_particle_adaptor(td) == pi)
assert(IMP.kernel._take_particle_adaptor(tdd) == pi)
assert(IMP.kernel._take_particle_adaptor(tdt) == pi)
