#include <dendro/cli/clipboard.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace dendro::cli {

bool copy_to_clipboard(const std::string& text) {
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    EmptyClipboard();
    const int wide_size = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(wide_size) * sizeof(wchar_t));
    if (memory == nullptr) {
        CloseClipboard();
        return false;
    }
    auto* wide_text = static_cast<wchar_t*>(GlobalLock(memory));
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide_text, wide_size);
    GlobalUnlock(memory);
    SetClipboardData(CF_UNICODETEXT, memory);
    CloseClipboard();
    return true;
#else
    (void)text;
    return false;
#endif
}

} // namespace dendro::cli
