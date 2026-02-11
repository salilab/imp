import IMP.atom
import jax

m = IMP.Model()
mh = IMP.atom.read_pdb('mini.pdb', m)
IMP.atom.add_dope_score_data(mh)

ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
dpc = IMP.container.AllPairContainer(ps)
dps = IMP.atom.DopePairScore(7.0)
d = IMP.container.PairsRestraint(dps, dpc)
score = d.evaluate(False)

ji = d._get_jax()
jm = ji.get_jax_model()
j = jax.jit(ji.score_func)
print(j(jm))
