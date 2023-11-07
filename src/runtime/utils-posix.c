#include "runtime/utils.h"
#include "runtime/checking.h"
#include <sys/time.h>

double yalx_current_mills_in_precision(void) {
    struct timeval jiffy;

    int rs = gettimeofday(&jiffy, NULL);
    DCHECK(rs == 0);
    USE(rs);

    return (double)jiffy.tv_sec * 1000.0f + (double)jiffy.tv_usec / 1000.0f;
}
