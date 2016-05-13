module("cfg")

DEFAULT_LOG_PATH = "./logs"

LOGLV = 0


collectTime, latitude, longitude, direction, onespeed = 1, 8, 7, 10, 11
time_sub_frame = 3600*5 --tsdb每次取5个小时的数据
min_sub_speed = 40 --用于当计算里程中当时间差为1秒时，允许两点的最大速度差值
packet_time = 11  --用于途径道路里程中最大丢包时间11秒 
min_count = 3000  --算里程一次允许取的最大点数
max_aver_speed = 130 --两点允许的最大平均速度（130公里每小时）
redis_gps_data_min_time = 3600*6 --关机前6小时的数据通过redis取，其余时间通过tsdb取
one_range_time = 600 ---200米范围 10分钟
