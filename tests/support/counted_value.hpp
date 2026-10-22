#pragma once

#include <format>

namespace test_utils
{
    struct counted_value
    {
    };

    inline int g_format_count = 0;
} // namespace test_utils

template <> struct std::formatter<test_utils::counted_value>
{
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    auto format(const test_utils::counted_value &, std::format_context &ctx) const
    {
        ++test_utils::g_format_count;
        return std::format_to(ctx.out(), "counted");
    }
};
