import IMP.kernel

m = IMP.kernel.Model()
o = IMP.kernel._ConstOptimizer(m)
o.get_score_threshold()
