//
//  main.c
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include "prg_frame.h"

int main(int argc, char **argv)
{
	TRY
	{
		// initialize frame
		initialize(argc, argv);
		// run frame
		run();
	}
	CATCH
	{
		// stop frame
		stop();
	}
	FINALLY
	{
		// clean
		finally();
	}
	END;

#ifdef USE_MEMHOOK
  #include "node_manager.h"
	ManagerPrint("./memleak.tace", 0);
#endif

	return EXIT_SUCCESS;
}

