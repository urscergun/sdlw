// sdlw::FileList - a directory browser built on ListView.
//
// Populates a ListView with the files and folders of a path (folders first,
// with a ".." entry to go up). Columns: Name, Size, Type. Double-click or Enter
// on a folder navigates into it; on a file, update() returns true (activated).
// Uses std::filesystem (standard library) — no extra dependency.
#pragma once

#include "sdlw/listview.h"
#include "sdlw/focus.h"

#include <cstdint>
#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class FileList : public Focusable {
public:
    struct Entry {
        std::string   name;
        bool          isDir = false;
        std::uintmax_t size = 0;
    };

    FileList() = default;
    FileList(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);

    // Load a directory. Returns false (and keeps the current listing) if the
    // path can't be read. Relative paths are resolved to absolute.
    bool setPath(const std::string& path);
    const std::string& path() const { return path_; }

    // The entry under the selection, or nullptr.
    const Entry* selectedEntry() const;

    ListView& list() { return list_; }
    ListView::Style& style() { return list_.style(); }

    // Handle selection/scroll/sort and navigation. Returns true on the frame a
    // *file* is activated (double-click or Enter); folder/".." activations
    // navigate and return false.
    bool update(Window& win, Font& font);
    void draw(SDL_Renderer* renderer, Font& font);

    // Focusable (delegates to the inner list).
    void focusRect(float& x, float& y, float& w, float& h) const override { list_.focusRect(x, y, w, h); }
    void setFocus(bool f, Window& win) override { list_.setFocus(f, win); }
    bool focused() const override { return list_.focused(); }

private:
    void applyColumns();
    void rebuildRows();

    ListView    list_;
    std::string path_;
    std::vector<Entry> entries_;   // insertion order (folders first)
    float w_ = 0;
};

} // namespace sdlw
