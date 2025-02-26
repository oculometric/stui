#define STUI_IMPLEMENTATION
#define DEBUG
#include <stui.h>

size_t focus_index = 1;

void increment_focus()
{
    focus_index = (focus_index + 1) % 5;
}

int main(int argc, char** argv)
{

    stui::Terminal::configure();

    stui::Label t1("text widget", -1);
    stui::Spinner s1(0, 0);
    stui::Spinner s2(0, 1);
    stui::Spinner s3(0, 2);
    stui::Spinner s4(0, 3);
    stui::HorizontalSpacer hs(2);
    stui::Label t2("right-aligned text", 1);
    stui::HorizontalBox top_box({ &t1, &hs, &s1, &hs, &s2, &hs, &s3, &hs, &s4, &hs, &t2 });
    stui::ProgressBar pb(0.2f);
    stui::TextInputBox tib("type in me", nullptr, true);
    stui::BorderedBox box0(&tib, "demo text input");

    stui::VerticalSpacer vs(2);

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
    stui::ImageView image_view(image, stui::Coordinate{ 12,12 });

    string demo_text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec viverra nisi non metus feugiat, ut tempus massa sodales. Quisque efficitur finibus nibh, vel vulputate purus facilisis ac. Duis pharetra orci ac tincidunt suscipit. Nam hendrerit fringilla orci, ac commodo leo viverra vel. Donec vel vestibulum quam, in consectetur libero. Nunc pulvinar ligula et diam mollis porta. Etiam ac lobortis justo. Nunc scelerisque velit quis lectus dictum, et fermentum dui congue. Praesent semper luctus nisi ac tincidunt. Proin semper turpis vel quam mattis ultricies. Aenean varius quis neque eget feugiat. Etiam est odio, auctor eu enim a, convallis viverra leo. Suspendisse potenti. Nunc sit amet tellus sit amet magna imperdiet dictum id ut nisl.\n\nEtiam maximus pharetra elementum. Proin a tempus ante. Aenean ut arcu eu tellus gravida laoreet et rhoncus dolor. Nullam elementum, ante vel gravida congue, tellus dui vehicula lectus, a posuere leo nisi nec leo. Integer vel lobortis sem. Maecenas luctus semper magna non vehicula. Cras maximus lorem urna, ut malesuada enim varius eu. Fusce tristique tincidunt eros, at semper nunc tincidunt ut. Phasellus sagittis lectus ac pretium mattis. Vestibulum dictum elementum pellentesque. In eget eros nunc.Etiam nisl metus, feugiat a purus et, viverra pellentesque ante. Phasellus malesuada cursus risus ac semper. Suspendisse molestie purus ac augue aliquam, sit amet dapibus odio auctor. Donec vitae odio elit. Phasellus et ligula ac nunc dictum suscipit non pretium turpis.";
    stui::TextArea ta(demo_text, 0);
    stui::BorderedBox box1(&ta, "demo text");
    stui::RadioButton rb({"option 1", "option 2", "option 3", "option 4"}, 0, true);
    stui::HorizontalBox box4({ &box1, &rb });

    stui::VerticalBox right_box({ &image_view, &box4 });
    
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
    stui::ListView list(args, 0, 0);
    stui::BorderedBox box2(&list, "program args");

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
    stui::TreeView tree(&root_node, 0, 0);
    stui::BorderedBox box3(&tree, "tree demo");

    stui::VerticalBox left_box({ &box2, &box3 });
    stui::SizeLimiter left_limiter(&left_box, stui::Coordinate{ 30, -1 });

    stui::HorizontalBox lower_box({ &left_limiter, &right_box });

    stui::HorizontalDivider hd;

    stui::VerticalBox root({ &top_box, &hd, &pb, &box0, &vs, &lower_box });

    auto frame_time = clock_type::now();
    float timer = 0.0f;

    vector<stui::Component*> focusables = { &rb, &ta, &tib, &list, &tree };
    
    while (true)
    {
        stui::Renderer::FrameData delta = stui::Renderer::targetFramerate(24, frame_time);
        timer += delta.delta_time;

        pb.fraction = delta.active_fraction;//fmod(timer, 1.0f);
        t1.text = "fps: " + to_string(1.0f / delta.delta_time);
        s1.state = (timer * 8);
        s2.state = s1.state;
        s3.state = s1.state;
        s4.state = s1.state;

        if (timer > 2.0f)
        {
            timer -= 2.0f;
        }

        for (size_t i = 0; i < focusables.size(); i++) focusables[i]->focused = (i == focus_index);

        stui::Renderer::handleInput(focusables[focus_index], { stui::Input::Shortcut{ stui::Input::Key{ '\t', stui::Input::ControlKeys::NONE }, increment_focus } });

        stui::Renderer::render(&root);
    }
}