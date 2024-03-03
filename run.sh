# When making the parallel executable, we will need to edit some constants for use in creating TLS Views depending on the dataset we are using

declare -a filenames

dir="datasets"

for file in "$dir"/*; do
    filenames+=("$file")
done

for datafile in "${filenames[@]}"; do
    # extract the K and total_values for kmeans-parallel because they must be known at compile-time
    line=$(sed "1q" ${datafile})
    total_values=$(echo "$line" | awk '{print $2}')
    K=$(echo "$line" | awk '{print $3}')

    dataline1=21
    dataline2=22

    # replace the constant compile-time K and total_values for Kmeans-parallel
    sed -i "${dataline1}c\const int constK = $K;" src/kmeans-parallel.cpp
    sed -i "${dataline2}c\const int const_total_values = $total_values;" src/kmeans-parallel.cpp

    # Make the executables
    sed -i '2c\FILE := kmeans-serial' Makefile
    make

    sed -i '2c\FILE := better-kmeans-serial' Makefile
    make

    sed -i '2c\FILE := kmeans-parallel' Makefile
    make

    # extract the subdirectory name from the data filename
    temp=${datafile##*/}
    subdirectory=${temp%.*}

    # make the output and execution_data subdirectories if needed
    mkdir -p outputs/$subdirectory
    mkdir -p execution_data/$subdirectory
    mkdir -p outputs/correctness_check

    # execute 100 iterations to get sufficient data on execution times due to small variations
    
    for i in {1..100}; do
        ./bin/kmeans-serial $datafile > outputs/$subdirectory/kmeans-serial.txt
        ./bin/better-kmeans-serial $datafile > outputs/$subdirectory/better-kmeans-serial.txt
        ./bin/kmeans-parallel $datafile > outputs/$subdirectory/kmeans-parallel.txt

        # check for correctness
        tail -n +5 outputs/$subdirectory/kmeans-serial.txt > outputs/correctness_check/temp1
        tail -n +5 outputs/$subdirectory/better-kmeans-serial.txt > outputs/correctness_check/temp2
        tail -n +5 outputs/$subdirectory/kmeans-parallel.txt > outputs/correctness_check/temp3

        # Compare the files
        diff1=$(diff outputs/correctness_check/temp1 outputs/correctness_check/temp2)
        diff2=$(diff outputs/correctness_check/temp1 outputs/correctness_check/temp3)

        # Check if the diff outputs are empty (i.e., the files are the same)
        if [ -z "$diff1" ] && [ -z "$diff2" ]; then
            echo "[CORRECTNESS CHECK]: PASSED"
        else
            echo "[CORRECTNESS CHECK]: FAILED"
        fi

        # Clean up the temporary files
        rm -rf outputs/correctness_check/*

        # gather the execution times
        declare -a s_times
        declare -a bs_times
        declare -a p_times
        temp_s=$(sed -n '3p' outputs/$subdirectory/kmeans-serial.txt)
        temp_bs=$(sed -n '3p' outputs/$subdirectory/better-kmeans-serial.txt)
        temp_p=$(sed -n '3p' outputs/$subdirectory/kmeans-parallel.txt)
        time_s=$(echo $temp_s | cut -d'=' -f2 | tr -d ' ')
        time_bs=$(echo $temp_bs | cut -d'=' -f2 | tr -d ' ')
        time_p=$(echo $temp_p | cut -d'=' -f2 | tr -d ' ')
        s_times+=("$time_s")
        bs_times+=("$time_bs")
        p_times+=("$time_p")
    done

    declare -a s_data
    declare -a bs_data
    declare -a p_data
    arrays=(s_times bs_times p_times)
    data_arrs=(s_data bs_data p_data)

    # Define the CSV file
    csv_file="execution_times.csv"

    # Write the header to the CSV file
    echo "Datafile,Execution Type,Min Time (us),Max Time (us),Mean Time (us)" > $csv_file

    # Outer loop over the arrays
    for index in ${!arrays[@]}; do
        # Use indirect reference to get the actual array
        arrayname=${arrays[index]}
        array=("${!arrayname[@]}")

        # Calculate the sum
        sum=0
        # Initialize min and max to the first element
        min=${array[0]}
        max=${array[0]}
        for num in "${array[@]}"; do
            ((sum += num))
            # Update min if necessary
            if ((num < min)); then
                min=$num
            fi
            # Update max if necessary
            if ((num > max)); then
                max=$num
            fi
        done

        # Calculate the mean
        mean=$(echo "scale=2; $sum / ${#array[@]}" | bc)

        # Write the data to the CSV file
        execution_type=""
        case $index in
            0) execution_type="Serial" ;;
            1) execution_type="Better Serial" ;;
            2) execution_type="Parallel" ;;
        esac

        # Append the data to the CSV file
        echo "$datafile,$execution_type,$min,$max,$mean" >> $csv_file
    done

    # Define the speedup arrays
    declare -a speedup_bs_to_s
    declare -a speedup_p_to_s
    declare -a speedup_p_to_bs

    # Calculate the speedup for better-serial to serial
    for index in ${!s_data[@]}; do
        speedup=$(echo "scale=2; ${s_data[index]} / ${bs_data[index]}" | bc)
        speedup_bs_to_s+=($speedup)
    done

    # Calculate the speedup for parallel to serial
    for index in ${!s_data[@]}; do
        speedup=$(echo "scale=2; ${s_data[index]} / ${p_data[index]}" | bc)
        speedup_p_to_s+=($speedup)
    done

    # Calculate the speedup for parallel to better-serial
    for index in ${!bs_data[@]}; do
        speedup=$(echo "scale=2; ${bs_data[index]} / ${p_data[index]}" | bc)
        speedup_p_to_bs+=($speedup)
    done

    # Define the CSV files
    csv_file_s="speedup_to_s.csv"
    csv_file_bs="speedup_to_bs.csv"

    # Write the headers to the CSV files
    echo "Datafile,Execution Type,Speedup" > $csv_file_s
    echo "Datafile,Execution Type,Speedup" > $csv_file_bs

    # Calculate the speedup for better-serial to serial
    for index in ${!s_data[@]}; do
        speedup=$(echo "scale=2; ${s_data[index]} / ${bs_data[index]}" | bc)
        speedup_bs_to_s+=($speedup)

        # Append the data to the CSV file
        echo "$datafile,better-serial,$speedup" >> $csv_file_s
    done

    # Calculate the speedup for parallel to serial
    for index in ${!s_data[@]}; do
        speedup=$(echo "scale=2; ${s_data[index]} / ${p_data[index]}" | bc)
        speedup_p_to_s+=($speedup)

        # Append the data to the CSV file
        echo "$datafile,parallel,$speedup" >> $csv_file_s
    done

    # Calculate the speedup for parallel to better-serial
    for index in ${!bs_data[@]}; do
        speedup=$(echo "scale=2; ${bs_data[index]} / ${p_data[index]}" | bc)
        speedup_p_to_bs+=($speedup)

        # Append the data to the CSV file
        echo "$datafile,parallel,$speedup" >> $csv_file_bs
    done

    
done
