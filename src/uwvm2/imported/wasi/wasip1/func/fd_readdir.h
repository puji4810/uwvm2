

/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      MacroModel
 * @version     2.0.0
 * @copyright   APL-2.0 License
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

#pragma once

#ifndef UWVM_MODULE
// std
# include <cstddef>
# include <cstdint>
# include <climits>
# include <cstring>
# include <limits>
# include <concepts>
# include <bit>
# include <memory>
# include <type_traits>
// macro
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
# include <uwvm2/utils/macro/push_macros.h>
// platform
# if !UWVM_HAS_BUILTIN(__builtin_alloca) && (defined(_WIN32) && !defined(__WINE__) && !defined(__BIONIC__) && !defined(__CYGWIN__))
#  include <malloc.h>
# elif !UWVM_HAS_BUILTIN(__builtin_alloca)
#  include <alloca.h>
# endif
# if !defined(_WIN32)
#  include <errno.h>
#  include <unistd.h>
#  include <sys/stat.h>
# endif
// import
# include <fast_io.h>
# include <uwvm2/uwvm_predefine/utils/ansies/impl.h>
# include <uwvm2/uwvm_predefine/io/impl.h>
# include <uwvm2/utils/mutex/impl.h>
# include <uwvm2/utils/debug/impl.h>
# include <uwvm2/object/memory/linear/impl.h>
# include <uwvm2/imported/wasi/wasip1/abi/impl.h>
# include <uwvm2/imported/wasi/wasip1/fd_manager/impl.h>
# include <uwvm2/imported/wasi/wasip1/memory/impl.h>
# include <uwvm2/imported/wasi/wasip1/environment/impl.h>
# include "base.h"
# include "posix.h"
#endif

#ifndef UWVM_CPP_EXCEPTIONS
# warning "Without enabling C++ exceptions, using this WASI function may cause termination."
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::imported::wasi::wasip1::func
{
    /// @brief     WasiPreview1.fd_readdir
    /// @details   __wasi_errno_t __wasi_fd_readdir(__wasi_fd fd, void *buf, __wasi_size_t buf_len, __wasi_dircookie cookie, __wasi_size_t *buf_used);
    ::uwvm2::imported::wasi::wasip1::abi::errno_t fd_readdir(
        ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment<::uwvm2::object::memory::linear::native_memory_t> & env,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_t fd,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_t buf,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_size_t buf_len,
        ::uwvm2::imported::wasi::wasip1::abi::dircookie_t cookie,
        ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_t buf_used) noexcept
    {
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
        if(env.wasip1_memory == nullptr) [[unlikely]]
        {
            // Security issues inherent to virtual machines
            ::uwvm2::utils::debug::trap_and_inform_bug_pos();
        }
#endif
        auto& memory{*env.wasip1_memory};

        auto const trace_wasip1_call{env.trace_wasip1_call};

        if(trace_wasip1_call) [[unlikely]]
        {
#ifdef UWVM
            ::fast_io::io::perr(::uwvm2::uwvm::io::u8log_output,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL_AND_SET_WHITE),
                                u8"uwvm: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                u8"[info]  ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"wasip1: ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_YELLOW),
                                u8"fd_readdir",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8"(",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                fd,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                ::fast_io::mnp::addrvw(buf),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                buf_len,
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(cookie)>>>(cookie),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8", ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_LT_GREEN),
                                ::fast_io::mnp::addrvw(buf_used),
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_WHITE),
                                u8") ",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_ORANGE),
                                u8"(wasi-trace)\n",
                                ::fast_io::mnp::cond(::uwvm2::uwvm::utils::ansies::put_color, UWVM_COLOR_U8_RST_ALL));
#else
            ::fast_io::io::perr(::fast_io::u8err(),
                                u8"uwvm: [info]  wasip1: fd_readdir(",
                                fd,
                                u8", ",
                                ::fast_io::mnp::addrvw(buf),
                                u8", ",
                                buf_len,
                                u8", ",
                                static_cast<::std::underlying_type_t<::std::remove_cvref_t<decltype(cookie)>>>(cookie),
                                u8", ",
                                ::fast_io::mnp::addrvw(buf_used),
                                u8") (wasi-trace)\n");
