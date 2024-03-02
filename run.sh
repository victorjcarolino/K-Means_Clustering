# When making the parallel executable, we will need to edit some constants for use in creating TLS Views depending on the dataset we are using

datafile=$1
line=$(sed "1q" ${datafile})
total_values=$(echo "$line" | awk '{print $2}')
K=$(echo "$line" | awk '{print $3}')

dataline1=21
dataline2=22

sed -i "${dataline1}c\const int constK = $K;" src/kmeans-parallel.cpp
sed -i "${dataline2}c\const int const_total_values = $total_values;" src/kmeans-parallel.cpp

# First make the executables
sed -i '2c\FILE := kmeans-serial' Makefile
make

sed -i '2c\FILE := better-kmeans-serial' Makefile
make

sed -i '2c\FILE := kmeans-parallel' Makefile
make

mkdir outputs/$2

./bin/kmeans-serial $datafile > outputs/$2/kmeans-serial.txt
./bin/better-kmeans-serial $datafile > outputs/$2/better-kmeans-serial.txt
./bin/kmeans-parallel $datafile > outputs/$2/kmeans-parallel.txt