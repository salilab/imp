## \example statistics/write_a_metric.py
## This simple example shows how to write an IMP.statistics.Metric in python.

import IMP.statistics
import math
import random
class MyMetric(IMP.statistics.Metric):
    """Define a metric on a list of floating point numbers based on their difference"""
    def __init__(self, nums):
        """Store the list of numbers to measure distances between"""
        IMP.statistics.Metric.__init__(self, "MyMetric%1%")
        self._nums=nums
    def get_distance(self, i, j):
        """Return the magnitude of the distance between the ith and jth number"""
        return math.fabs(self._nums[i]-self._nums[j])
    def get_number_of_items(self):
        return len(self._nums)


mm= MyMetric([random.uniform(0,1) for i in range(0,15)])

cc= IMP.statistics.create_centrality_clustering(mm, .1, 3)
print cc.get_number_of_clusters()
