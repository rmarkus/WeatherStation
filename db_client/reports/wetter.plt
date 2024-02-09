reset session
set timefmt "%Y-%m-%d %H:%M:%S"
set xdata time
set format x "%m-%d\n%H:%M"
#set yrange [0:30]
#set y2range [1000:1100]
set ytics nomirror 
set y2tics 
set datafile separator '|'
#plot 'wetter2.csv' using 1:2 axes x1y1 with lines, 'pressure.csv' using 1:2 axes x1y2 with lines
plot 'temp.dat' using 1:2 axes x1y1 with lines
