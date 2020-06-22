## \example AA/analyze_dofs.py
# Examples ported from kinetics branch
# (details should be added in future)

import numpy
#import sklearn
#import pandas
import matplotlib.pyplot

# Simple script to plot degrees of freedom
# distributions from a DOF dat file

infile = "dofs_0.1.dat"
infile2 = "./AA_0.78_4/dofs.dat"
data = numpy.genfromtxt(infile, delimiter=" ")
#data2 = numpy.genfromtxt(infile2, delimiter=",")

#hm, xe, ye = numpy.histogram2d(data[:,0],data[:,1], bins=50)
#extent = [xe[0], xe[-1], ye[0], ye[-1]]
print(data.shape, len(data[:,0]))
#matplotlib.pyplot.clf()
#matplotlib.pyplot.imshow(hm.T, extent=extent, origin='lower')
matplotlib.pyplot.tripcolor(data[:,0], data[:,1], data[:,2],15)
#matplotlib.pyplot.pcolor(data)
matplotlib.pyplot.show()

'''
U, s, V = numpy.linalg.svd(data)


fig, ax = matplotlib.pyplot.subplots(1,3, figsize=(12,5))


ax[0].plot(U)
ax[1].plot(s)
ax[2].plot(V.T)
matplotlib.pyplot.plot(numpy.transpose(U[0:3]))
matplotlib.pyplot.show()

'''

#fig,ax = matplotlib.pyplot.subplots(1,2, figsize = (12,6))
#print data.shape
#ax[0].scatter(data[:,0], data[:,1], s=0.2)

#ax[1].scatter(data2[:,0], data2[:,1], s=0.2)
#matplotlib.pyplot.show()
