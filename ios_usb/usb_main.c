uint32_t[2] default_uptime = { 0, 0 }; //.rodata:101403F8

void __startRootHubs(int options) { //TODO: figure out that wonky math for error status
	uint32_t[4] errlist; //SP+4-0x14
	uint32_t[2] uptime; //SP+0x14, +0x18
	//no idea why they do it like this
	memcpy(&uptime, &dword_101403F8, sizeof(uptime));
	IOS_GetUpTimeStruct(&uptime);
	sub_1012CD80(dword_10145000, 3, "Activating root hubs @ uptime %d.%03d s with options 0x%x.\n", uptime[1], uptime[0] / 1000, options);
	ret = UhsServerAddHc(dword_10145020, 0, options);
	if ( ret >= 0 ) { //negative = error
		sub_1012CD80(dword_10145000, 0, "%s OK.\n", "UhsServerAddHc 0");
		ret = UhsServerAddHc(dword_10145020, 1, options);
		if ( ret >= 0 ) { //negative = error
			sub_1012CD80(dword_10145000, 0, "%s OK.\n", "UhsServerAddHc 1");
		} else {
			if ( ret & 0x8000 )
				v7 = ~(ret << 16);
			else
				v7 = ret << 16;
			if ( ret & 0x8000 )
				v8 = ~(v7 >> 16);
			else
				v8 = v7 >> 16;
			sub_1012CD80(
				dword_10145000,
				2,
				"%s, file %s, function %s, line %d, status %d:%d\n",
				"UhsServerAddHc 1",
				"usb_main.c",
				"__startRootHubs",
				0x176,
				(~ret >> 16) & 0x3FF,
				v8);
		} //fall-through
	} else {
		if ( ret & 0x8000 )
			v5 = ~(ret << 16);
		else
			v5 = ret << 16;
		if ( ret & 0x8000 )
			v6 = ~(v5 >> 16);
		else
			v6 = v5 >> 16;
		sub_1012CD80(
			dword_10145000,
			2,
			"%s, file %s, function %s, line %d, status %d:%d\n",
			"UhsServerAddHc 0",
			"usb_main.c",
			"__startRootHubs",
			0x173,
			(~ret >> 16) & 0x3FF,
			v6);
	} //fall-through, no return(?)
}

sub_1012CE60(R0, R1) {
	if (&dword_101450C0 < 0) //last error?
		ret = IOS_CreateSemaphore(1, 1); //???
		if (ret < 0) {
			sub_1012F250("LOG ERROR: mutex init error!\n");
			return -1;
		}
		ret = IOS_WaitSemaphore(&dword_101450C0, 0);
		if (ret != 0) {
			sub_1012F250("LOG ERROR: log_open(%s, %d, ...) failed\n", R0, R1);
			return -1;
		}



	IOS_SignalSemaphore(&dword_101450C0);
	//unfinished cuz not dealing with this bs tonight, some loop conditional shenanigans
}
