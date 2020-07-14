# pipe this to gnuplot or use -g option
set xlabel 'col1'
set ylabel 'col2'
set datafile separator ','
$data << EOD
-3,-1
-2,0
-1,1
0,0
1,-1
2,0
3,1
EOD
plot '$data' using 1:2 with linespoints title ''
pause mouse close
# pipe this to gnuplot or use -g option
