# Ricardo Marmolejo Garcia
from threading import Thread
from threading import Condition
from threading import BoundedSemaphore
import collections
import traceback
import multiprocessing

class worker(Thread):
    def __init__(self, tasks, lock, func, sync, results, lock_result, *args, **kwargs):
        Thread.__init__(self)
        self._tasks = tasks
        self.lock = lock
        self.func = func
        self.sync = sync
        self._results = results
        self.lock_result = lock_result
        self.args = args
        self.kwargs = kwargs
        self.interrupted = False

    def cancel(self):
        self.interrupted = True

    def run(self):
        self.sync.acquire()
        try:
            pendingJob = True
            while pendingJob and (not self.interrupted):
                self.lock.acquire()
                try:
                    pendingJob = len(self._tasks) > 0
                    if not pendingJob:
                        break
                    data = self._tasks.popleft()
                finally:
                    self.lock.release()
                
                # call function
                try:
                    result = self.func(data, *self.args, **self.kwargs)
                    self.lock_result.acquire()
                    try:
                        self._results.append((data, result, None))
                    finally:
                        self.lock_result.release()
                except Exception as e:
                    self.lock_result.acquire()
                    try:
                        self._results.append((data, e, traceback.format_exc()))
                    finally:
                        self.lock_result.release()
        finally:
            self.sync.release()

class map_parallel:
    def __init__(self, func, tasks, *args, **kwargs):
        self._input = tasks
        self._tasks = collections.deque(tasks)
        self.num_threads = min(multiprocessing.cpu_count(), len(self._tasks))
        self.sync = BoundedSemaphore(self.num_threads)
        self.lock = Condition()
        self.workers = []
        self._results = collections.deque()
        self.lock_result = Condition()
        for i in range(self.num_threads):
            w = worker(self._tasks, self.lock, func, self.sync, self._results, self.lock_result, *args, **kwargs)
            w.daemon = False
            w.start()
            self.workers.append(w)
    
    def cancel(self):
        # stop work !
        for w in self.workers:
            w.cancel()
        self._tasks = []
    
    def pending(self):
        self.lock.acquire()
        try:
            return len(self._tasks)
        finally:
            self.lock.release()
    
    def __iter__(self):
        self.sync.acquire(self.num_threads)
        for w in self.workers:
            w.join()
        results = {}
        for data, result, extra in self._results:
            if extra is None:
                results[data] = result
            else:
                raise result
        for data in self._input:
            yield results[data]

    def get(self):
        return list(self)

if __name__ == '__main__':
    input = [1, 2, 3, 4, 5, 6, 7, 8, 9]
    pool = map_parallel(lambda x: x*x*x, input)
    for i, e in zip(input, pool):
        print 'f({}) = {}'.format(i, e)
