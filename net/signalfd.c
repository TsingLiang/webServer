#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/signalfd.h>

int main()
{
	struct signalfd_siginfo sigInfo;
	sigset_t mask;
	int sigfd;

	sigemptyset(&mask);
	sigfd = signalfd(-1, &mask, 0);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	
	assert(sigprocmask(SIG_BLOCK, &mask, NULL) >= 0);
	signalfd(sigfd, &mask, 0);
	assert(sigfd > 0);
	
	while(1)
	{
		int n = read(sigfd, &sigInfo, sizeof(sigInfo));
		assert(n == sizeof(sigInfo));
		
		if(sigInfo.ssi_signo == SIGINT)
			printf("get sig int.\n");	
		else if(sigInfo.ssi_signo == SIGQUIT)
			printf("get sig quit.\n");
		else
			printf("get sig quit.\n");
	}
}
