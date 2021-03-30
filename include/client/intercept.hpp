#ifndef GEKKOFS_INTERCEPT_HPP
#define GEKKOFS_INTERCEPT_HPP

namespace gkfs::preload {

int
internal_hook_guard_wrapper(long syscall_number, long arg0, long arg1,
                            long arg2, long arg3, long arg4, long arg5,
                            long* syscall_return_value);

int
hook_guard_wrapper(long syscall_number, long arg0, long arg1, long arg2,
                   long arg3, long arg4, long arg5, long* syscall_return_value);

void
start_self_interception();

void
start_interception();

void
stop_interception();

} // namespace gkfs::preload

#endif
