This module contains functionality for dealing with kinematic mechanical structures and their associated internal coordinates system. Please note that it is still at an experimental testing phase and should be used with caution.

## Overview
This module provides a variety of functionality for defining and controlling kinematic structures (chains, trees and forests) over sets of rigid body particles. The IMP::kinematics::KinematicForest data structure provides the high-level interface to define the kinematic structure over a set of particles and to control their associated internal coordinates system. The basic building block of a kinematic structures is a kinematic joint (also known in the literature as a 'kinematic pair'), represented in the abstract class IMP::kinematics::Joint. A kinematic joint connects a pair of rigid bodies. See [Wikipedia](http://en.wikipedia.org/wiki/Kinematic_pair) for some background theory about kinematic joints. The kinematics module supports differnt types of joints (prismatic / revolute / etc.) that differ in the constraints on the degrees of freedom of the joint. For instance, a prismatic joint or a slider (IMP::kinematics::PrismaticJoint) allows the two rigid bodies to slide along a shared axis among them (one degree of freedom), whereas a revolute joint (IMP::kinematics::RevoluteJoints) allows only rotation about the shared axis (one degree of freedom). The library also containes composite joints (IMP::kinematics::CompositeJoint) which allows composing several joints over the same pair of rigid bodies.

## Conventions
The name "Joint" is used to refer to a kinematic pair between two rigid bodies.

_Author(s)_: Dina Schneidman, Barak Raveh

_License_: \external{http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html,LGPL}.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

*/
