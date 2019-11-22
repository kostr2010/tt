#include <time.h>
#include <stdio.h>

int main() {
	printf("%d %d\n", time(NULL), time(NULL));
	
	time_t ltime;
	struct tm result;
	char stime[32];

	ltime = time(NULL);
	localtime_r(&ltime, &result);
	asctime_r(&result, stime);	
	
	printf("%s\n", stime);

	strftime(stime, 32, "%x", &result);

	printf("%s\n", stime);

	return 0;
}
