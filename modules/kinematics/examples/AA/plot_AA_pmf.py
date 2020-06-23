## \example AA/plot_AA_pmf.py
''' Matplotlib script to plot the score vs. phi/psi for the alanine dipeptide example
'''
import numpy
import matplotlib
import matplotlib.pyplot

fh = open("AA_charmm_out/enumerate_phi_psi.dat", "r")

# Burn the first line
fh.readline()

# Fields 1 is the score, 2 and 3 are phi/psi
scores = []
phi = []
psi = []
for i in fh.readlines():
    fields = i.split(" ")
    scores.append(float(fields[1]))
    phi.append(float(fields[2]))
    psi.append(float(fields[3]))

phi = numpy.unique(phi)
psi = numpy.unique(psi)
Phi, Psi = numpy.meshgrid(phi,psi)

Scores = numpy.array(scores).reshape(len(phi), len(psi))

fig, ax = matplotlib.pyplot.subplots()
ax.set_xlabel("Phi")
ax.set_ylabel("Psi")
im = ax.pcolormesh(Phi, Psi, Scores, vmin = 0, vmax = 10000)
fig.colorbar(im, ax=ax)
print("Plot of enumerate_phi_psi.dat created at AA_pmf.png")

matplotlib.pyplot.savefig("AA_pmf.png")
