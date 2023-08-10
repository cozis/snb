#include "widget/table.h"
#include "widget/group.h"
#include "widget/button.h"
#include "widget/buff_view.h"
#include "widget/text_input.h"
#include "widget/split_view.h"

void initStyle(void);
void freeStyle(void);
void increaseFontSize(void);
void decreaseFontSize(void);
TextInput  *createStylizedTextInput(void);
Button     *createStylizedButton(const char *label, void *context, ButtonCallback callback);
GroupView  *createStylizedGroupView(void);
BufferView *createStylizedBufferView(void);
void         initStylizedTableView(TableView *table, void *context, TableFunctions iter_funcs, TableCallback callback);
void stylizedSplitView(SplitDirection dir, Widget *first, Widget *second);
