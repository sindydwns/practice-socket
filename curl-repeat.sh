mkdir -p ./test
for ((i=1; i<=20; i++))
do
	curl localhost:8080 -v > ./test/res_$i.txt &
done

wait

sleep 1

for ((i=1; i<20; i++))
do
	diff ./test/res_$i.txt ./test/res_$((i+1)).txt
	if [ $? -ne 0 ]; then
		echo ./test/res_$i.txt ./test/res_$((i+1)).txt
	fi
done

rm -rf ./test
