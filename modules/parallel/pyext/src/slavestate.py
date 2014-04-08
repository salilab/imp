class SlaveState:

    def __init__(self, desc):
        self.desc = desc

init = SlaveState("Initial state")
started = SlaveState("Started, but not yet connected back to master")
connected = SlaveState("Connected, with no active task")
running_task = SlaveState("Connected, and running a task")
dead = SlaveState("Disconnected due to failure")
