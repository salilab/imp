class WorkerState:

    def __init__(self, desc):
        self.desc = desc

init = WorkerState("Initial state")
started = WorkerState("Started, but not yet connected back to manager")
connected = WorkerState("Connected, with no active task")
running_task = WorkerState("Connected, and running a task")
dead = WorkerState("Disconnected due to failure")
