class SimpleTask(object):

    """Simply return the input argument"""

    def __init__(self, inp):
        self.inp = inp

    def __call__(self):
        return self.inp


def error_task():
    """Raise an exception on the worker"""
    raise ValueError("Worker error")


def simple_func(*args):
    return args
