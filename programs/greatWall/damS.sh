printf "%20s%20s\n" "5分钟异常发生总次数:" `cat $1 | wc -l`

array_name=(
"driView:"
"MDDS:"
"roadRank:"
"acb:"
"roadRank-dispatch:"
"rtmiles:"
"driView2:"
"driView3:"
"rmq-java:"
"checkequipment:"
"releaseServer:"
"MDDS2:"
"rtmiles2:"
"roadRank_v2:"
"pbgive:"
"ashman:"
"driview_scene1:"
"driview_scene2:"
"driview_scene3:"
)

array_host=(
"172\.16\.51\.53\:4040"
"172\.16\.51\.62\:4120"
"172\.16\.71\.32\:4100"
"172\.16\.51\.97\:6868"
"172\.16\.51\.160\:4100"
"172\.16\.51\.102\:4140"
"172\.16\.51\.82\:4040"
"172\.16\.51\.56\:4040"
"172\.16\.51\.4\:8178"
"172\.16\.71\.33\:2222"
"172\.16\.51\.89\:4170"
"172\.16\.51\.24\:4120"
"172\.16\.51\.72\:4140"
"172\.16\.51\.91\:4100"
"172\.16\.51\.96\:8222"
"172\.16\.51\.151\:4080"
"172\.16\.51\.157\:4040"
"172\.16\.51\.158\:4040"
"172\.16\.51\.159\:4040"
)


for i in "${!array_name[@]}";
do
	printf "%20s%20s\n" ${array_name[i]} `cat $1 | grep ${array_host[i]} | wc -l`
done


while read LINE
do
	data=$LINE
	for i in "${!array_name[@]}";
	do
		data=`echo $data | grep -v ${array_host[i]}`
	done
	if [ "$data" !=  "" ];then
		echo $data
	fi
done < $1
