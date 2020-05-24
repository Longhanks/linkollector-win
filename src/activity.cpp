#include "activity.h"

namespace linkollector::win {

std::string activity_to_string(activity activity_) noexcept {
    switch (activity_) {
    case activity::url: {
        return "URL";
    }
    case activity::text: {
        return "TEXT";
    }
    }
    LINKOLLECTOR_UNREACHABLE;
}

std::wstring activity_to_wstring(activity activity_) noexcept {
    switch (activity_) {
    case activity::url: {
        return L"URL";
    }
    case activity::text: {
        return L"TEXT";
    }
    }
    LINKOLLECTOR_UNREACHABLE;
}

std::optional<activity>
activity_from_string(std::string_view activity_) noexcept {
    std::string lowercase_activity;
    std::transform(
        std::begin(activity_),
        std::end(activity_),
        std::back_inserter(lowercase_activity),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lowercase_activity == "url") {
        return activity::url;
    }
    if (lowercase_activity == "text") {
        return activity::text;
    }
    return std::nullopt;
}

std::optional<std::pair<activity, std::string>>
deserialize(gsl::span<std::byte> msg) noexcept {
    const auto delimiter_begin =
        std::search(std::begin(msg),
                    std::end(msg),
                    std::begin(activity_delimiter_bin),
                    std::end(activity_delimiter_bin));

    if (delimiter_begin == std::begin(msg) ||
        delimiter_begin == std::end(msg)) {
        return std::nullopt;
    }

    const auto delimiter_end = [&delimiter_begin]() {
        auto begin_ = delimiter_begin;
        std::advance(begin_, activity_delimiter.size());
        return begin_;
    }();

    if (std::distance(delimiter_end, std::end(msg)) == 0) {
        return std::nullopt;
    }

    const auto activity_string =
        std::string(static_cast<char *>(static_cast<void *>(msg.data())),
                    static_cast<std::size_t>(
                        std::distance(std::begin(msg), delimiter_begin)));

    auto maybe_activity = activity_from_string(activity_string);

    if (!maybe_activity.has_value()) {
        return std::nullopt;
    }

    auto activity_ = *maybe_activity;
    auto string_ = std::string(
        static_cast<char *>(static_cast<void *>(&(*delimiter_end))),
        static_cast<std::size_t>(std::distance(delimiter_end, std::end(msg))));

    return {std::make_pair(activity_, std::move(string_))};
}

} // namespace linkollector::win
