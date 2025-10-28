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
#include <cstring>

#include <fast_io.h>

#include <uwvm2/imported/wasi/wasip1/func/path_create_directory.h>

int main()
{
    using ::uwvm2::imported::wasi::wasip1::abi::errno_t;
    using ::uwvm2::imported::wasi::wasip1::abi::rights_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_posix_fd_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_size_t;
    using ::uwvm2::imported::wasi::wasip1::abi::wasi_void_ptr_t;
    using ::uwvm2::imported::wasi::wasip1::environment::wasip1_environment;
    using ::uwvm2::object::memory::linear::native_memory_t;

    native_memory_t memory{};
    memory.init_by_page_count(2uz);

    wasip1_environment<native_memory_t> env{.wasip1_memory = ::std::addressof(memory),
                                            .argv = {},
                                            .envs = {},
                                            .fd_storage = {},
                                            .mount_dir_roots = {},
                                            .trace_wasip1_call = false};

    // Prepare FD table
    env.fd_storage.opens.resize(10uz);

    // Case 0: negative fd -> ebadf
    {
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(-1),
                                                                                      static_cast<wasi_void_ptr_t>(0u),
                                                                                      static_cast<wasi_size_t>(0u));
        if(ret != errno_t::ebadf)
        {
            ::fast_io::io::perrln("error: pcd32 Case 0 expected ebadf");
            ::fast_io::fast_terminate();
        }
    }

    // Common: set up a directory fd at index 3 with '.'
    auto& fd_ok = *env.fd_storage.opens.index_unchecked(3uz).fd_p;
    fd_ok.rights_base = static_cast<rights_t>(-1);
    fd_ok.rights_inherit = static_cast<rights_t>(-1);
    fd_ok.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::dir);
    {
        auto& ds = fd_ok.wasi_fd.ptr->wasi_fd_storage.storage.dir_stack;
        ::uwvm2::imported::wasi::wasip1::fd_manager::dir_stack_entry_ref_t entry{};
        entry.ptr->dir_stack.file = ::fast_io::dir_file{u8"."};
        ds.dir_stack.push_back(::std::move(entry));
    }

    // Case 1: rights missing -> enotcapable
    {
        auto& fd1 = *env.fd_storage.opens.index_unchecked(4uz).fd_p;
        fd1.rights_base = static_cast<rights_t>(0);
        fd1.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::dir);
        {
            auto& ds = fd1.wasi_fd.ptr->wasi_fd_storage.storage.dir_stack;
            ::uwvm2::imported::wasi::wasip1::fd_manager::dir_stack_entry_ref_t entry{};
            entry.ptr->dir_stack.file = ::fast_io::dir_file{u8"."};
            ds.dir_stack.push_back(::std::move(entry));
        }

        constexpr wasi_void_ptr_t p{4096u};
        constexpr auto s = u8"uwvm_ut_pcd32_no_rights";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"uwvm_ut_pcd32_no_rights") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(4),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"uwvm_ut_pcd32_no_rights") - 1u));
        if(ret != errno_t::enotcapable)
        {
            ::fast_io::io::perrln("error: pcd32 Case 1 expected enotcapable");
            ::fast_io::fast_terminate();
        }
    }

    // Case 2: fd is file -> enotdir
    {
        auto& fd2 = *env.fd_storage.opens.index_unchecked(5uz).fd_p;
        fd2.rights_base = static_cast<rights_t>(-1);
        fd2.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::file);

        constexpr wasi_void_ptr_t p{8192u};
        constexpr auto s = u8"uwvm_ut_pcd32_notdir";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"uwvm_ut_pcd32_notdir") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(5),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"uwvm_ut_pcd32_notdir") - 1u));
        if(ret != errno_t::enotdir)
        {
            ::fast_io::io::perrln("error: pcd32 Case 2 expected enotdir");
            ::fast_io::fast_terminate();
        }
    }

    // Case 3: absolute path -> eperm
    {
        auto& fd3 = *env.fd_storage.opens.index_unchecked(6uz).fd_p;
        fd3.rights_base = static_cast<rights_t>(-1);
        fd3.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::dir);
        {
            auto& ds = fd3.wasi_fd.ptr->wasi_fd_storage.storage.dir_stack;
            ::uwvm2::imported::wasi::wasip1::fd_manager::dir_stack_entry_ref_t entry{};
            entry.ptr->dir_stack.file = ::fast_io::dir_file{u8"."};
            ds.dir_stack.push_back(::std::move(entry));
        }

        constexpr wasi_void_ptr_t p{12288u};
        constexpr auto s = u8"/abs32_dir";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"/abs32_dir") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(6),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"/abs32_dir") - 1u));
        if(ret != errno_t::eperm)
        {
            ::fast_io::io::perrln("error: pcd32 Case 3 expected eperm");
            ::fast_io::fast_terminate();
        }
    }

    // Case 4: '.' last component -> eexist
    {
        auto& fd4 = *env.fd_storage.opens.index_unchecked(7uz).fd_p;
        fd4.rights_base = static_cast<rights_t>(-1);
        fd4.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::dir);
        {
            auto& ds = fd4.wasi_fd.ptr->wasi_fd_storage.storage.dir_stack;
            ::uwvm2::imported::wasi::wasip1::fd_manager::dir_stack_entry_ref_t entry{};
            entry.ptr->dir_stack.file = ::fast_io::dir_file{u8"."};
            ds.dir_stack.push_back(::std::move(entry));
        }

        constexpr wasi_void_ptr_t p{16384u};
        constexpr auto s = u8".";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8".") - 1u);

        auto const ret =
            ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env, static_cast<wasi_posix_fd_t>(7), p, static_cast<wasi_size_t>(sizeof(u8".") - 1u));
        if(ret != errno_t::eexist)
        {
            ::fast_io::io::perrln("error: pcd32 Case 4 expected eexist");
            ::fast_io::fast_terminate();
        }
    }

    // Case 5: empty -> einval
    {
        auto& fd5 = *env.fd_storage.opens.index_unchecked(8uz).fd_p;
        fd5.rights_base = static_cast<rights_t>(-1);
        fd5.wasi_fd.ptr->wasi_fd_storage.reset_type(::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_type_e::dir);
        {
            auto& ds = fd5.wasi_fd.ptr->wasi_fd_storage.storage.dir_stack;
            ::uwvm2::imported::wasi::wasip1::fd_manager::dir_stack_entry_ref_t entry{};
            entry.ptr->dir_stack.file = ::fast_io::dir_file{u8"."};
            ds.dir_stack.push_back(::std::move(entry));
        }

        constexpr wasi_void_ptr_t p{18432u};
        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env, static_cast<wasi_posix_fd_t>(8), p, static_cast<wasi_size_t>(0u));
        if(ret != errno_t::einval)
        {
            ::fast_io::io::perrln("error: pcd32 Case 5 expected einval");
            ::fast_io::fast_terminate();
        }
    }

    // Case 6: success create and cleanup
    {
        // cleanup (ignore failures)
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"uwvm_ut_pcd32_ok", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{20480u};
        constexpr auto s = u8"uwvm_ut_pcd32_ok";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"uwvm_ut_pcd32_ok") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"uwvm_ut_pcd32_ok") - 1u));
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln("error: pcd32 Case 6 expected esuccess");
            ::fast_io::fast_terminate();
        }

        // cleanup (ignore failures)
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"uwvm_ut_pcd32_ok", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 7: a/b/. -> eexist when a/b exists
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_dot_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_dot_a/b");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{22528u};
        constexpr auto s = u8"pcd32_dot_a/b/.";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_dot_a/b/.") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_dot_a/b/.") - 1u));
        if(ret != errno_t::eexist)
        {
            ::fast_io::io::perrln("error: pcd32 Case 7 expected eexist");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_dot_a/b", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_dot_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 8: a/x/. where x not exist -> enoent
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_dot2_a");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{24576u};
        constexpr auto s = u8"pcd32_dot2_a/x/.";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_dot2_a/x/.") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_dot2_a/x/.") - 1u));
        if(ret != errno_t::enoent)
        {
            ::fast_io::io::perrln("error: pcd32 Case 8 expected enoent");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_dot2_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 9: a/f/. where f is file -> enotdir
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_dot3_a");
        }
        catch(::fast_io::error)
        {
        }
        {
            ::fast_io::native_file f{u8"pcd32_dot3_a/f", ::fast_io::open_mode::out};
        }

        constexpr wasi_void_ptr_t p{26624u};
        constexpr auto s = u8"pcd32_dot3_a/f/.";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_dot3_a/f/.") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_dot3_a/f/.") - 1u));
        if(ret != errno_t::enotdir)
        {
            ::fast_io::io::perrln("error: pcd32 Case 9 expected enotdir");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_dot3_a/f", {});
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_dot3_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 10: a/b/../c -> success when a,b exist and c not exist
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp_a/b");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{28672u};
        constexpr auto s = u8"pcd32_pp_a/b/../c";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_pp_a/b/../c") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_pp_a/b/../c") - 1u));
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln("error: pcd32 Case 10 expected esuccess");
            ::fast_io::fast_terminate();
        }

        // verify created
        try
        {
            ::fast_io::dir_file df{u8"pcd32_pp_a/c"};
        }
        catch(::fast_io::error)
        {
            ::fast_io::io::perrln("error: pcd32 Case 10 verify created failed");
            ::fast_io::fast_terminate();
        }

        // cleanup
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp_a/c", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp_a/b", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 11: a/b/../c -> enoent when b missing
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp2_a");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{30720u};
        constexpr auto s = u8"pcd32_pp2_a/b/../c";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_pp2_a/b/../c") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_pp2_a/b/../c") - 1u));
        if(ret != errno_t::enoent)
        {
            ::fast_io::io::perrln("error: pcd32 Case 11 expected enoent");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp2_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 12: a/b/../c -> enotdir when b is file
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp3_a");
        }
        catch(::fast_io::error)
        {
        }
        {
            ::fast_io::native_file f{u8"pcd32_pp3_a/b", ::fast_io::open_mode::out};
        }

        constexpr wasi_void_ptr_t p{32768u};
        constexpr auto s = u8"pcd32_pp3_a/b/../c";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_pp3_a/b/../c") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_pp3_a/b/../c") - 1u));
        if(ret != errno_t::enotdir)
        {
            ::fast_io::io::perrln("error: pcd32 Case 12 expected enotdir");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp3_a/b", {});
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp3_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 13: a/b/../c -> eexist when c already exists
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp4_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp4_a/b");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_pp4_a/c");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{34816u};
        constexpr auto s = u8"pcd32_pp4_a/b/../c";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_pp4_a/b/../c") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_pp4_a/b/../c") - 1u));
        if(ret != errno_t::eexist)
        {
            ::fast_io::io::perrln("error: pcd32 Case 13 expected eexist");
            ::fast_io::fast_terminate();
        }

        // cleanup
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp4_a/c", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp4_a/b", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_pp4_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 14: symlink intermediate -> create under target directory
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_sym_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_symlinkat(u8"pcd32_sym_a", ::fast_io::at_fdcwd(), u8"pcd32_link_to_a");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{36864u};
        constexpr auto s = u8"pcd32_link_to_a/c";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_link_to_a/c") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_link_to_a/c") - 1u));
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln("error: pcd32 Case 14 expected esuccess");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::dir_file df{u8"pcd32_sym_a/c"};
        }
        catch(::fast_io::error)
        {
            ::fast_io::io::perrln("error: pcd32 Case 14 verify created failed");
            ::fast_io::fast_terminate();
        }

        // cleanup
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_sym_a/c", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_link_to_a", {});
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_sym_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 15: symlink loop -> eloop
    {
        try
        {
            ::fast_io::native_symlinkat(u8"pcd32_loop", ::fast_io::at_fdcwd(), u8"pcd32_loop");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{38912u};
        constexpr auto s = u8"pcd32_loop/x";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_loop/x") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_loop/x") - 1u));
        if(ret != errno_t::eloop)
        {
            ::fast_io::io::perrln("error: pcd32 Case 15 expected eloop");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_loop", {});
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 16: symlink escapes root -> eperm
    {
        try
        {
            ::fast_io::native_symlinkat(u8"..", ::fast_io::at_fdcwd(), u8"pcd32_up");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{40960u};
        constexpr auto s = u8"pcd32_up/x";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_up/x") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_up/x") - 1u));
        if(ret != errno_t::eperm)
        {
            ::fast_io::io::perrln("error: pcd32 Case 16 expected eperm");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_up", {});
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 17: creating existing directory without trailing '.' -> eexist
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_ex_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_ex_a/b");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{43008u};
        constexpr auto s = u8"pcd32_ex_a/b";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_ex_a/b") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_ex_a/b") - 1u));
        if(ret != errno_t::eexist)
        {
            ::fast_io::io::perrln("error: pcd32 Case 17 expected eexist");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_ex_a/b", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_ex_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 18: symlink to absolute path (e.g., /etc) -> eperm
    {
        try
        {
            ::fast_io::native_symlinkat(u8"/etc", ::fast_io::at_fdcwd(), u8"pcd32_abs");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{45056u};
        constexpr auto s = u8"pcd32_abs/hack";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_abs/hack") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_abs/hack") - 1u));
        if(ret != errno_t::eperm)
        {
            ::fast_io::io::perrln("error: pcd32 Case 18 expected eperm");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_abs", {});
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 19: two-node symlink loop A<->B -> eloop
    {
        try
        {
            ::fast_io::native_symlinkat(u8"pcd32_loop2_b", ::fast_io::at_fdcwd(), u8"pcd32_loop2_a");
            ::fast_io::native_symlinkat(u8"pcd32_loop2_a", ::fast_io::at_fdcwd(), u8"pcd32_loop2_b");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{47104u};
        constexpr auto s = u8"pcd32_loop2_a/x";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_loop2_a/x") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_loop2_a/x") - 1u));
        if(ret != errno_t::eloop)
        {
            ::fast_io::io::perrln("error: pcd32 Case 19 expected eloop");
            ::fast_io::fast_terminate();
        }

        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_loop2_a", {});
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_loop2_b", {});
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 20: symlink to ".." inside subdir -> esuccess (pcd32_escape_a/up/x -> x)
    {
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"x", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_escape_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_symlinkat(u8"..", ::fast_io::at_fdcwd(), u8"pcd32_escape_a/up");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{49152u};
        constexpr auto s = u8"pcd32_escape_a/up/x";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_escape_a/up/x") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_escape_a/up/x") - 1u));
        if(ret != errno_t::esuccess)
        {
            ::fast_io::io::perrln("error: pcd32 Case 20 expected esuccess");
            ::fast_io::fast_terminate();
        }

        // verify created at CWD: "x"
        try
        {
            ::fast_io::dir_file df{u8"x"};
        }
        catch(::fast_io::error)
        {
            ::fast_io::io::perrln("error: pcd32 Case 20 verify created failed");
            ::fast_io::fast_terminate();
        }

        // cleanup created "x"
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"x", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_escape_a/up", {});
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_escape_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 21: permission boundary on parent dir -> eacces/eperm
    {
        try
        {
            ::fast_io::native_mkdirat(::fast_io::at_fdcwd(), u8"pcd32_ro_a");
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_fchmodat(::fast_io::at_fdcwd(), u8"pcd32_ro_a", static_cast<::fast_io::perms>(0555));
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{51200u};
        constexpr auto s = u8"pcd32_ro_a/no";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_ro_a/no") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_ro_a/no") - 1u));
        if(!(ret == errno_t::eacces || ret == errno_t::eperm))
        {
            ::fast_io::io::perrln("error: pcd32 Case 21 expected eacces/eperm");
            ::fast_io::fast_terminate();
        }

        // restore and cleanup
        try
        {
            ::fast_io::native_fchmodat(::fast_io::at_fdcwd(), u8"pcd32_ro_a", static_cast<::fast_io::perms>(0777));
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_ro_a", ::fast_io::native_at_flags::removedir);
        }
        catch(::fast_io::error)
        {
        }
    }

    // Case 22: symlink to a file as intermediate -> enotdir
    {
        try
        {
            ::fast_io::native_file f{u8"pcd32_file", ::fast_io::open_mode::out};
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_symlinkat(u8"pcd32_file", ::fast_io::at_fdcwd(), u8"pcd32_link_to_file");
        }
        catch(::fast_io::error)
        {
        }

        constexpr wasi_void_ptr_t p{53248u};
        constexpr auto s = u8"pcd32_link_to_file/x";
        ::uwvm2::imported::wasi::wasip1::memory::write_all_to_memory_wasm32(memory,
                                                                            p,
                                                                            reinterpret_cast<::std::byte const*>(s),
                                                                            reinterpret_cast<::std::byte const*>(s) + sizeof(u8"pcd32_link_to_file/x") - 1u);

        auto const ret = ::uwvm2::imported::wasi::wasip1::func::path_create_directory(env,
                                                                                      static_cast<wasi_posix_fd_t>(3),
                                                                                      p,
                                                                                      static_cast<wasi_size_t>(sizeof(u8"pcd32_link_to_file/x") - 1u));
        if(ret != errno_t::enotdir)
        {
            ::fast_io::io::perrln("error: pcd32 Case 22 expected enotdir");
            ::fast_io::fast_terminate();
        }

        // cleanup
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_link_to_file", {});
        }
        catch(::fast_io::error)
        {
        }
        try
        {
            ::fast_io::native_unlinkat(::fast_io::at_fdcwd(), u8"pcd32_file", {});
        }
        catch(::fast_io::error)
        {
        }
    }

    return 0;
}

