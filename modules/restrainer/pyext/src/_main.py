from _xml_parser import XMLRepresentation
from _xml_parser import XMLRestraint
from _xml_parser import XMLOptimization
from _xml_parser import XMLDisplay


class Main(object):

    def __init__(self):
        self.representation = None
        self.restraint = None
        self.optimization = None
        self.display = None
        self.log = None
        self._logname = None

    def add_representation(self, fname):
        if self.representation:
            raise Exception("Representation cannot be added twice!")
        self.representation = XMLRepresentation(fname).run()
        return self.representation

    def add_restraint(self, fname):
        if self.restraint:
            raise Exception("Restraint cannot be added twice!")
        if not self.representation:
            raise Exception("Please add Representation before Restraint!")
        self.restraint = XMLRestraint(fname).run()
        self.restraint.add_to_representation(self.representation)
        return self.restraint

    def add_optimization(self, fname):
        if self.optimization:
            raise Exception("Optimization cannot be added twice!")
        self.optimization = XMLOptimization(fname).run()
        return self.optimization

    def add_display(self, fname, logname=None):
        if self.display:
            raise Exception("Display cannot be added twice!")
        self.display = XMLDisplay(fname).run()
        self._logname = logname
        self._create_log()
        return self.display

    def _create_log(self):
        if not self.log:
            if self.display and self.representation:
                if not self.restraint:
                    self.representation.get_model()
                if self._logname:
                    logname = self._logname
                else:
                    logname = 'log%03d.py'
                self.log = self.display.create_log(
                    self.representation, logname)
                self.log.set_was_used(True)

    def optimize(self):
        if self.restraint:
            if not self.representation:
                raise Exception("Representation missing!")
            self._create_log()
            if self.optimization:
                if self._logname:
                    self.optimization.run(self.restraint, self.log)
                else:
                    self.optimization.run(self.restraint, None)
        elif self.optimization:
            raise Exception("Restraint missing!")

    def get_model(self):
        if self.representation:
            return self.representation._model
        else:
            raise Exception("Representation missing!")
