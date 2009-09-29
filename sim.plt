set term aqua
set grid
set yrange [0:*]
set xlabel "Days"
set ylabel "People"
plot 'report.out' using  2:7 title 'I' with li lw 3 lt 1
replot 'report.out' using 2:((int($2))%2==0 ? ($7) : 1/0):8 notitle with errorlines lt 1
replot 'report.out' using  2:5 title 'E' with li lw 3 lt 3
replot 'report.out' using  2:((int($2))%2==0 ? ($5) : 1/0):6 notitle with errorlines lt 3
#replot 'report.out' using  2:9 title 'R' with li lw 3
#replot 'report.out' using  2:3 title 'S' with li lw 3



