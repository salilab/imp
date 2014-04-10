foreach i ( 1 2 3 4 5 6 7 8) ;
  bash make_energy.sh LOGno$i > ENERGYno$i
  bash make_energy.sh LOGyes$i > ENERGYyes$i
end
#gnuplot compare.gnu -p
