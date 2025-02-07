module("cfg")

DEFAULT_LOG_PATH = "./logs"

LOGLV = 0

CHECK_DISTANCE = 0.001

INTERVAL_TIME = 65

area_bl = {
	['ShangHai'] = {
		['B1'] = 73,
		['L1'] = 30,
		['B2'] = 112.6,
		['L2'] = 54,
	},
	['BeiJing']  = {
		['B1'] = 112.6,
		['L1'] = 38.36,
		['B2'] = 136,
		['L2'] = 54,
	},
	['LinYi'] = {
		['B1'] = 112.6,
		['L1'] = 34.48,
		['B2'] = 136,
		['L2'] = 38.36,
	},
	['ChengDu'] = {
		['B1'] = 112.6,
		['L1'] = 30.42,
		['B2'] = 136,
		['L2'] = 34.48,
	},
	['ChongQing'] = {
		['B1'] = 112.6,
		['L1'] = 3,
		['B2'] = 136,
		['L2'] = 30.42,
	},
	['GuangZhou'] = {
		['B1'] = 73,
		['L1'] = 3,
		['B2'] = 112.6,
		['L2'] = 30.12,
	}
}
