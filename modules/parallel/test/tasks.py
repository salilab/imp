class SimpleTask(object):
    """Simply return the input argument"""
    def __init__(self, inp):
        self.inp = inp

    def __call__(self):
        return self.inp
