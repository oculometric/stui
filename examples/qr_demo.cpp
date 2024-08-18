#define STUI_IMPLEMENTATION
#define DEBUG
#include <stui.h>
#include <stui_extensions.h>

int main(int argc, const char** argv)
{

    stui::Terminal::configure("QR Code Demo");

    stui::QRCodeView qr("text");

    stui::Renderer::render(&qr);
    stui::Terminal::unConfigure(false);

    return 0;
}