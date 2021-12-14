
reset

# png
set terminal pngcairo size 1800,1200 enhanced font 'Verdana,30'
set output 'crash/png/full_throughput.png'

set border linewidth 5
set key outside above


# Set color of linestyle 2 to #8b0000
set style line 2 linecolor rgb '#8b0000' linetype 2 linewidth 5 pointtype 4 pointsize 3 dashtype 2
# Set yerror color of linestyle 3 to #8b0000
set style line 3 linecolor rgb '#8b0000' linetype 2 linewidth 3.75 pointtype 4 pointsize 3


# Set color of linestyle 4 to #0000ff
set style line 4 linecolor rgb '#0000ff' linetype 4 linewidth 5 pointtype 6 pointsize 3 dashtype 2
# Set yerror color of linestyle 5 to #0000ff
set style line 5 linecolor rgb '#0000ff' linetype 4 linewidth 3.75 pointtype 6 pointsize 3

# set axis
set tics scale 0.75
set xlabel 'Offered Load (Kops/s)'
set ylabel 'Throughput (Kops/s)'
set xrange [0:*]
set yrange [0:*]
plot 'crash/dat/read_throughput.dat' title 'Read' with yerrorbars linestyle 3, '' notitle with lines linestyle 2, 'crash/dat/write_throughput.dat' title 'Write' with yerrorbars linestyle 5, '' notitle with lines linestyle 4
