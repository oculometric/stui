#define STUI_IMPLEMENTATION
#define DEBUG
#include <stui_script.h>

using namespace stui;

int main()
{
    Terminal::configure("script test", 0.5f);

    LayoutReader reader;
    Page* pg = reader.readPageFromFile("demo.sls");
    if (!pg) return 1;

    pg->get<ProgressBar>("progress_bar")->fraction = 0.99f;
    pg->render();

    Terminal::unConfigure(false);
    cout << endl;

    return 0;
}