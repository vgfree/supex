curl -v -0  -T 123.wav   "127.0.0.1:80/saveSound?filename=18696770041_1379903830_xxx.wav&duration=50&codec=MPEG-1%20Layer%203&channels=1&rate=16000&bit=8"
 curl -v -0  --upload-file 123.wav   "127.0.0.1:80/saveSound?filename=18696770041_1379903830_xxx.wav&duration=50&codec=MPEG-1%20Layer%203&channels=1&rate=16000&bit=8"

curl -v -0  -d "asdasdas"   "127.0.0.1:80/saveSound?filename=18696770041_1379903830_xxx.wav&duration=50&codec=MPEG-1%20Layer%203&channels=1&rate=16000&bit=8"

wget -O "out.txt" --user-agent="Mozilla/5.0" --post-file=$1 -c "127.0.0.1:80/saveSound?filename=18696770041_1379903830_xxx.wav&duration=50&codec=MPEG-1%20Layer%203&channels=1&rate=16000&bit=8"
