#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <nvml.h>

int
main(void)
{
	int r;
	int count;

	r = nvmlInit();
	if (r != NVML_SUCCESS) {
		fprintf(stderr, "nvmlInit(): %s(%d)\n", nvmlErrorString(r), r);
		exit(EXIT_FAILURE);
	}
	printf("Lib initialized\n");
	r = nvmlDeviceGetCount(&count);
	if (r != NVML_SUCCESS) {
		fprintf(stderr, "nvmlDeviceGetCount(): %s(%d)\n", nvmlErrorString(r), r);
		exit(EXIT_FAILURE);
	}
	printf("%d\n", count);

	return 0;
}
