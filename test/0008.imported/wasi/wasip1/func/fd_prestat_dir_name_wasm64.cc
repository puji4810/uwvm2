/*************************************************************
 * Ultimate WebAssembly Virtual Machine (Version 2)          *
 * Copyright (c) 2025-present UlteSoft. All rights reserved. *
 * Licensed under the APL-2.0 License (see LICENSE file).    *
 *************************************************************/

/**
 * @author      GPT-5
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

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <fast_io.h>

#include <uwvm2/imported/wasi/wasip1/func/fd_prestat_dir_name_wasm64.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_size_wasm64_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(1uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory), .argv = {}, .envs = {}, .fd_storage = {}, .mount_dir_roots={}, .trace_wasip1_call = false};

    // Prepare FD table
    env.fd_storage.opens.resize(8uz);

    // Case 0: negative fd → ebadf
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_prestat_dir_name_wasm64(env,
                                                                                           static_cast<wasi_posix_fd_wasm64_t>(-1),
                                                                                           static_cast<wasi_void_ptr_wasm64_t>(0u),
                                                                                           static_cast<wasi_size_wasm64_t>(0u));
        if(ret != errno_wasm64_t::ebadf) { ::fast_io::fast_terminate(); }
    }

    // Set fd=3 as preopened directory with name "/sandbox"
    auto &fde = *env.fd_storage.opens.index_unchecked(3uz).fd_p;
    fde.preloaded_dir = ::uwvm2::utils::container::u8string_view{u8"/sandbox"};

    // Case 1: buffer too small → enobufs
    {
        constexpr wasi_void_ptr_wasm64_t buf{4096u};
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_prestat_dir_name_wasm64(env,
                                                                                           static_cast<wasi_posix_fd_wasm64_t>(3),
                                                                                           buf,
                                                                                           static_cast<wasi_size_wasm64_t>(3u));
        if(ret != errno_wasm64_t::enobufs) { ::fast_io::fast_terminate(); }
    }

    // Case 2: success and exact bytes copied, no NUL
    {
        constexpr wasi_void_ptr_wasm64_t buf{8192u};
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_prestat_dir_name_wasm64(env,
                                                                                           static_cast<wasi_posix_fd_wasm64_t>(3),
                                                                                           buf,
                                                                                           static_cast<wasi_size_wasm64_t>(8u));
        if(ret != errno_wasm64_t::esuccess) { ::fast_io::fast_terminate(); }

        if(std::memcmp(memory.memory_begin + static_cast<std::size_t>(buf), "/sandbox", 8uz) != 0) { ::fast_io::fast_terminate(); }
    }

    // Case 3: not preopened dir → enotdir
    {
        auto &fdx = *env.fd_storage.opens.index_unchecked(4uz).fd_p;
        fdx.preloaded_dir = {};
        constexpr wasi_void_ptr_wasm64_t buf{12288u};
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::fd_prestat_dir_name_wasm64(env,
                                                                                           static_cast<wasi_posix_fd_wasm64_t>(4),
                                                                                           buf,
                                                                                           static_cast<wasi_size_wasm64_t>(1u));
        if(ret != errno_wasm64_t::enotdir) { ::fast_io::fast_terminate(); }
    }

    return 0;
}








