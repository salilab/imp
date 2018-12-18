import IMP
import IMP.test
import IMP.pmi.samplers

class Tests(IMP.test.TestCase):

    def test_serial(self):
        """Test the _SerialReplicaExchange class"""
        s = IMP.pmi.samplers._SerialReplicaExchange()
        self.assertEqual(s.get_number_of_replicas(), 1)
        ts = s.create_temperatures(100., 200., 45)
        self.assertEqual(len(ts), 1)
        self.assertAlmostEqual(ts[0], 100.0, delta=1e-6)
        self.assertEqual(s.get_my_index(), 0)
        self.assertEqual(s.get_friend_index(0), 0)
        s.set_my_parameter("temp", ['foo', 'bar'])
        self.assertEqual(s.get_my_parameter("temp"), ['foo', 'bar'])
        self.assertEqual(s.get_friend_parameter("temp", 0), ['foo', 'bar'])
        self.assertEqual(s.do_exchange(0, 0, 0), False)


if __name__ == '__main__':
    IMP.test.main()
