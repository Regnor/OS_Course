#include "exp.h"

NTSTATUS NtMyNewSystemCall()
{
	static long callCount = 0;
	DbgPrint("WRK NtMyNewSystemCall: %d\n", ++callCount);
	return STATUS_SUCCESS;
}