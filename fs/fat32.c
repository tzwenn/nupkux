#include <fs/fat32.h>
#include <mm.h>
#include <kernel/devices/fdc.h>

UINT device_read(UCHAR *device, UINT pos, UINT n, UCHAR* buffer)
{
	UINT len = (n%512)?(n/512+512):(n/512);
	UCHAR *buf = (UCHAR *)malloc(len);
	if (!fdc_read_block(pos,buf,len)) return 0;
	memcpy(buffer,buf,n);
	free(buf);
	return n;
}

fat32discr fat32_read_discr(UCHAR *device)
{
	
}
