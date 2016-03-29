package.cpath = "../lib/?.so;./lib/?.so;../?.so;" .. package.cpath
local ffi = require('ffi');

ffi.cdef[[
	struct timeval {
		long	tv_sec;
		int	tv_usec;
	};
	struct tm {
		int	tm_sec;	
		int	tm_min;	
		int	tm_hour;
		int	tm_mday;
		int	tm_mon;	
		int	tm_year;
		int	tm_wday;
		int	tm_yday;
		int	tm_isdst;
		long	tm_gmtoff;
		char	*tm_zone;
	};
	int gettimeofday(struct timeval *, void *);
	struct tm *localtime_r(const long *, struct tm *);
	size_t strftime(char *, size_t, const char *, const struct tm *);
]];

function print_time()
	local tm = ffi.new('struct tm[1]');
	local tv = ffi.new('struct timeval[1]');
	local timeptr = ffi.cast('long *', tv);
	local timestr = ffi.new('char[32]');
	
	ffi.C.gettimeofday(tv, nil);
	ffi.C.localtime_r(timeptr, tm);
	ffi.C.strftime(timestr, ffi.sizeof('char[32]'), [[%Y-%m-%d %H:%M:%S]], tm);
	print(ffi.string(timestr),tonumber(tv[0].tv_usec));
end

print_time()
