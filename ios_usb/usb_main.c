static uint32_t[2] default_uptime = { 0, 0 }; //.rodata:101403F8
static int32_t logHandle = -1; //.data:10145000, written by .text:101001c0

// This compiled to some heckin wonky math in the original binary.
// Goal here is to have the same semantics, not to be instruction-perfect.
#define STATUS_MAKE(hi, lo) (((uint16_t)hi << 16) | (uint16_t)lo)
#define STATUS_LO(x) (x & 0xFFFF)
#define STATUS_HI(x) ((x >> 16) & 0xFFFF)

//.text:10100000 (note: not entrypoint!)
int32_t __startRootHubs(int options) {
	uint32_t[4] errlist; //SP+4-0x14
	uint32_t[2] uptime; //SP+0x14, +0x18
	int32_t status, ret;

	//no idea why they do it like this
	memcpy(&uptime, &default_uptime, sizeof(default_uptime));
	IOS_GetUpTimeStruct(&uptime); //UND #0x190
	USBVReport(logHandle, 3, "Activating root hubs @ uptime %d.%03d s with options 0x%x.\n", uptime[1], uptime[0] / 1000, options); //.text:1012CD80, looks a lot like COSVReport

	ret = UhsServerAddHc(dword_10145020, 0, options); //.text:1010fd28
	if ( ret < 0 ) { //negative = error
		if ( STATUS_LO(ret) < 0 ) {
			status = STATUS_MAKE(-1, STATUS_LO(ret));
		} else {
			status = STATUS_MAKE(0, STATUS_LO(ret));
		}
		USBVReport(
			logHandle, 2,
			"%s, file %s, function %s, line %d, status %d:%d\n",
			"UhsServerAddHc 0",
#ifdef ACCURATE
			"usb_main.c",
			"__startRootHubs",
			371,
#else
			__FILE__,
			__FUNCTION__,
			__LINE__,
#endif
			(~err >> 16) & 0x3FF, err);
	} else {
		USBVReport(logHandle, 0, "%s OK.\n", "UhsServerAddHc 0");
	}

	ret = UhsServerAddHc(dword_10145020, 1, options);
	if ( ret < 0 ) { //negative = error
		/* The original code for this isn't nearly as nice, presumably there
		   was some struct or other thing involved originally */
		if ( STATUS_LO(ret) < 0 ) {
			status = STATUS_MAKE(-1, STATUS_LO(ret));
		} else {
			status = STATUS_MAKE(0, STATUS_LO(ret));
		}

		USBVReport(
			logHandle,
			2,
			"%s, file %s, function %s, line %d, status %d:%d\n",
			"UhsServerAddHc 1",
#ifdef ACCURATE
			"usb_main.c",
			"__startRootHubs",
			374,
#else
			__FILE__,
			__FUNCTION__,
			__LINE__,
#endif
			(~status >> 16) & 0x3FF,
			status);
	} else {
		USBVReport(logHandle, 0, "%s OK.\n", "UhsServerAddHc 1");
	}

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
