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
// macro
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_push_macro.h>
# include <uwvm2/utils/macro/push_macros.h>
// import
# include <fast_io.h>
# include <uwvm2/uwvm_predefine/utils/ansies/impl.h>
# include <uwvm2/uwvm_predefine/io/impl.h>
# include <uwvm2/object/memory/linear/impl.h>
# include <uwvm2/imported/wasi/wasip1/abi/impl.h>
# include <uwvm2/imported/wasi/wasip1/fd_manager/impl.h>
# include <uwvm2/imported/wasi/wasip1/memory/impl.h>
# include <uwvm2/imported/wasi/wasip1/environment/impl.h>
#endif

#ifndef UWVM_MODULE_EXPORT
# define UWVM_MODULE_EXPORT
#endif

UWVM_MODULE_EXPORT namespace uwvm2::imported::wasi::wasip1::func
{
    template <::std::integral I>
        requires (::std::signed_integral<I>)
    inline constexpr bool add_overflow(I a, I b, I & result) noexcept
    {
#if UWVM_HAS_BUILTIN(__builtin_add_overflow)
        return __builtin_add_overflow(a, b, ::std::addressof(result));
#else
        // The result of signed addition is undefined behavior.
        // But the results still have to be provided.

        if(b > 0 && a > ::std::numeric_limits<I>::max() - b)
        {
            // overflow
            // The behavior of __builtin_add_overflow is well-defined as overflow wrap-around, so an unsigned implementation is used here.
            using unsigned_I = ::std::make_unsigned_t<I>;
            result = static_cast<I>(static_cast<unsigned_I>(a) + static_cast<unsigned_I>(b));
            return true;
        }

        if(b < 0 && a < ::std::numeric_limits<I>::min() - b)
        {
            // underflow
            // The behavior of __builtin_add_overflow is well-defined as overflow wrap-around, so an unsigned implementation is used here.
            using unsigned_I = ::std::make_unsigned_t<I>;
            result = static_cast<I>(static_cast<unsigned_I>(a) + static_cast<unsigned_I>(b));
            return true;
        }

        // default, no overflow
        result = a + b;
        return false;
#endif
    }

    template <::std::integral I>
        requires (::std::unsigned_integral<I>)
    inline constexpr bool add_overflow(I a, I b, I & result) noexcept
    {
#if defined(_MSC_VER) && !defined(__clang__)
        if UWVM_IF_NOT_CONSTEVAL
        {
# if defined(_M_X64)
            if constexpr(::std::same_as<I, ::std::uint64_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u64(false, a, b, ::std::addressof(result));
            }
            else if constexpr(::std::same_as<I, ::std::uint32_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u32(false, a, b, ::std::addressof(result));
            }
            else if constexpr(::std::same_as<I, ::std::uint16_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u16(false, a, b, ::std::addressof(result));
            }
            else if constexpr(::std::same_as<I, ::std::uint8_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u8(false, a, b, ::std::addressof(result));
            }
            else
            {
                // msvc's `_addcarry_u32` does not specialize for `unsigned long`
                result = static_cast<I>(a + b);
                return result < a;
            }

# elif defined(_M_X32)
            if constexpr(::std::same_as<I, ::std::uint32_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u32(false, a, b, ::std::addressof(result));
            }
            else if constexpr(::std::same_as<I, ::std::uint16_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u16(false, a, b, ::std::addressof(result));
            }
            else if constexpr(::std::same_as<I, ::std::uint8_t>)
            {
                // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
                return ::fast_io::intrinsics::msvc::x86::_addcarry_u8(false, a, b, ::std::addressof(result));
            }
            else
            {
                result = static_cast<I>(a + b);
                return result < a;
            }

# else  // ARM, ARM64, ARM64ec
            result = static_cast<I>(a + b);
            return result < a;
# endif
        }
        else
        {
            result = static_cast<I>(a + b);
            return result < a;
        }

#elif UWVM_HAS_BUILTIN(__builtin_add_overflow)
        // Parameters can't be filled with ::std::addressof(i), to ensure determinism of exceptions.
        return __builtin_add_overflow(a, b, ::std::addressof(result));

#else
        result = static_cast<I>(a + b);
        return result < a;
#endif
    }

    struct fast_io_io_scatter_t_allocator_guard
    {
        using allocator = ::fast_io::native_typed_global_allocator<::fast_io::io_scatter_t>;
        ::fast_io::io_scatter_t* ptr{};

        inline constexpr fast_io_io_scatter_t_allocator_guard() noexcept = default;

        inline constexpr fast_io_io_scatter_t_allocator_guard(::fast_io::io_scatter_t* o_ptr) noexcept : ptr{o_ptr} {}

        fast_io_io_scatter_t_allocator_guard(fast_io_io_scatter_t_allocator_guard const&) = delete;
        fast_io_io_scatter_t_allocator_guard& operator= (fast_io_io_scatter_t_allocator_guard const&) = delete;

        fast_io_io_scatter_t_allocator_guard(fast_io_io_scatter_t_allocator_guard&& other) = delete;
        fast_io_io_scatter_t_allocator_guard& operator= (fast_io_io_scatter_t_allocator_guard&& other) = delete;

        inline constexpr ~fast_io_io_scatter_t_allocator_guard()
        {
            // The deallocator performs internal null pointer checks.
            allocator::deallocate(ptr);
        }
    };

    // The path series functions of wasi cannot traverse paths beyond the specified path, not the preopen path.

    enum class dir_type_e : ::std::uint_fast8_t
    {
        curr,
        prev,
        next,
    };

    struct split_path_t
    {
        ::uwvm2::utils::container::u8string next_name{};
        dir_type_e dir_type{};
    };

    struct split_path_res_t
    {
        ::uwvm2::utils::container::vector<split_path_t> res{};
        bool is_absolute{};
    };

    inline constexpr split_path_res_t split_path(::uwvm2::utils::container::u8string_view path) noexcept
    {
        split_path_res_t result{};

        // Indicates whether the path is absolute; the leading ‘/’ will be skipped during actual splitting.
        result.is_absolute = !path.empty() && path.front_unchecked() == u8'/';

        // Prevent extreme cases from exceeding boundaries (almost impossible to hit).
        if(path.size() == ::std::numeric_limits<::std::size_t>::max()) [[unlikely]] { return {}; }

        ::std::size_t i{};
        while(i < path.size())
        {
            // Skip consecutive '/'
            while(i < path.size() && path.index_unchecked(i) == u8'/') { ++i; }
            if(i >= path.size()) { break; }

            ::std::size_t j{i};
            while(j < path.size() && path.index_unchecked(j) != u8'/') { ++j; }
            ::uwvm2::utils::container::u8string_view token{path.subview_unchecked(i, j - i)};

            // Precisely identify '.' and '..', otherwise treat as a normal name field.
            if(token == u8".") { result.res.emplace_back(::uwvm2::utils::container::u8string{}, dir_type_e::curr); }
            else if(token == u8"..") { result.res.emplace_back(::uwvm2::utils::container::u8string{}, dir_type_e::prev); }
            else
            {
                result.res.emplace_back(::uwvm2::utils::container::u8string{token}, dir_type_e::next);
            }

            i = j + 1uz;
        }

        return result;
    }
}  // namespace uwvm2::imported::wasi::wasip1::func

#ifndef UWVM_MODULE
// macro
# include <uwvm2/utils/macro/pop_macros.h>
# include <uwvm2/uwvm_predefine/utils/ansies/uwvm_color_pop_macro.h>
#endif
