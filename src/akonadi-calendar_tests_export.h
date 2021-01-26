#ifndef AKONADICALENDARTEST_EXPORT_H
#define AKONADICALENDARTEST_EXPORT_H

#include "akonadi-calendar_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef AKONADI_CALENDAR_TESTS_EXPORT
#define AKONADI_CALENDAR_TESTS_EXPORT AKONADI_CALENDAR_EXPORT
#endif
#else /* not compiling tests */
#define AKONADI_CALENDAR_TESTS_EXPORT
#endif

#endif
