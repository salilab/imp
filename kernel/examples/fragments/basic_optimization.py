m= IMP::Model()
particles = setup_model(m)
fname= "/tmp/best_scoring_config.yaml"
best_score=1000000
o= IMP.ConjugateGradients()
o.set_model(m)
for i in range(0,1000):
    randomize(particles)
    score=o.optimize(1000)
    if score < best_score:
        IMP::core::write_yaml(m, fname)
        best_score= score
IMP::core::read(fname, m)
