module("CONFIG_LIST")


OWN_LIST = {
	["p2p_continuous_driving_mileage_remind"] = {
		["spx_txt_to_url_send_weibo"] = {
			["level"]    = 80,
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_fatigue_driving"] = {
		["half_url_incr_idx_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/productList/fatigueDriving/%s.amr",
			["idx_key"]  = "driveOnlineHoursPoint",
			["idx_base"] = "30000",
			["app_key"]  = "179429838",		
			["idx_max"]  = 28,
			["secret"]   = "D2715E17E86D292BB2B9EA494E532F85F22EE9DB",
			["interval"] = 300,
		},
	},
	["p2p_over_speed"] = {
		["half_url_random_idx_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/productList/overSpeed/%s.amr",
			["idx_base"] = "30000",
			["idx_max"]  = 28,
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
		["spx_txt_to_url_send_weibo"] = {
			["level"]    = 80,
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_offsite_remind"] = {
		["half_url_unknow_str_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/productList/OffsiteRemind/%s.amr",
			["fill_key"]  = "driveOnlineHoursPoint",
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_solarcalendar"] = {
		["half_url_power_on_unknow_str_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/productList/solarCalendar/%s.amr",
			["fill_key"]  = "driveOnlineHoursPoint",
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_weather_forcast"] = {
		["spx_txt_to_url_send_weibo"] = {
			["level"]    = 80,
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["power_on_weibo"] = {
		["half_url_power_on_unknow_str_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/productList/powerOnGreetings/%s.amr",
			["fill_key"]  = "driveOnlineHoursPoint",
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_road_traffic_remind"] = {
		["spx_txt_to_url_send_weibo"] = {
			["level"]    = 80,
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_fetch_4_miles_ahead_poi"] = {
		["half_url_unknow_str_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/newproductList/POIRemind/%s.amr",
			["fill_key"]  = "driveOnlineHoursPoint",
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_assemble_scene"] = {
		["half_url_unknow_str_send_weibo"] = {
			["level"]    = 80,
			["halfURL"]  = "http://127.0.0.1/productList/assembleScene/%s.amr",
			["fill_key"]  = "driveOnlineHoursPoint",
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},
	["p2p_system_update"] = {
		["spx_txt_to_url_send_weibo"] = {
			["level"]    = 80,
			["app_key"]  = "3406572696",
			["secret"]   = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
			["interval"] = 300,
		},
	},

    --[[
	["a_d_fetch_4_miles_ahead_poi"] = {
		["bool"] = {
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["half_url_mult_idx_send_weibo"] = {
				["level"] = 70,
				["secret"] = "52E8DCDEB8DBBAD220652851AE34339B008F5B48",
				["idx_key"] = "4MilesAheadPositionTypeSet",
				["app_key"] = "2491067261",
				["halfURL"] = "http://127.0.0.1/productList/POIRemind/%s.amr",
				["carry_key"] = "4MilesAheadPositionCarry",
				["interval"] = 30,
			},
		},
	},
	["a_d_highway_over_speed"] = {
		["bool"] = {
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["full_url_send_weibo"] = {
				["level"] = 25,
				["secret"] = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
				["app_key"] = "3406572696",
				["fullURL"] = "http://127.0.0.1/productList/overSpeed/urbanArea/20001.amr",
				["interval"] = 300,
			},
		},
	},
	["a_d_urban_over_speed"] = {
		["bool"] = {
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["full_url_send_weibo"] = {
				["level"] = 25,
				["secret"] = "04A069DD5AAFE20712CFE846650E02D239C1D4C1",
				["app_key"] = "3406572696",
				["fullURL"] = "http://127.0.0.1/productList/overSpeed/highway/20001.amr",
				["interval"] = 300,
			},
		},
	},
	["a_d_driving_mileage"] = {
		["bool"] = {
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["half_url_incr_idx_send_weibo"] = {
				["level"] = 80,
				["halfURL"] = "http://127.0.0.1/productList/drivingMileage/%s.amr",
				["idx_key"] = "driveOnlineMileagePoint",
				["idx_max"] = 51,
				["app_key"] = "1106086205",
				["idx_base"] = "40000",
				["secret"] = "C6BF9297AAE1A12E267FBE4DED1C99FB54132607",
				["interval"] = 300,
			},
		},
	},
    ]]--
}
