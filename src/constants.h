#pragma once

#include <Windows.h>

namespace linkollector::win {

constexpr const int WINDOW_CLASS_NAME_MAX_SIZE = 256;

constexpr const int LINE_PADDING = 14;
constexpr const int CONTENT_PADDING = 20;

constexpr const int LINE_HEIGHT = 1;

constexpr const int WINDOW_WIDTH_96 = 640;
constexpr const int WINDOW_HEIGHT_96 = 480;

constexpr const int ACTING_DIALOG_WIDTH_96 = 312;
constexpr const int ACTING_DIALOG_HEIGHT_96 = 140;

constexpr const int PROGRESS_BAR_WIDTH_96 = 160;
constexpr const int PROGRESS_BAR_HEIGHT_96 = 15;
constexpr const int PROGRESS_BAR_ANIMATION_MS_96 = 30;

constexpr const int BUTTON_WIDTH_MINIMUM_96 = 75;
constexpr const int LABEL_TO_TEXT_FIELD_PADDING_96 = 8;
constexpr const int TEXT_FIELD_SINGLE_LINE_HEIGHT_MINIMUM_96 = 23;
constexpr const int TEXT_FIELD_WIDTH_MINIMUM_96 = 98;
constexpr const int GROUP_BOX_TOP_PADDING_96 = 16;
constexpr const int GROUP_BOX_LEFT_PADDING_96 = 9;
constexpr const int GROUP_BOX_BOTTOM_PADDING_96 = 9;
constexpr const int GROUP_BOX_BETWEEN_CHILDREN_PADDING_96 = 4;

constexpr const UINT_PTR ID_TEXT_FIELD_TO_DEVICE = 0;
constexpr const UINT_PTR ID_TEXT_FIELD_MESSAGE_CONTENT = 1;

} // namespace linkollector::win
