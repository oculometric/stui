#define STUI_IMPLEMENTATION
#define DEBUG
#include <stui_script.h>

using namespace stui;

int main()
{
    Terminal::configure("script test", 0.5f);

    LayoutReader reader;
    Page* pg = reader.readPage("demo.sls");
    if (pg) pg->render();

    return 0;
}