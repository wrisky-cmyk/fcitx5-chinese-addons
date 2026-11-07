/*
 * SPDX-FileCopyrightText: 2026 fcitx5-chinese-addons contributors
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "gbcharset.h"
#include <fcitx-utils/utf8.h>
#include "gb2312table.h"

namespace fcitx {

bool isGB2312Char(uint32_t codepoint) {
    // GB2312 hanzi all live in CJK Unified Ideographs block (U+4E00-U+9FFF).
    // CJK Extension A (U+3400-U+4DBF), Compatibility Ideographs (U+F900+) and
    // Extension B and beyond are all outside of GB2312.
    if (codepoint < 0x3400) {
        return true;
    }
    if (codepoint <= 0x4DBF) {
        return false;
    }
    if (codepoint < 0x4E00) {
        // Yijing hexagram symbols and other non-ideograph characters.
        return true;
    }
    return isGB2312Codepoint(codepoint);
}

bool isGB2312Word(std::string_view word) {
    uint32_t chr;
    for (auto iter = word.begin(), end = word.end(); iter != end;) {
        auto next = utf8::getNextChar(iter, end, &chr);
        if (next == iter) {
            // Invalid UTF-8, do not filter it.
            return true;
        }
        iter = next;
        if (!isGB2312Char(chr)) {
            return false;
        }
    }
    return true;
}

} // namespace fcitx
