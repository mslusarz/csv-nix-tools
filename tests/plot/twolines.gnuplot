# pipe this to gnuplot or use -g option
set xlabel 'col1' noenhanced
set datafile separator ','
$data << EOD
-3,-1,-6
-2,0,-4
-1,1,-2
0,0,0
1,-1,2
2,0,4
3,1,6
EOD
plot '$data' using 1:2 with linespoints title 'col2' noenhanced, '' using 1:3 with linespoints title 'col3' noenhanced
pause mouse close
# pipe this to gnuplot or use -g option
