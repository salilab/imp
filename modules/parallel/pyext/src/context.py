class Context(object):
    def __init__(self, manager, startup=None):
        self._manager = manager
        self._startup = startup
        self._tasks = []

    def add_task(self, task):
        self._tasks.append(task)

    def get_results_unordered(self):
        return self._manager._get_results_unordered(self)
