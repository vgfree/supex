module("BOOL_FUNC_LIST", package.seeall)

--不能有空格
OWN_HINT = {
}

OWN_ARGS = {
	-->> poistion
	is_4_miles_ahead_have_poi = {
		type_delay = 300,
	},
}

OWN_LIST = {
	-->> poistion
	is_4_miles_ahead_have_poi = function( app_name, key )
		return string.format('judge.is_4_miles_ahead_have_poi("%s")', app_name)
	end,
}
