import re, math, unittest


class IMPTestCase(unittest.TestCase):
    """ Super class for IMP test cases """

    def LoadCoordinates(self, pdb_file):
        """ Load coordinates from a PDB file """
        fp = open(pdb_file, 'r')

        coords = []
        for line in fp:
            if line[0:4] == 'ATOM':
                coords.append((float(line[30:38]), float(line[38:46]), float(line[46:54])))

        fp.close()
        return coords

    def LoadAttributes(self, attr_file):
        """ Load attributes from an IMP attributes file """
        fp = open("test_attr.imp", "r")

        out = {}
        for line in fp:
            items = re.split('\s+', line)
            next_item = 0;
            num_items = len(items)
            loading = True
            state = 'GET_NAME'
            data = []

            while loading:
                if next_item >= num_items or items[next_item] == '':
                    if state == 'GET_NAME':
                        loading = False;
                    else:
                        line = fp.next()
                        items = re.split('\s+', line)
                        next_item = 0
                        num_items = len(items)

                else:
                    if state == 'GET_NAME':
                        name = items[next_item]
                        state = 'GET_TYPE'

                    elif state == 'GET_TYPE':
                        type = items[next_item]
                        state = 'GET_NUM_ATTRS'

                    elif state == 'GET_NUM_ATTRS':
                        num_attrs = int(items[next_item])
                        state = 'GET_DATA'
                        data_cnt = 0

                    elif state == 'GET_DATA':
                        if type == 'INT_ARRAY':
                            data.append(int(items[next_item]))
                        elif type == 'FLOAT_ARRAY':
                            data.append(float(items[next_item]))
                        data_cnt = data_cnt + 1
                        if data_cnt >= num_attrs:
                            loading = False

                    next_item = next_item + 1

            if state == 'GET_DATA':
                out[name] = data

        fp.close()
        return out

    def TestAbsPos (self, point, operator, ref_value, x_mask, y_mask, z_mask):
        """ Test absolute position of the given point """
        """ Use masks to indicate which coordinates to use """
        if x_mask + y_mask + z_mask < 2:
            value = point[0]*x_mask + point[1]*y_mask + point[2]*z_mask
        else:
            value = math.sqrt(point[0]*point[0]*x_mask + point[1]*point[1]*y_mask + point[2]*point[2]*z_mask)

        if eval(str(value) + operator + str(ref_value)) == False:
            print "  ** FAILED ** ", str(value) + operator + str(ref_value), "  (", x_mask, y_mask, z_mask, point, ")"
        return eval(str(value) + operator + str(ref_value))

    def TestInTorus (self, point, main_radius, tube_radius):
        """ Test if given point is in the torus """
        rad_dist = math.sqrt(point[0]*point[0] + point[1]*point[1])
        tube_ctr_x = point[0] / rad_dist * main_radius
        tube_ctr_y = point[1] / rad_dist * main_radius
        dx = abs(point[0] - tube_ctr_x)
        dy = abs(point[1] - tube_ctr_y)
        tube_dist = math.sqrt(dx*dx + dy*dy + point[2]*point[2])

        if tube_dist < tube_radius:
            return True
        else:
            print "  ** FAILED ** ", tube_dist, " > ", tube_radius, "  (", point, ")"
            return False

    def Distance (self, pointA, pointB):
        """ Return distance between two given points """
        dx = pointA[0] - pointB[0]
        dy = pointA[1] - pointB[1]
        dz = pointA[2] - pointB[2]
        return math.sqrt(dx*dx + dy*dy + dz*dz)

    def IMP_Distance (self, particles, idx0, idx1):
        """ Return distance between two given particles """
        dx = particles[idx0].x() - particles[idx1].x()
        dy = particles[idx0].y() - particles[idx1].y()
        dz = particles[idx0].z() - particles[idx1].z()
        return math.sqrt(dx*dx + dy*dy + dz*dz)

    def TestMinDistance (self, pointA, pointB, dist):
        """ Test if given points are more than the given distance apart """
        if self.Distance(pointA, pointB) > dist:
            return True
        else:
            print "  ** FAILED ** ", self.Distance(pointA, pointB), " < ", dist, "  (", pointA, ",", pointB, ")"
            return False

    def TestMaxDistance (self, pointA, pointB, dist):
        """ Test if given points are less than the given distance apart """
        if self.Distance(pointA, pointB) < dist:
            return True
        else:
            print "  ** FAILED ** ", self.Distance(pointA, pointB), " > ", dist, "  (", pointA, ",", pointB, ")"
            return False
