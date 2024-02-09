reset session
set timefmt "%Y.%m.%d"
set xdata time
#set format x "%m-%d\n%H:%M"
#set yrange [0:30]
#set y2range [1000:1100]
set ytics nomirror 
#set y2tics 
set datafile separator '|'
plot 'temp2.dat' using 1:2 with lines title "max", 'temp2.dat' using 1:3 with lines title "min", 'temp2.dat' using 1:4  with lines title "avg"
