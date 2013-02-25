from Queue import Queue
from threading import Thread

if hasattr(Queue, 'task_done'):
    def cpu_count():
        """Return the number of processors this machine has"""
        try:
            import multiprocessing
            return multiprocessing.cpu_count()
        except ImportError:
            return 1

    class _Worker(Thread):
        """Thread executing tasks from a given tasks queue"""
        def __init__(self, tasks):
            Thread.__init__(self)
            self.tasks = tasks
            self.daemon = True
            self.start()

        def run(self):
            while True:
                func, args, kargs = self.tasks.get()
                try: func(*args, **kargs)
                except Exception, e: print e
                self.tasks.task_done()

    class ThreadPool:
        """Pool of threads consuming tasks from a queue"""
        def __init__(self, num_threads=-1):
            if num_threads == -1:
                num_threads=2*cpu_count()
            print "Creating thread pool with", num_threads
            self.tasks = Queue(-1)
            for _ in range(num_threads): _Worker(self.tasks)

        def add_task(self, func, *args, **kargs):
            """Add a task to the queue"""
            self.tasks.put((func, args, kargs))

        def wait_completion(self):
            """Wait for completion of all the tasks in the queue"""
            self.tasks.join()

else:
    # If we don't have newer enough Queue (Python <2.5) then just run the
    # tasks on a single core
    class ThreadPool:
        def __init__(self, num_threads=-1):
            pass

        def add_task(self, func, *args, **kargs):
            """Run task immediately"""
            func(*args, **kargs)

        def wait_completion(self):
            pass
