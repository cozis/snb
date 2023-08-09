#include <string.h>
#include "table.h"
#include "../utils/basic.h"

static void handleEvent(Widget *widget, Event event);
static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area);
static void free_(Widget *widget);

bool initTableView(TableView *table, WidgetStyle *base_style, TableStyle *style, void *context, TableFunctions funcs, TableCallback callback)
{
    initWidget(&table->base, base_style, draw, free_, handleEvent);
    table->context = context;
    table->style = style;
    table->funcs = funcs;
    table->callback = callback;
    table->num_rows = 0;
    table->num_columns = 0;
    table->loaded_font = GetFontDefault();
    table->loaded_font_file = NULL;
    table->loaded_font_size = 24;
    return true;
}

static void reloadFont(TableView *table)
{
    const char *font_file = table->style->font_file;
    float       font_size = table->style->font_size;

    Font font;
    if (table->style->font_file)
        font = LoadFontEx(font_file, font_size, NULL, 250);
    else
        font = GetFontDefault();

    UnloadFont(table->loaded_font); // FIXME: Is it okay to unload the default font?
    table->loaded_font = font;
    table->loaded_font_file = font_file;
    table->loaded_font_size = font_size;
}

static void reloadStyleIfChanged(TableView *table)
{
    if (table->style) {
        bool changed_font_file = (table->style->font_file != table->loaded_font_file);
        bool changed_font_size = (table->style->font_size != table->loaded_font_size);
        if (changed_font_file || changed_font_size)
            reloadFont(table);
    }
}

void setColumnLabel(TableView *table, int index, const char *label)
{
    if (index > table->num_columns)
        return; // Index out of range

    if (index == table->num_columns && index == MAX_TABLE_COLUMNS)
        return; // Can't add more columns

    char  *dst = table->column_labels[index];
    size_t max = MAX_TABLE_COLUMN_LABEL;

    strncpy(dst, label, max);
    dst[max-1] = '\0';

    if (index == table->num_columns) {
        table->column_width[index] = 0;
        table->num_columns++;
    }
}

void tableViewChanged(TableView *table)
{
    table->active = -1;
    setScrollX((Widget*) table, 0);
    setScrollY((Widget*) table, 0);
}

static void handleEvent(Widget *widget, Event event)
{
    TableView *table = (TableView*) widget;
    switch (event.type) {
        case EVENT_MOUSE_LEFT_DOWN:
        {
            float   pad_v = table->style->pad_v;
            float entry_h = 2 * pad_v + table->style->entry_h;
            int entry = (event.mouse.y / entry_h) - 1;
            if (entry >= 0 && entry < table->num_rows) {
                if (table->callback)
                    table->callback(table->context, entry);
                table->active = entry;
                setMouseFocus(widget);
            }
        }
        break;

        case EVENT_MOUSE_LEFT_UP:
        table->active = -1;
        setMouseFocus(NULL);
        break;

        default:
        break;
    }
}

static void startIteration(TableView *table)
{
    table->funcs.start(table->context);
}

static void stopIteration(TableView *table)
{
    table->funcs.end(table->context);
}

static bool nextIteration(TableView *table)
{
    return table->funcs.next(table->context);
}

static void getField(TableView *table, int index, char *dst, size_t max)
{
    table->funcs.field(table->context, index, dst, max);
}

static Vector2 draw(Widget *widget, Vector2 offset, Vector2 area)
{
    TableView *table = (TableView*) widget;
    reloadStyleIfChanged(table);

    float pad_h = table->style->pad_h;
    float pad_v = table->style->pad_v;
    float entry_h = 2 * pad_v + table->style->entry_h;
    float entry_y = offset.y;

    Font  font       = table->loaded_font;
    float font_size  = table->loaded_font_size;

    float current_table_w = 0;
    for (int i = 0; i < table->num_columns; i++)
        current_table_w += table->column_width[i];
    current_table_w = MAX(area.x, current_table_w);

    float column_width[MAX_TABLE_COLUMNS];
    for (int i = 0; i < table->num_columns; i++)
        column_width[i] = 0;

    bool first = true;
    int num_rows = 0;
    startIteration(table);
    do {

        float entry_x = offset.x;
        float entry_w = 0;

        Color font_color;
            
        if (!first && num_rows == table->active) {
            font_color = table->style->font_active;
            DrawRectangle(entry_x, entry_y, current_table_w, entry_h, table->style->background_active);
        } else {
            font_color = table->style->font_color;
        }

        float cell_x = entry_x;
        float cell_y = entry_y;
        for (int i = 0; i < table->num_columns; i++) {

            char buffer[1024];
            char *label;
            if (first)
                label = table->column_labels[i];
            else {
                getField(table, i, buffer, sizeof(buffer));
                label = buffer;
            }

            float spacing = 0;
            Vector2 text_area = MeasureTextEx(font, label, font_size, spacing);
            
            Vector2 position = {
                .x = cell_x + pad_h,
                .y = cell_y + (entry_h - text_area.y) / 2,
            };
            
            DrawTextEx(font, label, position, font_size, spacing, font_color);
            cell_x += table->column_width[i];

            float cell_w = text_area.x + 2 * pad_h;
            column_width[i] = MAX(column_width[i], cell_w);
            entry_w += cell_w;
        }

        if (!first) {
            Vector2 begin, end;
            begin.x = entry_x;
            begin.y = entry_y;
            end.x = entry_x + current_table_w;
            end.y = entry_y;
            DrawLineV(begin, end, GRAY);
        }

        if (!first)
            num_rows++;
        first = false;
        entry_y += entry_h;
    } while (nextIteration(table));
    stopIteration(table);

    table->num_rows = num_rows;
    if (num_rows <= table->active)
        table->active = -1;

    Vector2 logic_area = {
        .x = 0, // To be calculated
        .y = entry_y - offset.y,
    };
    for (int i = 0; i < table->num_columns; i++)
        logic_area.x += column_width[i];

    Vector2 column = offset;
    for (int i = 0; i < table->num_columns-1; i++) {
        
        column.x += table->column_width[i];

        Vector2 begin = column;
        Vector2 end   = column;
        
        end.y += MAX(logic_area.y, area.y);

        DrawLineV(begin, end, GRAY);
    }

    for (int i = 0; i < table->num_columns; i++)
        table->column_width[i] = column_width[i];

    return logic_area;
}

static void free_(Widget *widget)
{
    TableView *table = (TableView*) widget;
    (void) table;
}
