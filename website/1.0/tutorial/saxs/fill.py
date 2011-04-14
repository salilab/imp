from modeller import *
from modeller.automodel import *

class MyModel(automodel):
    def special_patches(self, aln):
        self.rename_segments(segment_ids=['A'],
                             renumber_residues=[944])


e = environ()

a = MyModel(e, alnfile='3KFO-fill.ali',
            knowns='3KFO', sequence='3KFO-fill')
a.starting_model = 1
a.ending_model = 5
a.make()
