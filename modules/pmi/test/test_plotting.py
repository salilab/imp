import IMP.pmi
import IMP.pmi.analysis
import IMP.test
import IMP.rmf
import sys

class Tests(IMP.test.TestCase):
    @IMP.test.expectedFailure
    def test_graphxl(self):
        import IMP.pmi.plotting.topology
        dd={"med6":["med6"],
            "med8":["med8"],
            "med11":["med11"],
            "med17-NTD":[(1,102,"med17")],
            "med17-CTD":[(103,687,"med17")],
            "med18":["med18"],
            "med20":["med20"],
            "med22":["med22"],
            "med4":["med4"],
            "med7":["med7"],
            "med9":["med9"],
            "med31":["med31"],
            "med21":["med21"],
            "med10":["med10"],
            "med1":["med1"],
            "med14-NTD":[(1,711,"med14")],
            "med14-CTD":[(712,1000,"med14")],
            "med19":["med19"],
            "med2":["med2"],
            "med3":["med3"],
            "med5":["med5"],
            "med15":["med15"],
            "med16":["med16"]}

        g = IMP.pmi.plotting.topology.TopologyPlot(IMP.Model(),dd,15.0)
        g.add_rmf('0.rmf3',0)
        g.make_plot('out.png')

if __name__ == '__main__':
    IMP.test.main()
