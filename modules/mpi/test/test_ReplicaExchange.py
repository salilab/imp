#!/usr/bin/env python

# general imports
from numpy import *
from random import uniform

# imp general
import IMP

import math

import IMP.mpi

# unit testing framework
import IMP.test

rem = IMP.mpi.ReplicaExchange()


class Test(IMP.test.TestCase):

    "simple test cases to check if Replica Exchange works"

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        # IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)

    def testSetGetMyParameters(self):
        "Test Replica Exchange set and get my parameters"
        myindex = rem.get_my_index()
        test_value_in = float(myindex) * 2.0
        rem.set_my_parameter("test", [test_value_in])
        test_value_out = rem.get_my_parameter("test")[0]
        self.assertAlmostEqual(test_value_in, test_value_out, delta=0.00001)

    def testGetFriendParameter(self):
        "Test Replica Exchange get friend parameter"
        myindex = rem.get_my_index()
        my_test_value = float(myindex) * 2.0
        rem.set_my_parameter("test", [my_test_value])
        findex = rem.get_friend_index(0)
        friend_test_value_ref = float(findex) * 2.0
        friend_test_value = rem.get_friend_parameter("test", findex)[0]
        self.assertAlmostEqual(friend_test_value_ref, friend_test_value,
                               delta=0.00001)

    def testExchange(self):
        "Test Replica Exchange do exchange"
        myindex = rem.get_my_index()
        my_test_value_0 = float(myindex) * 2.0
        rem.set_my_parameter("test", [my_test_value_0])
        findex = rem.get_friend_index(0)
        friend_test_value_0 = rem.get_friend_parameter("test", findex)[0]
        rem.do_exchange(0.0, 0.0, findex)
        my_test_value_1 = rem.get_my_parameter("test")[0]
        findex = rem.get_friend_index(0)
        friend_test_value_1 = rem.get_friend_parameter("test", findex)[0]

        self.assertAlmostEqual(my_test_value_0, friend_test_value_1,
                               delta=0.0001)

        self.assertAlmostEqual(my_test_value_1, friend_test_value_0,
                               delta=0.0001)

if __name__ == '__main__':
    IMP.test.main()
