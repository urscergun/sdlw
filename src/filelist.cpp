#include "sdlw/filelist.h"
#include "sdlw/font.h"

#include "text_util.h" // detail::toLower for case-insensitive sort

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace sdlw {

namespace {
std::string humanSize(std::uintmax_t b) {
    const char* unit[] = { "B", "KB", "MB", "GB", "TB" };
    double s = double(b);
    int i = 0;
    while (s >= 1024.0 && i < 4) { s /= 1024.0; ++i; }
    char buf[32];
    if (i == 0) std::snprintf(buf, sizeof buf, "%ju B", static_cast<std::uintmax_t>(b));
    else        std::snprintf(buf, sizeof buf, "%.1f %s", s, unit[i]);
    return buf;
}
std::string typeLabel(const std::string& name, bool isDir) {
    if (isDir) return name == ".." ? std::string() : std::string("Folder");
    std::string ext = fs::path(name).extension().string(); // ".cpp"
    return ext.empty() ? std::string("File") : ext.substr(1);
}
} // namespace

FileList::FileList(float x, float y, float w, float h) { setRect(x, y, w, h); }

void FileList::setRect(float x, float y, float w, float h) {
    list_.setRect(x, y, w, h);
    w_ = w;
    applyColumns();
}

void FileList::applyColumns() {
    const float sizeW = 82, typeW = 82;
    float nameW = std::max(120.0f, w_ - sizeW - typeW - 14);
    list_.setColumns({ { "Name", nameW, ListView::Align::Left },
                       { "Size", sizeW, ListView::Align::Right },
                       { "Type", typeW, ListView::Align::Left } });
}

bool FileList::setPath(const std::string& path) {
    std::error_code ec;
    fs::path dir = fs::weakly_canonical(fs::path(path), ec);
    if (ec) { dir = fs::absolute(fs::path(path), ec); }
    if (ec || !fs::is_directory(dir, ec)) return false;

    std::vector<Entry> es;
    if (dir.has_parent_path() && dir.parent_path() != dir)
        es.push_back({ "..", true, 0 });

    for (fs::directory_iterator it(dir, fs::directory_options::skip_permission_denied, ec), end;
         !ec && it != end; it.increment(ec)) {
        const fs::directory_entry& de = *it;
        std::error_code e2;
        Entry entry;
        entry.name = de.path().filename().string();
        entry.isDir = de.is_directory(e2);
        entry.size = (!entry.isDir && de.is_regular_file(e2)) ? de.file_size(e2) : 0;
        es.push_back(std::move(entry));
    }

    // ".." first, then folders, then files; alphabetical (case-insensitive).
    auto rank = [](const Entry& e) { return e.name == ".." ? 0 : (e.isDir ? 1 : 2); };
    std::sort(es.begin(), es.end(), [&](const Entry& a, const Entry& b) {
        int ra = rank(a), rb = rank(b);
        if (ra != rb) return ra < rb;
        return detail::toLower(a.name) < detail::toLower(b.name);
    });

    entries_ = std::move(es);
    path_ = dir.string();
    rebuildRows();
    return true;
}

void FileList::rebuildRows() {
    std::vector<ListView::Row> rows;
    rows.reserve(entries_.size());
    for (const Entry& e : entries_) {
        rows.push_back({ e.name,
                         e.isDir ? std::string() : humanSize(e.size),
                         typeLabel(e.name, e.isDir) });
    }
    list_.setRows(std::move(rows));
    list_.setSelected(entries_.empty() ? -1 : 0);
}

const FileList::Entry* FileList::selectedEntry() const {
    const ListView::Row* r = list_.selectedRow();
    if (!r || r->empty()) return nullptr;
    const std::string& name = (*r)[0];
    for (const Entry& e : entries_) if (e.name == name) return &e;
    return nullptr;
}

bool FileList::update(Window& win, Font& font) {
    list_.update(win, font);
    if (!list_.rowActivated()) return false;

    const Entry* e = selectedEntry();
    if (!e) return false;
    if (e->name == "..") { setPath(fs::path(path_).parent_path().string()); return false; }
    if (e->isDir)        { setPath((fs::path(path_) / e->name).string()); return false; }
    return true;   // a file was activated
}

void FileList::draw(SDL_Renderer* renderer, Font& font) { list_.draw(renderer, font); }

} // namespace sdlw
