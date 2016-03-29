//
//  main.c
//  supex
//
//  Created by 周凯 on 15/9/16.
//  Copyright © 2015年 zk. All rights reserved.
//

#include "prg_frame.h"

int main(int argc, char **argv)
{
	TRY
	{
		init(argc, argv);
		run();
	}
	CATCH
	{
		stop();
	}
	FINALLY
	{
		finally();
	}
	END;

	return EXIT_SUCCESS;
}

