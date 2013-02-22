import sys

def not_exit():
    raise RuntimeError("runtime error in PiVy")

def get_writer(parent):
    class PivyWriter(parent):
        def __init__(self):
            parent.__init__(self, "pivy")
            self._setup_pivy()
        def _setup_pivy(self):
            import pivy.sogui
            import pivy.coin
            import os
            self.pivy = pivy
            # actually, pivy seems to call exit on failure. Bah.
            if "DISPLAY" not in os.environ.keys():
                raise RuntimeError("No display variable found")
            myWindow = pivy.sogui.SoGui.init(sys.argv[0])
            if myWindow == None:
                raise RuntimeError("Can't open PiVy window.")
            self.window=myWindow
            print "scene"
            scene = pivy.coin.SoSeparator()

            # Create a viewer in which to see our scene graph.
            viewer = pivy.sogui.SoGuiExaminerViewer(myWindow)

            # Put our scene into viewer, change the title
            viewer.setSceneGraph(scene)
            print "title"
            viewer.setTitle("IMP")
            print "show"
            viewer.show()
            self.root=scene
            self.viewer=viewer
        def handle_color(self, parent, c):
            color= self.pivy.coin.SoMFColor()
            color.setValue(c.get_red(), c.get_green(), c.get_blue())
            myMaterial = self.pivy.coin.SoMaterial()
            myMaterial.diffuseColor.setValue(color)
            parent.addChild(myMaterial)
        def handle_sphere(self, s, c, n):
            sep = self.pivy.coin.SoSeparator()
            self.root.addChild(sep)
            s.set_was_used(True)
            self.handle_color(sep, c)
            tr= self.pivy.coin.SoTransform()
            tr.translation.setValue(s.get_geometry().get_center()[0],
                                    s.get_geometry().get_center()[1],
                                    s.get_geometry().get_center()[2])
            tr.scaleFactor.setValue(s.get_geometry().get_radius(),
                                    s.get_geometry().get_radius(),
                                    s.get_geometry().get_radius())
            sep.addChild(tr)
            sphere=self.pivy.coin.SoSphere()
            sep.addChild(sphere)
            return True
        def handle_label(self, s, c, n):
            txt= s.get_text()
            loc= s.get_location()
            s.set_was_used(True)
            sep = self.pivy.coin.SoSeparator()
            self.handle_color(sep, c)
            tr= self.pivy.coin.SoTransform()
            tr.translation.setValue(loc.get_center()[0], loc.get_center()[1], loc.get_center()[2])
            sep.addChild(tr)
            to= self.pivy.coin.SoText2()
            to.string.setValue(txt)
            sep.addChild(to)
            return True
        def handle_cylinder(self, s, c, n):
            s.set_was_used(True)
            sep = self.pivy.coin.SoSeparator()
            self.root.addChild(sep)
            self.handle_color(sep, c)
            tr= self.pivy.coin.SoTransform()
            cyl= s.get_geometry()
            seg=cyl.get_segment()
            center= seg.get_middle_point()
        #tr.translation.setValue(self.pivy.coin.SbVec3f(0, seg.get_length()/2, 0))
            tr.translation.setValue(self.pivy.coin.SbVec3f(center[0], center[1],
                                                           center[2]))
        #tr.center.setValue(self.pivy.coin.SbVec3f(center[0], center[1], center[2]))
            uv= seg.get_direction().get_unit_vector()
            tr.rotation.setValue(self.pivy.coin.SbRotation(self.pivy.coin.SbVec3f(0,1,0),
                                                           self.pivy.coin.SbVec3f(uv[0],uv[1],uv[2])))
        #print "tr", tr.translation.getValue()[0], tr.translation.getValue()[1], tr.translation.getValue()[2]
        #print "scale", tr.scaleFactor.getValue()[0],  tr.scaleFactor.getValue()[1],  tr.scaleFactor.getValue()[2]
        # tr.rotation.getValue()
            sep.addChild(tr)
            sphere=self.pivy.coin.SoCylinder()
            sphere.radius.setValue(cyl.get_radius())
            sphere.height.setValue(cyl.get_segment().get_length())
        #sphere.addPart(self.pivy.coin.SoCylinder.ALL)
            sep.addChild(sphere)
            return True
        def show(self):
            self.set_was_used(True)
            self.pivy.sogui.SoGui.show(self.window) # Display main window
            self.pivy.sogui.SoGui.mainLoop()
    try:
        import pivy.sogui
    except:
        return None
    return PivyWriter
