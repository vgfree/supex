module("BOOL_FUNC_LIST", package.seeall)

--不能有空格
OWN_HINT = {
	is_off_site = {
	},
	drive_online_point = {
	},
	check_time_is_between_in = {
	},
	is_over_speed = {
		speed = 125,
	},
}

OWN_ARGS = {
	is_off_site = {
	},
	-->> time
	drive_online_point = {
		increase = 60*60,
	},
	check_time_is_between_in = {
		time_start = 23*60*60,
		time_end = 4*60*60
	},
	-->> direction
	is_over_speed = {
		speed = 125,
	},

}

OWN_LIST = {
	-->> poistion
	is_off_site = function( app_name, key )
		return string.format('judge.is_off_site("%s")', app_name)
	end,

	-->> time
	drive_online_point = function( app_name, key )
		return string.format('judge.drive_online_point("%s")', app_name)
	end,
	check_time_is_between_in = function (  app_name, key  )
		return string.format('judge.check_time_is_between_in("%s")', app_name)
	end,
	-->> direction
	is_over_speed = function( app_name, key )
		return string.format('judge.is_over_speed("%s")', app_name)
	end,

	is_road_traffic = function( app_name, key )
		return string.format('judge.is_road_traffic("%s")', app_name)
	end,

}
