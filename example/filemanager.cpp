// Example sdlw application: a two-pane file manager.
//
// Two FileList panes side by side. Focus a pane (click or Tab), pick an entry,
// then Copy/Move it to the *other* pane's folder, or Delete it. Double-click a
// folder (or "..") to navigate. Built with the layout engine, so it reflows on
// resize. There is no WinMain/main here; sdlw provides the entry point.
#include "sdlw/window.h"
#include "sdlw/font.h"
#include "sdlw/label.h"
#include "sdlw/button.h"
#include "sdlw/filelist.h"
#include "sdlw/focus.h"
#include "sdlw/layout.h"

#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

#define SDLW_DECL_FONT(sz)                                     \
    extern "C" {                                               \
        extern const unsigned char dejavusans_##sz##_fnt[];    \
        extern const unsigned int  dejavusans_##sz##_fnt_len;  \
        extern const unsigned char dejavusans_##sz##_bmp[];    \
        extern const unsigned int  dejavusans_##sz##_bmp_len;  \
    }
SDLW_DECL_FONT(16)
SDLW_DECL_FONT(24)

using namespace sdlw;

// Copy a file or directory (recursively), overwriting the destination.
static bool copyEntry(const fs::path& from, const fs::path& to, bool isDir, std::error_code& ec) {
    if (isDir) fs::copy(from, to, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
    else       fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
    return !ec;
}

int Main(int argc, char** argv) {
    (void)argc; (void)argv;

    Window win({ .title = "sdlw file manager", .width = 900, .height = 600 });
    if (!win.ok()) { std::fprintf(stderr, "window: %s\n", win.error()); return 1; }

    Font heading, ui;
    if (!heading.loadFromMemory(win.renderer(), dejavusans_24_fnt, dejavusans_24_fnt_len,
                                dejavusans_24_bmp, dejavusans_24_bmp_len) ||
        !ui.loadFromMemory(win.renderer(), dejavusans_16_fnt, dejavusans_16_fnt_len,
                           dejavusans_16_bmp, dejavusans_16_bmp_len)) {
        std::fprintf(stderr, "font: %s\n", ui.error());
        return 1;
    }

    std::error_code ec;
    std::string start = fs::current_path(ec).string();

    Label title("sdlw file manager", 0, 0);
    Label leftPath("", 0, 0), rightPath("", 0, 0);
    FileList left, right;
    left.setPath(start);
    right.setPath(start);

    Button copyBtn("Copy (F5)", 0, 0, 0, 0);
    Button moveBtn("Move (F6)", 0, 0, 0, 0);
    Button delBtn("Delete (F8)", 0, 0, 0, 0);
    Label status("Point at a pane (it highlights), pick an item. Copy F5, Move F6, Delete F8/Del, Up Backspace.", 0, 0);
    status.style().color[0] = 180; status.style().color[1] = 190; status.style().color[2] = 200;

    // Layout tree.
    VBox leftCol({ .spacing = 4 });
    leftCol.add(leftPath, Size::fixed(20));
    leftCol.add(left, Size::flex(1));
    VBox rightCol({ .spacing = 4 });
    rightCol.add(rightPath, Size::fixed(20));
    rightCol.add(right, Size::flex(1));

    HBox panes({ .spacing = 12 });
    panes.add(leftCol, Size::flex(1));
    panes.add(rightCol, Size::flex(1));

    HBox buttons({ .spacing = 8 });
    buttons.add(copyBtn, Size::fixed(140));
    buttons.add(moveBtn, Size::fixed(140));
    buttons.add(delBtn, Size::fixed(110));
    buttons.addSpacer(Size::flex());

    VBox root({ .padding = 12, .spacing = 8 });
    root.add(title, Size::fixed(30));
    root.add(panes, Size::flex(1));
    root.add(buttons, Size::fixed(34));
    root.add(status, Size::fixed(22));

    FocusManager focus;
    focus.add(&left); focus.add(&right);
    //focus.add(&copyBtn); focus.add(&moveBtn); focus.add(&delBtn);
    focus.setFocus(&left, win);

    FileList* active = &left;   // source pane (last-focused of the two)

    auto refresh = [](FileList& f) { f.setPath(f.path()); };

    while (win.pumpEvents()) {
        root.arrange({ 0, 0, float(win.width()), float(win.height()) });

        focus.update(win);
        left.update(win, ui);
        right.update(win, ui);

        // The "active" (source) pane is the one under the mouse; if the cursor
        // isn't over a pane (e.g. it's on a button), fall back to the focused
        // pane, else keep the last one. All operations use this single pane, so
        // Copy/Move/Delete/Backspace are always consistent.
        auto under = [&](FileList& f) {
            float x, y, w, h; f.focusRect(x, y, w, h);
            return win.mouseX() >= x && win.mouseX() < x + w &&
                   win.mouseY() >= y && win.mouseY() < y + h;
        };
        if (under(left))       active = &left;
        else if (under(right)) active = &right;
        else if (focus.focused() == &left)  active = &left;
        else if (focus.focused() == &right) active = &right;
        FileList* other = (active == &left) ? &right : &left;

        // Update the buttons (so they track hover/press), then add key triggers.
        bool doCopy   = copyBtn.update(win);
        bool doMove   = moveBtn.update(win);
        bool doDelete = delBtn.update(win);
        if (win.keyPressed(Key::F5)) doCopy = true;
        if (win.keyPressed(Key::F6)) doMove = true;
        if (win.keyPressed(Key::F8) || win.keyPressed(Key::Delete)) doDelete = true;
        bool doUp = win.keyPressed(Key::Backspace);

        if (doCopy) {
            const FileList::Entry* e = active->selectedEntry();
            if (!e || e->name == "..") status.setText("Select a file or folder first.");
            else {
                std::error_code e1;
                copyEntry(fs::path(active->path()) / e->name, fs::path(other->path()) / e->name, e->isDir, e1);
                if (e1) status.setText("Copy failed: " + e1.message());
                else { status.setText("Copied " + e->name); refresh(*other); }
            }
        }
        if (doMove) {
            const FileList::Entry* e = active->selectedEntry();
            if (!e || e->name == "..") status.setText("Select a file or folder first.");
            else {
                fs::path from = fs::path(active->path()) / e->name;
                fs::path to   = fs::path(other->path()) / e->name;
                std::error_code e1;
                fs::rename(from, to, e1);
                if (e1) {                       // cross-device: copy then delete
                    e1.clear();
                    if (copyEntry(from, to, e->isDir, e1)) { std::error_code e2; fs::remove_all(from, e2); }
                }
                if (e1) status.setText("Move failed: " + e1.message());
                else { status.setText("Moved " + e->name); refresh(*active); refresh(*other); }
            }
        }
        if (doDelete) {
            const FileList::Entry* e = active->selectedEntry();
            if (!e || e->name == "..") status.setText("Select a file or folder first.");
            else {
                std::error_code e1;
                fs::remove_all(fs::path(active->path()) / e->name, e1);
                if (e1) status.setText("Delete failed: " + e1.message());
                else { status.setText("Deleted " + e->name); refresh(*active); }
            }
        }
        if (doUp) {   // Backspace: go up one directory in the active pane.
            fs::path p = fs::path(active->path());
            fs::path parent = p.parent_path();
            if (!parent.empty() && parent != p) active->setPath(parent.string());
        }

        leftPath.setText(left.path());
        rightPath.setText(right.path());
        // Highlight the active pane's path label.
        auto tint = [&](Label& l, bool on) {
            l.style().color[0] = on ? 120 : 190; l.style().color[1] = on ? 200 : 190; l.style().color[2] = on ? 255 : 200;
        };
        tint(leftPath, active == &left);
        tint(rightPath, active == &right);

        win.clear(24, 24, 32);
        title.style().color[0] = 120; title.style().color[1] = 200; title.style().color[2] = 255;
        title.draw(win.renderer(), heading);
        leftPath.draw(win.renderer(), ui);
        rightPath.draw(win.renderer(), ui);
        left.draw(win.renderer(), ui);
        right.draw(win.renderer(), ui);
        copyBtn.draw(win.renderer(), ui);
        moveBtn.draw(win.renderer(), ui);
        delBtn.draw(win.renderer(), ui);
        status.draw(win.renderer(), ui);
        focus.drawFocusRing(win.renderer());
        win.present();
    }
    return 0;
}
