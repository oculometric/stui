#define STUI_IMPLEMENTATION
#define DEBUG
#include <stui_script.h>

using namespace stui;

int main()
{
    Terminal::configure("script test", 0.5f);

    LayoutReader reader;
    reader.readPage("demo.sls");

    return 0;
}