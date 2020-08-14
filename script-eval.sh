#!/bin/sh
#All directories accessed are wrt gem5 home directory
echo "Script executing..."

# echo "Enter 0 to rebuild gem5, 1 to continue with the script (no gem5 build)..."
# read b
#if [ "${b}" -eq 0 ]; then 
#	echo "Building gem5..."
#	python `which scons` ./build/X86/gem5.opt
#fi

#Input file generated is located at ./workload/merkle/ as input.txt
if [ -e ./workload/merkle/input_gen.txt ]; then
	echo "input_file exists. Please delete current file and rerun this script if you want to regenerate the input."
	echo "Continuing to simulation"
else
	echo "Enter the size of the random file to be generated in bytes:"
	size=16777216 
	#read size
	echo "Generating input file. Please wait..."
	python3 ./workload/merkle/InputGen.py --file-size=${size} 
	echo "File generated at ./merkle/ with name input_gen.txt"
fi

#we should restrict to using a block size which is <= than mem-size.
#Iterate through the following block sizes
#Block size chosen such that it is either <= than L1, or <= than L2, or > L2. 
#This gives us 3 different configs for an input of given size. 
#1kb, 64kb, 8mb, 16mb
blocksizes=( 1024 65536 8388608 16777216 )
blocksizenames=( "1k" "64k" "8m" "16m" )
l=${#blocksizes[@]}
echo "Starting simulations"
gem5=gem5.opt
cachesize=64kB
l2_size=4MB
#Executing merkle tree with 3 different configurations
for((i=0;i<l;i++)); do
	blocksize=${blocksizes[$i]}
	blocksizename=${blocksizenames[$i]}
	echo "========== DDR,        Blocksize: ${blocksizenames[$i]} ($blocksize bytes) =========="
	#g++ -o merkle_ddr -static -O0 -lm ./merkle/merkle_ddr.c ./merkle/helper.c ./merkle/sha256.c -I ./include/ ./util/m5/src/x86/m5op.S
	#build/X86/$gem5  configs/example/se.py --cpu-type=TimingSimpleCPU --cpu-clock=1GHz --caches --l2cache --l1d_size=$cachesize --l1i_size=$cachesize --l2_size=$l2_size --l1d_assoc=4 --l1i_assoc=4 --l2_assoc=8 --mem-size=512MB --coherence-granularity=64B --mem-type=DDR3_1600_8x8 -c merkle_ddr -o "./workload/merkle/input_gen.txt $blocksize" > ./m5out/outputlog_ddr.txt 2>./m5out/errlog_ddr.txt
	mv ./m5out/stats.txt ./m5out/merkle_ddr.txt

	echo "========== HMC + PIM,  Blocksize: ${blocksizenames[$i]} ($blocksize bytes) =========="
	#g++ -o merkle_pim -static -O0 -lm ./merkle/merkle_pim.c ./merkle/helper.c ./merkle/sha256.c -I ./include/ ./util/m5/src/x86/m5op.S
	#build/X86/$gem5  configs/example/se.py --cpu-type=TimingSimpleCPU --cpu-clock=1GHz --caches --l2cache --l1d_size=$cachesize --l1i_size=$cachesize --l2_size=$l2_size --l1d_assoc=4 --l1i_assoc=4 --l2_assoc=8 --mem-size=512MB --coherence-granularity=64B --mem-type=HMC_2500_1x32 -c merkle_pim -o "./workload/merkle/input_gen.txt $blocksize" > ./m5out/outputlog_pim.txt 2>./m5out/errlog_pim.txt
	mv ./m5out/stats.txt ./m5out/merkle_pim.txt

#Generating a stats comparison file
#./m5out/input_attr.txt has a list of attributes to be compared, new attributes can be added or removed
	python3 ./merkle/StatsComparison.py --file-path="./m5out/merkle_ddr.txt;./m5out/merkle_pim.txt" --output-path="./m5out/output_comparison.txt" --attr-path="./merkle/input_attr.txt"
	target_out_dir=m5out-$blocksizename
	echo "Moving m5out to $target_out_dir"
	mkdir -p $target_out_dir
	mv m5out $target_out_dir
	echo "Comparison file  and output logs can be found at $target_out_dir "
done
