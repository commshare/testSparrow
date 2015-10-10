#ifndef _SC_ERROR_H_
#define _SC_ERROR_H_

#include <errno.h>
#include <stddef.h>

/* error handling */
#define DTERROR(e) (-(e))
#define DTUNERROR(e) (-(e))

/*common*/
#define SCERROR_NONE               DTERROR(0)
#define SCERROR_FAIL               DTERROR(1)

/*dtplayer*/
#define SCERROR_READ_FAILED        DTERROR(100)
#define SCERROR_READ_TIMEOUT       DTERROR(101)
#define SCERROR_READ_AGAIN         DTERROR(102)

#define SCERROR_READ_EOF           DTERROR(150)
#endif
