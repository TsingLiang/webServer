#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int main()
{
	struct itimerspec time;
	struct timeval now;
	
	gettimeofday(&now, NULL);

	time.it_interval.tv_sec = 1;
	time.it_interval.tv_nsec = 0;
	time.it_value.tv_sec = now.tv_sec + 1; 
	time.it_value.tv_nsec = now.tv_usec * 1000;
	
	int fd = timerfd_create(CLOCK_REALTIME, 0);
	assert(fd > 0);
	
	timerfd_settime(fd, TFD_TIMER_ABSTIME, &time, NULL);
	
	int count = 0;

	while(1)
	{
		uint64_t exp;
		int n = read(fd, &exp, sizeof(exp));
		assert(n == sizeof(exp));
		printf("time out %d.\n", ++count);
	}

	return 0;
}
