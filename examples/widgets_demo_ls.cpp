#define STUI_IMPLEMENTATION
#define DEBUG
#include <stui.h>
#include <stui_script.h>

int main(int argc, char** argv)
{
    stui::Terminal::configure();

    stui::LayoutReader reader;
    stui::Page* page = reader.readPageFromFile("examples/layoutscripts/widgets_demo.sls");

    stui::Label* t1 = page->get<stui::Label>("t1");
    stui::Spinner* s1 = page->get<stui::Spinner>("s1");
    stui::Spinner* s2 = page->get<stui::Spinner>("s2");
    stui::Spinner* s3 = page->get<stui::Spinner>("s3");
    stui::Spinner* s4 = page->get<stui::Spinner>("s4");
    stui::ProgressBar* pb = page->get<stui::ProgressBar>("pb");
    stui::TextInputBox* tib = page->get<stui::TextInputBox>("tib");

    uint8_t image[12 * 12] =
    {
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x99, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0xcc, 0x99, 0x66, 0x66, 0x66, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0x99, 0x66, 0x66, 0x66, 0x33, 0x00, 0x33, 0xcc, 0xcc, 0xcc,
        0xcc, 0x99, 0x66, 0x66, 0x66, 0x33, 0x33, 0x00, 0x33, 0x66, 0x99, 0xcc,
        0xcc, 0x99, 0x66, 0x66, 0x99, 0x99, 0x99, 0x66, 0x33, 0x33, 0x66, 0xcc,
        0xcc, 0x66, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x66, 0x33, 0x66, 0xcc,
        0xcc, 0xcc, 0xcc, 0x99, 0x99, 0xcc, 0x99, 0x99, 0x99, 0x99, 0x99, 0xcc,
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc
    };
    stui::ImageView* image_view = page->get<stui::ImageView>("image_view");
    image_view->grayscale_image = image;
    image_view->image_size = stui::Coordinate{ 12, 12 };

    string demo_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec viverra nisi non metus feugiat, ut tempus massa sodales. Quisque efficitur finibus nibh, vel vulputate purus facilisis ac. Duis pharetra orci ac tincidunt suscipit. Nam hendrerit fringilla orci, ac commodo leo viverra vel. Donec vel vestibulum quam, in consectetur libero. Nunc pulvinar ligula et diam mollis porta. Etiam ac lobortis justo. Nunc scelerisque velit quis lectus dictum, et fermentum dui congue. Praesent semper luctus nisi ac tincidunt. Proin semper turpis vel quam mattis ultricies. Aenean varius quis neque eget feugiat. Etiam est odio, auctor eu enim a, convallis viverra leo. Suspendisse potenti. Nunc sit amet tellus sit amet magna imperdiet dictum id ut nisl.\n\nEtiam maximus pharetra elementum. Proin a tempus ante. Aenean ut arcu eu tellus gravida laoreet et rhoncus dolor. Nullam elementum, ante vel gravida congue, tellus dui vehicula lectus, a posuere leo nisi nec leo. Integer vel lobortis sem. Maecenas luctus semper magna non vehicula. Cras maximus lorem urna, ut malesuada enim varius eu. Fusce tristique tincidunt eros, at semper nunc tincidunt ut. Phasellus sagittis lectus ac pretium mattis. Vestibulum dictum elementum pellentesque. In eget eros nunc.Etiam nisl metus, feugiat a purus et, viverra pellentesque ante. Phasellus malesuada cursus risus ac semper. Suspendisse molestie purus ac augue aliquam, sit amet dapibus odio auctor. Donec vitae odio elit. Phasellus et ligula ac nunc dictum suscipit non pretium turpis.";
    page->get<stui::TextArea>("ta")->text = demo_text;
    
    vector<string> args;
    args.push_back("heres the first of a list");
    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);
    args.push_back("and heres the last");
    args.push_back("item a");
    args.push_back("item b");
    args.push_back("item c");
    args.push_back("item d");
    args.push_back("item e");
    args.push_back("item f");
    args.push_back("item g");
    page->get<stui::ListView>("list")->elements = args;

    stui::TreeView::Node root_node
    {
        "root",
        { 
            new stui::TreeView::Node{ "a", { }, 1, false },
            new stui::TreeView::Node{ "b", { }, 2, false },
            new stui::TreeView::Node{ "c", 
            {
                new stui::TreeView::Node{ "0", { }, 11, false },
                new stui::TreeView::Node{ "1", { }, 12, false },
                new stui::TreeView::Node{ "2", { }, 13, false },
                new stui::TreeView::Node{ "3", { }, 14, false }
            }, 5, false },
            new stui::TreeView::Node{ "d", { }, 3, false },
            new stui::TreeView::Node{ "e", { }, 4, false }
        },
        0,
        false
    };
    stui::TreeView* tree = page->get<stui::TreeView>("tree");
    tree->root = &root_node;

    float timer = 0.0f;

    page->focusable_component_sequence = { (*page)["rb"], (*page)["ta"], tib, (*page)["list"], tree };
    
    while (true)
    {
        stui::Renderer::FrameData delta = page->framerate(24);
        timer += delta.delta_time;

        pb->fraction = delta.active_fraction;//fmod(timer, 1.0f);
        t1->text = "fps: " + to_string(1.0f / delta.delta_time);
        s1->state = (timer * 8);
        s2->state = s1->state;
        s3->state = s1->state;
        s4->state = s1->state;

        if (timer > 2.0f)
        {
            timer -= 2.0f;
        }

        page->checkInput();
        page->render();
    }
}