#先通过config先获取tokencode 
#http://192.168.71.55/config?kernelver=3_3_0&verno=drivereyes_V151227_1.0_P3_1.0.2&imsi=915445969973651&mod=SG900&imei=915445969973651&androidver=4_4_2&modelver=CV5021CBDL7FG_0022&basebandver=M6290A2_408_WM930_2___Nov_22_2013_07_44_17__EC78_&buildver=CV5021CBDL7FG_0022_20151209
#然后运行脚步 进行测试

while true
do
time1=170616015820
time2=$((time1+1))
time3=$((time2+1))
time4=$((time3+1))
TOKENCODE="Rz4ZTblyzP"
#echo $time1
#echo $time2
#echo "http://192.168.71.55/newstatus" -d "mt=25760&nt=0&imei=915445969973651&imsi=915445969973651&mod=SG900&tokencode=$TOKENCODE&gps=$time1,121.4116563E, 31.209886N,220,4,9;$time2,121.4116563E,31.209886N,251,2,8;$time3,121.4116563E, 31.209886N,264,3,7;$time4,121.4116563E, 31.209886N,22,22,0"
curl -v "http://192.168.71.55/newstatus" -d "mt=25760&nt=0&imei=915445969973651&imsi=915445969973651&mod=SG900&tokencode=$TOKENCODE&gps=$time1,121.4116563E, 31.209886N,220,4,9;$time2,121.4116563E,31.209886N,251,2,8;$time3,121.4116563E, 31.209886N,264,3,7;$time4,121.4116563E, 31.209886N,22,22,0"
#sleep 1
done
