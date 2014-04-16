#  set term wxt font ",5"
#  #set terminal pdfcairo font ",5"
#  #set output "tmp.pdf"
#  set multiplot layout 8,1        # engage multiplot mode
#  plot [][0:75]'ENERGYno1' u 1:3 every 50 w l, 'ENERGYyes1' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno2' u 1:3 every 50 w l, 'ENERGYyes2' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno3' u 1:3 every 50 w l, 'ENERGYyes3' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno4' u 1:3 every 50 w l, 'ENERGYyes4' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno5' u 1:3 every 50 w l, 'ENERGYyes5' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno6' u 1:3 every 50 w l, 'ENERGYyes6' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno7' u 1:3 every 50 w l, 'ENERGYyes7' u 1:3 every 50 w l
#  plot [][0:75]'ENERGYno8' u 1:3 every 50 w l, 'ENERGYyes8' u 1:3 every 50 w l
#  unset multiplot

set term x11 2 font ",5"
set multiplot layout 6,2
plot [][0:75]'ENERGYno1' u 1:3 every 50 w l
plot [][0:75]'ENERGYno2' u 1:3 every 50 w l
plot [][0:75]'ENERGYno3' u 1:3 every 50 w l
plot [][0:75]'ENERGYno4' u 1:3 every 50 w l
plot [][0:75]'ENERGYno5' u 1:3 every 50 w l
plot [][0:75]'ENERGYno6' u 1:3 every 50 w l
plot [][0:75]'ENERGYno7' u 1:3 every 50 w l
plot [][0:75]'ENERGYno8' u 1:3 every 50 w l
plot [][0:75]'ENERGYno9' u 1:3 every 50 w l
plot [][0:75]'ENERGYno10' u 1:3 every 50 w l
plot [][0:75]'ENERGYno11' u 1:3 every 50 w l
plot [][0:75]'ENERGYno12' u 1:3 every 50 w l
unset multiplot
