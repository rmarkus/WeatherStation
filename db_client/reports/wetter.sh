#export MYSQL_HOST=....
#export MYSQL_PWD=...
echo "select date_time, val  from wetter where domain like '%temp%';" | mariadb -u wetter wetterdb | sed 's/\t/|/g' > temp.dat
echo "select date(date_time), sum(val) from wetter w where domain like '%regen%' group by date(date_time);" | mariadb  -u wetter wetterdb | sed 's/\t/|/g' > regen.dat

gnuplot wetter.plt -
gnuplot rain.plt -
