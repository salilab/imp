import IMP
IMP.set_deprecation_exceptions(True)

m = IMP.Model()
pi = m.add_particle("p")
p = m.get_particle(pi)
td = IMP._TrivialDecorator.setup_particle(m, pi)
tdd = IMP._TrivialDerivedDecorator.setup_particle(m, pi)
tdt = IMP._TrivialTraitsDecorator.setup_particle(m, pi)
assert(IMP._take_particle_adaptor(p) == pi)
assert(IMP._take_particle_adaptor(td) == pi)
assert(IMP._take_particle_adaptor(tdd) == pi)
assert(IMP._take_particle_adaptor(tdt) == pi)
