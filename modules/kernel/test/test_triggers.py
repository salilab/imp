import IMP
import IMP.test

class Tests(IMP.test.TestCase):

    def test_triggers(self):
        """Test Model triggers"""
        m = IMP.Model()
        # Model age should increase on each call to update()
        # but should never be zero
        self.assertEqual(m.get_age(), 1)
        m.update()
        self.assertEqual(m.get_age(), 2)
        m.update()
        self.assertEqual(m.get_age(), 3)

        tk = IMP.TriggerKey("test_trigger")
        # Trigger has never been updated
        self.assertEqual(m.get_trigger_last_updated(tk), 0)

        m.set_trigger_updated(tk)
        self.assertEqual(m.get_trigger_last_updated(tk), 3)

        m.update()
        m.set_trigger_updated(tk)
        self.assertEqual(m.get_trigger_last_updated(tk), 4)


if __name__ == '__main__':
    IMP.test.main()
