set timefmt "%Y-%m-%d"
set datafile separator '|'
set xdata time
set format x "%d.%m.%y"
set style fill solid
set boxwidth 60000 absolute
plot 'regen.dat' using 1:2 axes x1y1 with boxes