#endif
        }

        // The negative value fd is invalid, and this check prevents subsequent undefined behavior.
        if(fd < 0) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_t::ebadf; }

        auto& wasm_fd_storage{env.fd_storage};

        // The pointer to `wasm_fd` is fixed and remains unchanged even when the vector within `fd_manager` is resized.
        ::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_t* curr_wasi_fd_t_p;  // no initialize

        // Subsequent operations involving the file descriptor require locking. curr_fd_release_guard release when return.
        ::uwvm2::utils::mutex::mutex_merely_release_guard_t curr_fd_release_guard{};

        {
            // Prevent operations to obtain the size or perform resizing at this time.
            // Only a lock is required when acquiring the unique pointer for the file descriptor. The lock can be released once the acquisition is complete.
            // Since the file descriptor's location is fixed and accessed via the unique pointer,

            // Simply acquiring data using a shared_lock
            ::uwvm2::utils::mutex::rw_shared_guard_t fds_lock{wasm_fd_storage.fds_rwlock};

            // Negative states have been excluded, so the conversion result will only be positive numbers.
            using unsigned_fd_t = ::std::make_unsigned_t<::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_t>;
            auto const unsigned_fd{static_cast<unsigned_fd_t>(fd)};

            // On platforms where `size_t` is smaller than the `fd` type, this check must be added.
            constexpr auto size_t_max{::std::numeric_limits<::std::size_t>::max()};
            if constexpr(::std::numeric_limits<unsigned_fd_t>::max() > size_t_max)
            {
                if(unsigned_fd > size_t_max) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_t::ebadf; }
            }

            auto const fd_opens_pos{static_cast<::std::size_t>(unsigned_fd)};

            // The minimum value in rename_map is greater than opensize.
            if(wasm_fd_storage.opens.size() <= fd_opens_pos)
            {
                // Possibly within the tree being renumbered
                if(auto const renumber_map_iter{wasm_fd_storage.renumber_map.find(fd)}; renumber_map_iter != wasm_fd_storage.renumber_map.end())
                {
                    curr_wasi_fd_t_p = renumber_map_iter->second.fd_p;
                }
                else [[unlikely]]
                {
                    return ::uwvm2::imported::wasi::wasip1::abi::errno_t::ebadf;
                }
            }
            else
            {
                // The addition here is safe.
                curr_wasi_fd_t_p = wasm_fd_storage.opens.index_unchecked(fd_opens_pos).fd_p;
            }

            // curr_wasi_fd_t_p never nullptr
#if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
            if(curr_wasi_fd_t_p == nullptr) [[unlikely]]
            {
                // Security issues inherent to virtual machines
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
            }
#endif

            // Other threads will definitely lock fds_rwlock when performing close operations (since they need to access the fd vector). If the current thread
            // is performing, no other thread can be executing any close operations simultaneously, eliminating any destruction issues. Therefore,
            // acquiring the lock at this point is safe. However, the problem arises when, immediately after acquiring the lock and before releasing the manager
            // lock and beginning fd operations, another thread executes a deletion that removes this fd. Subsequent operations by the current thread would then
            // encounter issues. Thus, locking must occur before releasing fds_rwlock.
            curr_fd_release_guard.device_p = ::std::addressof(curr_wasi_fd_t_p->fd_mutex);
            curr_fd_release_guard.lock();

            // After unlocking fds_lock, members within `wasm_fd_storage_t` can no longer be accessed or modified.
        }

        // curr_fd_uniptr is not null.
        auto& curr_fd{*curr_wasi_fd_t_p};

        // If obtained from the renumber map, it will always be the correct value. If obtained from the open vec, it requires checking whether it is closed.
        // Therefore, a unified check is implemented.
        if(curr_fd.close_pos != SIZE_MAX) [[unlikely]] { return ::uwvm2::imported::wasi::wasip1::abi::errno_t::ebadf; }

        // Rights check: readdir uses readdir permission
        if((curr_fd.rights_base & ::uwvm2::imported::wasi::wasip1::abi::rights_t::right_fd_readdir) !=
           ::uwvm2::imported::wasi::wasip1::abi::rights_t::right_fd_readdir) [[unlikely]]
        {
            return ::uwvm2::imported::wasi::wasip1::abi::errno_t::enotcapable;
        }

        using underlying_dircookie_t = ::std::make_unsigned_t<::uwvm2::imported::wasi::wasip1::abi::dircookie_t>;
        auto const underlying_dircookie{static_cast<underlying_dircookie_t>(cookie)};
        underlying_dircookie_t dircookie_counter{};

#if defined(_WIN32) && !defined(__CYGWIN__)
        // win32
        switch(curr_fd.file_type)
        {
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::socket:
            {
                return ::uwvm2::imported::wasi::wasip1::abi::errno_t::enotdir;
            }
# if defined(_WIN32_WINDOWS)
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::dir:
            {
                for(auto const& ent: current(at(::fast_io::posix_io_observer{curr_fd.dir_fd})))
                {
                    auto const d_next{static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_t>(dircookie_counter)};

                    if(dircookie_counter++ < underlying_dircookie) [[likely]] { continue; }

                    ::uwvm::vm::wasi::inode_t const d_ino{details::get_inode(ent)};

                    ::fast_io::u8cstring_view const fn{u8filename(ent)};

                    ::uwvm::vm::wasi::dirnamlen_t const d_namlen{static_cast<::uwvm::vm::wasi::dirnamlen_t>(fn.size())};

                    ::uwvm::vm::wasi::filetype_t d_type{};
                }
            }
# endif
            case ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::file:
            {
# if defined(_WIN32_WINDOWS)
                // Win9x distinguishes between directories and files; here it directly returns an error.
                return ::uwvm2::imported::wasi::wasip1::abi::errno_t::enotdir;
# else

                break;
# endif
            }
            [[unlikely]] default:
            {
# if (defined(_DEBUG) || defined(DEBUG)) && defined(UWVM_ENABLE_DETAILED_DEBUG_CHECK)
                // Security issues inherent to virtual machines
                ::uwvm2::utils::debug::trap_and_inform_bug_pos();
# endif
                return ::uwvm2::imported::wasi::wasip1::abi::errno_t::eio;
            }
        }
#else
        // posix
#endif
    }
}  // namespace uwvm2::imported::wasi::wasip1::func

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_pop_macro.h>
#endif

