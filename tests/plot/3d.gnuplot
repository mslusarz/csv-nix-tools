# pipe this to gnuplot or use -g option
set xlabel 'col1'
set ylabel 'col2'
set zlabel 'col3'
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
splot '$data' using 1:2:3 with points title ''
pause mouse close
# pipe this to gnuplot or use -g option
