/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      GPT-5
 * @version     2.0.0
 */

/****************************************
 *  _   _ __        ____     __ __  __  *
 * | | | |\ \      / /\ \   / /|  \/  | *
 * | | | | \ \ /\ / /  \ \ / / | |\/| | *
 * | |_| |  \ V  V /    \ V /  | |  | | *
 *  \___/    \_/\_/      \_/   |_|  |_| *
 *                                      *
 ****************************************/

#include <cstddef>
#include <cstdint>

#include <fast_io.h>

#include <uwvm2/imported/wasi/wasip1/func/fd_readdir_wasm64.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::rights_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(2uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory), .argv = {}, .envs = {}, .fd_storage = {}, .mount_dir_roots={}, .trace_wasip1_call = false};

    env.fd_storage.opens.resize(8uz);

    // Case 0: negative fd → ebadf
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_readdir_wasm64(env,
                                                                                   static_cast<wasi_posix_fd_wasm64_t>(-1),
                                                                                   static_cast<wasi_void_ptr_wasm64_t>(0u),
                                                                                   static_cast<wasi_size_wasm64_t>(0u),
                                                                                   static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>(0u),
                                                                                   static_cast<wasi_void_ptr_wasm64_t>(0u));
        if(ret != errno_wasm64_t::ebadf) { ::fast_io::fast_terminate(); }
    }

    // Case 1: directory enumeration writes entries and reports buf_used
    {
        auto &fde = *env.fd_storage.opens.index_unchecked(4uz).fd_p;
        fde.rights_base = static_cast<rights_wasm64_t>(-1);
        fde.rights_inherit = static_cast<rights_wasm64_t>(-1);
#if defined(_WIN32) && defined(_WIN32_WINDOWS)
        fde.file_type = ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::dir;
        fde.dir_fd = ::fast_io::win32_9xa_dir_file{u8"."};
#else
        fde.file_fd = ::fast_io::native_file{u8".", ::fast_io::open_mode::in | ::fast_io::open_mode::directory};
#endif

        constexpr wasi_void_ptr_wasm64_t buf_ptr{8192u};
        constexpr wasi_void_ptr_wasm64_t used_ptr{4096u};

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_readdir_wasm64(env,
                                                                                   static_cast<wasi_posix_fd_wasm64_t>(4),
                                                                                   buf_ptr,
                                                                                   static_cast<wasi_size_wasm64_t>(2048u),
                                                                                   static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>(0u),
                                                                                   used_ptr);
        if(ret != errno_wasm64_t::esuccess) { ::fast_io::fast_terminate(); }

        auto const used = ::uwvm2::imported::wasi::wasip1::memory::get_basic_wasm_type_from_memory_wasm64<wasi_size_wasm64_t>(memory, used_ptr);
        if(used > static_cast<wasi_size_wasm64_t>(2048u)) { ::fast_io::fast_terminate(); }
    }

    // Case 2: rights check enotcapable
    {
        auto &fde = *env.fd_storage.opens.index_unchecked(5uz).fd_p;
        // rights default 0
#if defined(_WIN32) && defined(_WIN32_WINDOWS)
        fde.file_type = ::uwvm2::imported::wasi::wasip1::fd_manager::win32_wasi_fd_typesize_t::dir;
        fde.dir_fd = ::fast_io::win32_9xa_dir_file{u8"."};
#else
        fde.file_fd = ::fast_io::native_file{u8".", ::fast_io::open_mode::in | ::fast_io::open_mode::directory};
#endif

        constexpr wasi_void_ptr_wasm64_t buf_ptr{16384u};
        constexpr wasi_void_ptr_wasm64_t used_ptr{12288u};

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_readdir_wasm64(env,
                                                                                   static_cast<wasi_posix_fd_wasm64_t>(5),
                                                                                   buf_ptr,
                                                                                   static_cast<wasi_size_wasm64_t>(1024u),
                                                                                   static_cast<::uwvm2::imported::wasi::wasip1::abi::dircookie_wasm64_t>(0u),
                                                                                   used_ptr);
        if(ret != errno_wasm64_t::enotcapable) { ::fast_io::fast_terminate(); }
    }

    return 0;
}


