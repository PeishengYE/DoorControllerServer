FILE_LIST=`cat ./status.txt | grep modified | cut -d':' -f2`
for each in $FILE_LIST; do
	echo "copying $each"
	#cp $each ./arm_exe
	cp $each ./intel_exe
	#git checkout $each
done
