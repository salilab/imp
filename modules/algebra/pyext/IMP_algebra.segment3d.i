namespace IMP {
 namespace algebra {
  /* Always check input to get_point() */
  %feature("shadow") Segment3D::get_point %{
    def get_point(self, i):
        if i < 0 or i >= 2:
            raise IMP.base.UsageException("Invalid point index")
        return $action(self, i)
  %}
 }
}
