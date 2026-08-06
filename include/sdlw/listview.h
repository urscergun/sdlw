// sdlw::ListView - a multi-column list (table) of rows, each with several cells.
//
// Columns have a title, width, and alignment. Rows are cell strings (one per
// column). A fixed header sits on top; the body scrolls vertically (wheel +
// scrollbar) and supports row selection and keyboard navigation. Focusable.
#pragma once

#include "sdlw/focus.h"
#include "sdlw/scrollbar.h"

#include <string>
#include <vector>

struct SDL_Renderer;

namespace sdlw {

class Window;
class Font;

class ListView : public Focusable {
public:
    enum class Align { Left, Right };
    enum class SortType { Alpha, Numeric };
    enum class SortDir { None, Ascending, Descending };
    struct Column {
        std::string title;
        float    width = 100;
        Align    align = Align::Left;
        SortType sort = SortType::Alpha; // how this column compares when sorted
        bool     sortable = true;        // clicking the header cycles the sort
    };

    struct Style {
        unsigned char bg[3]           = { 34,  34,  42 };
        unsigned char border[3]       = { 90,  90, 110 };
        unsigned char headerBg[3]     = { 48,  48,  60 };
        unsigned char headerText[3]   = { 210, 210, 220 };
        unsigned char item[3]         = { 220, 220, 228 };
        unsigned char hoverBg[3]      = { 52,  52,  64 };
        unsigned char selectedBg[3]   = { 52,  90, 150 };
        unsigned char selectedText[3] = { 245, 245, 250 };
        unsigned char gridline[3]     = { 60,  60,  74 };
        unsigned char scrollThumb[3]  = { 100, 100, 120 };
    };

    using Row = std::vector<std::string>;

    ListView() = default;
    ListView(float x, float y, float w, float h);

    void setRect(float x, float y, float w, float h);
    void setColumns(std::vector<Column> cols);
    void addColumn(std::string title, float width, Align align = Align::Left);
    void setRows(std::vector<Row> rows);
    void addRow(Row cells);
    void clear();
    void setShowHeader(bool on) { showHeader_ = on; }
    Style& style() { return style_; }

    int rowCount() const { return int(rows_.size()); }
    int selected() const { return selected_; }
    void setSelected(int index);
    const Row* selectedRow() const;
    const Row* rowAt(int displayIndex) const;    // row at a display position (post-sort)

    // Sorting. Clicking a sortable header cycles unordered -> ascending ->
    // descending -> unordered. You can also drive it programmatically.
    void sortBy(int column, SortDir dir);
    int     sortColumn() const { return sortCol_; }
    SortDir sortDir() const { return dir_; }

    // Selection/scroll/keyboard for this frame. Returns true if the selection
    // changed. rowActivated() is true on a double-click or Enter.
    bool update(Window& win, Font& font);
    bool rowActivated() const { return activated_; }

    void draw(SDL_Renderer* renderer, Font& font);

    // Focusable
    void focusRect(float& x, float& y, float& w, float& h) const override { x = x_; y = y_; w = w_; h = h_; }
    void setFocus(bool f, Window&) override { focused_ = f; }
    bool focused() const override { return focused_; }

private:
    int   rowHeight(Font& font) const;
    float headerHeight(Font& font) const;
    void  syncBar(Font& font);
    void  scrollToSelected(Font& font);
    float maxScroll(Font& font) const;
    float columnX(int i) const;             // left x of column i
    int   columnAtX(float mx) const;        // column under x, or -1
    void  rebuildOrder();                   // rebuild display order from current sort
    std::string cellStr(int row, int col) const;

    std::vector<Column> columns_;
    std::vector<Row>    rows_;               // data in insertion order
    std::vector<int>    order_;              // display order: indices into rows_
    float x_ = 0, y_ = 0, w_ = 0, h_ = 0;
    Style style_;
    int   selected_ = -1;                    // display index into order_
    int   hover_ = -1;
    float scroll_ = 0;
    bool  focused_ = false;
    bool  showHeader_ = true;
    bool  activated_ = false;
    int   rowPad_ = 6;
    int   sortCol_ = -1;                     // sorted column, or -1 (unordered)
    SortDir dir_ = SortDir::None;
    Scrollbar bar_;
};

} // namespace sdlw
