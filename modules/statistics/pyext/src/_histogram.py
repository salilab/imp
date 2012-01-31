def _show_histogram_1d(h, yscale, xscale, curves):
    import numpy as np
    import matplotlib.pyplot as plt
    import matplotlib.mlab as mlab
    fig= plt.figure()
    ax= fig.add_subplot(111)
    data=[]
    countsgrid= h.get_counts()
    bins=[countsgrid.get_bounding_box(i).get_corner(0)[0] for i in countsgrid.get_all_indexes()]
    counts= h.get_counts().get_all_voxels()
    gbb=h.get_bounding_box()
    if yscale=='linear':
        ax.bar(bins, counts, align='edge', width=bins[1]-bins[0])
    else:
        ax.plot(bins, counts, "bx-", linewidth=2)
    ax.set_xlim(gbb.get_corner(0)[0],
                gbb.get_corner(1)[0])
    ax.set_xscale(xscale)
    ax.set_yscale(yscale)
        # only scale based on histogram
    ax.set_autoscaley_on(False)
    for c in curves:
        ax.plot(bins, [c(x) for x in bins], "go-", linewidth=1)
    plt.show()

def _show_histogram_2d(h, yscale, xscale):
    import numpy as np
    import matplotlib.cm as cm
    import matplotlib.mlab as mlab
    import matplotlib.pyplot as plt
    cg= h.get_counts()
    steps=cg.get_unit_cell()
    x = np.arange(cg.get_bounding_box().get_corner(0)[0]+.5*steps[0],
                  cg.get_bounding_box().get_corner(1)[0]-.5*steps[0])
    y = np.arange(cg.get_bounding_box().get_corner(0)[1]+.5*steps[1],
                  cg.get_bounding_box().get_corner(1)[1]-.5*steps[1])
    X, Y = np.meshgrid(x, y)
    Z,junk= np.meshgrid(x,y)
    for i,xi in enumerate(x):
        for j, yj in enumerate(y):
            Z[i][j]=cg[cg.get_nearest_index(IMP.algebra.Vector2D(xi,yj))]
    im = plt.pcolor(X,Y, Z, cmap=cm.jet)
    plt.colorbar(im)
    plt.show()

def _show_histogram_3d(h):
    import IMP.display
    cg= h.get_counts()


def show_histogram(h, yscale='linear', xscale='linear',
                   curves=[]):
    if h.get_dimension()==1:
        _show_histogram_1d(h, yscale, xscale, curves)
    elif h.get_dimension()==2:
        _show_histogram_2d(h, yscale, xscale)
    elif h.get_dimension()==3:
        _show_histogram_3d(h)
    else:
        raise ValueError("Dimension "+str(h.get_dimension())+ "not supported")
