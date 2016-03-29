#!bash
#ffmpeg -i test.mp4 -codec:v libx264 -codec:a mp3 -map 0 -f ssegment -segment_format mpegts -segment_list index.m3u8 -segment_time 10 %03d.ts

#cvlc list.m3u8

cvlc step.m3u8
