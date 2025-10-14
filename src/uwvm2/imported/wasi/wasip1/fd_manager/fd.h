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
# include <limits>
# include <type_traits>
# include <memory>
# include <new>
// macro
# include <uwvm2/utils/macro/push_macros.h>
// import
# include <fast_io.h>
# include <uwvm2/utils/container/impl.h>
# include <uwvm2/utils/mutex/impl.h>
# include <uwvm2/parser/wasm/standard/wasm1/type/impl.h>
# include <uwvm2/imported/wasi/wasip1/abi/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

/*NONEXPORT*/ namespace uwvm2::imported::wasi::wasip1::environment
{
    struct mount_dir_root_t;
}

UWVM_MODULE_EXPORT namespace uwvm2::imported::wasi::wasip1::fd_manager
{
#if defined(_WIN32)
    // on WinNt: Files and paths are both managed under handle management, while sockets are managed by ws3.
    // on Win9x: Files are managed by handles, paths are managed as strings, and sockets are managed by ws3.
    enum class win32_wasi_fd_typesize_t : unsigned
    {
        file,
        socket,
# if defined(_WIN32_WINDOWS)
        dir,
# endif
    };
#endif

    /// @brief    WASI file descriptor
    /// @details  Using a singleton ensures that when encountering multithreaded scaling during usage, the file descriptors currently in use remain unaffected.
    struct wasi_fd_t
    {
        using mutex_t = ::uwvm2::utils::mutex::mutex_t;
        using mutex_alloc_t = ::fast_io::native_typed_global_allocator<mutex_t>;

        // ====== for wasi ======
        // Wasi does not differentiate between text and binary modes, so on Win32 systems, files can be opened using CreateFile even if the Win32 layer does not
        // provide CRLF escaping.
        ::fast_io::native_file file_fd{};

#if defined(_WIN32) && !defined(__CYGWIN__)
        /// @note The global win32_wsa_service must be created in advance.
# if !(defined(UWVM_ALREADY_PROVIDED_WIN32_WSA_SERVICE) || defined(UWVM))
        inline static ::fast_io::win32_wsa_service wasi_fd_win32_wsa_service{};  // [global]
# endif
        // Win32 sockets are independent of files, so socket functionality must be provided separately.
        ::fast_io::win32_socket_file socket_fd{};
# if defined(_WIN32_WINDOWS)
        // The file system of Windows 9x is independent and requires string storage.
        ::fast_io::win32_9xa_dir_file dir_fd{};
# endif
        win32_wasi_fd_typesize_t file_type{};
#endif

        ::uwvm2::imported::wasi::wasip1::abi::rights_t rights_base{};
        ::uwvm2::imported::wasi::wasip1::abi::rights_t rights_inherit{};

        // ====== for vm ======
        mutex_t fd_mutex{};  // [singleton]

        // The "close pos" is only valid for those in the "close" list and invalid for those in the "renumber map".
        ::std::size_t close_pos{SIZE_MAX};

        ::uwvm2::utils::container::u8string preloaded_dir{};

        inline constexpr wasi_fd_t() noexcept = default;

        inline constexpr wasi_fd_t(wasi_fd_t const& other) noexcept = delete;

        inline constexpr wasi_fd_t& operator= (wasi_fd_t const& other) noexcept = delete;

        inline constexpr wasi_fd_t(wasi_fd_t&& other) noexcept = delete;

        inline constexpr wasi_fd_t& operator= (wasi_fd_t&& other) noexcept = delete;

        inline ~wasi_fd_t() = default;
    };

    struct wasi_fd_unique_ptr_t UWVM_TRIVIALLY_RELOCATABLE_IF_ELIGIBLE
    {
        using wasi_fd_t_fast_io_type_allocator = ::fast_io::native_typed_global_allocator<wasi_fd_t>;

        wasi_fd_t* fd_p{};

        inline constexpr wasi_fd_unique_ptr_t() noexcept
        {
            this->fd_p = wasi_fd_t_fast_io_type_allocator::allocate(1uz);
            ::new(this->fd_p) wasi_fd_t{};
        }

        inline constexpr wasi_fd_unique_ptr_t(wasi_fd_unique_ptr_t const&) noexcept = delete;
        inline constexpr wasi_fd_unique_ptr_t& operator= (wasi_fd_unique_ptr_t const&) noexcept = delete;

        inline constexpr wasi_fd_unique_ptr_t(wasi_fd_unique_ptr_t&& other) noexcept
        {
            this->fd_p = other.fd_p;
            other.fd_p = nullptr;
        }

        inline constexpr wasi_fd_unique_ptr_t& operator= (wasi_fd_unique_ptr_t&& other) noexcept
        {
            if(::std::addressof(other) == this) [[unlikely]] { return *this; }

            if(this->fd_p) [[likely]]
            {
                ::std::destroy_at(this->fd_p);
                wasi_fd_t_fast_io_type_allocator::deallocate_n(this->fd_p, 1uz);
            }

            this->fd_p = other.fd_p;
            other.fd_p = nullptr;

            return *this;
        }

        inline constexpr void clear_destroy() noexcept
        {
            if(this->fd_p) [[likely]]
            {
                ::std::destroy_at(this->fd_p);
                wasi_fd_t_fast_io_type_allocator::deallocate_n(this->fd_p, 1uz);

                this->fd_p = nullptr;
            }
        }

        inline constexpr ~wasi_fd_unique_ptr_t()
        {
            if(this->fd_p) [[likely]]
            {
                ::std::destroy_at(this->fd_p);
                wasi_fd_t_fast_io_type_allocator::deallocate_n(this->fd_p, 1uz);

                // Multiple calls to the destructor are ub, no need to set it to nullptr here
            }
        }
    };
}

/// @brief Define container optimization operations for use with fast_io
UWVM_MODULE_EXPORT namespace fast_io::freestanding
{
    template <>
    struct is_trivially_copyable_or_relocatable<::uwvm2::imported::wasi::wasip1::fd_manager::wasi_fd_unique_ptr_t>
    {
        inline static constexpr bool value = true;
    };
}

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
#endif
