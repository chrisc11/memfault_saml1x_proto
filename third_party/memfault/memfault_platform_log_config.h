//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
// Logging depends on how your configuration does logging. See
// https://docs.memfault.com/docs/embedded/self-serve/#implement-logging-dependency

#ifdef __cplusplus
extern "C" {
#endif


#define MEMFAULT_LOG_DEBUG(fmt, ...)
#define MEMFAULT_LOG_INFO(fmt, ...)
#define MEMFAULT_LOG_WARN(fmt, ...)
#define MEMFAULT_LOG_ERROR(fmt, ...)


#ifdef __cplusplus
}
#endif
