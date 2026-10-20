// Throwaway CI probe for std::chrono time zone support
#include <chrono>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    struct stage_result
    {
        bool ok;
        std::string detail;
    };

    void run_stage(const std::string &name, const std::function<stage_result()> &stage)
    {
        try
        {
            const auto result = stage();
            std::cout << "PROBE " << name << (result.ok ? "=PASS " : "=FAIL ") << result.detail << '\n';
        }
        catch (const std::exception &e)
        {
            std::cout << "PROBE " << name << "=FAIL exception: " << e.what() << '\n';
        }
        catch (...)
        {
            std::cout << "PROBE " << name << "=FAIL unknown exception\n";
        }
    }

    std::optional<std::string> format_time(const std::string &format)
    {
        const auto split = format.find('%');
        std::string literal = format.substr(0, split);
        if (split == std::string::npos) return literal;

        const std::string spec = format.substr(split);
        if (spec.find_first_of("{}") != std::string::npos) return std::nullopt;

        const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        const std::string fmt = "{:" + spec + "}";
        auto render = [&](const auto &time) { return literal + std::vformat(fmt, std::make_format_args(time)); };

        try
        {
            return render(std::chrono::zoned_time{std::chrono::current_zone(), now});
        }
        catch (const std::format_error &)
        {
            return std::nullopt;
        }
        catch (const std::runtime_error &)
        {
            // no time zone database, fall back to UTC below
        }

        try
        {
            return render(now);
        }
        catch (const std::format_error &)
        {
            return std::nullopt;
        }
    }

    stage_result version_info()
    {
        std::string detail;
#ifdef __cpp_lib_chrono
        detail += std::format("__cpp_lib_chrono={} ", __cpp_lib_chrono);
#endif
#ifdef _GLIBCXX_RELEASE
        detail += std::format("_GLIBCXX_RELEASE={} ", _GLIBCXX_RELEASE);
#endif
#ifdef _LIBCPP_VERSION
        detail += std::format("_LIBCPP_VERSION={} ", _LIBCPP_VERSION);
#endif
#ifdef _MSC_VER
        detail += std::format("_MSC_VER={} ", _MSC_VER);
#endif
#ifdef __MINGW32__
        detail += "__MINGW32__ ";
#endif
        return {true, detail};
    }

    stage_result current_zone_name()
    {
        const auto *zone = std::chrono::current_zone();
        if (zone == nullptr) return {false, "current_zone() is null"};
        return {true, std::string(zone->name())};
    }

    stage_result tzdb_loads()
    {
        const auto *utc = std::chrono::locate_zone("UTC");
        return {utc != nullptr, std::format("UTC={} version={}", utc != nullptr ? utc->name() : "null",
                                            std::chrono::get_tzdb().version)};
    }

    stage_result zoned_time_constructs()
    {
        const std::chrono::zoned_time zt{std::chrono::current_zone(),
                                         std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())};
        return {true, std::format("{}", zt.get_sys_time().time_since_epoch().count())};
    }

    stage_result zoned_time_formats()
    {
        const std::chrono::zoned_time zt{std::chrono::current_zone(),
                                         std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())};
        const auto text = std::vformat("{:%d/%m/%Y__%H:%M:%S}", std::make_format_args(zt));
        return {text.size() == 20, std::format("\"{}\" size={}", text, text.size())};
    }

    stage_result utc_fallback_formats()
    {
        const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        const auto text = std::vformat("{:%Y-%m-%d %H:%M:%S %Z}", std::make_format_args(now));
        return {text.ends_with(" UTC"), std::format("\"{}\"", text)};
    }

    stage_result format_time_table()
    {
        const std::vector<std::string> accept{"at %H:%M", "bogus", "100%% at %H", "%H %Z", "%%"};
        const std::vector<std::string> reject{"%Q", "%H{}", "%H}}", "%"};
        bool ok = true;
        std::string detail;
        for (const auto &format : accept)
        {
            const auto text = format_time(format);
            ok = ok && text.has_value();
            detail += std::format("accept \"{}\" -> {} ", format, text ? "\"" + *text + "\"" : "nullopt(WRONG)");
        }
        for (const auto &format : reject)
        {
            const auto text = format_time(format);
            ok = ok && !text.has_value();
            detail += std::format("reject \"{}\" -> {} ", format, text ? "\"" + *text + "\"(WRONG)" : "nullopt");
        }
        const auto empty = format_time("");
        detail += std::format("info \"\" -> {}", empty ? "\"" + *empty + "\"" : "nullopt");
        return {ok, detail};
    }
} // namespace

int main()
{
    run_stage("1_version_info", version_info);
    run_stage("2_current_zone", current_zone_name);
    run_stage("3_tzdb_loads", tzdb_loads);
    run_stage("4_zoned_time", zoned_time_constructs);
    run_stage("5_zoned_time_format", zoned_time_formats);
    run_stage("6_utc_fallback_format", utc_fallback_formats);
    run_stage("7_format_time", format_time_table);
    return 0;
}
