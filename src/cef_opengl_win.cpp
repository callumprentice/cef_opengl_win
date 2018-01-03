/*
    CEF and OpenGL simple test
    Copyright(c) 2018 Callum Prentice (callum@gmail.com)

    MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "cef_app.h"
#include "cef_client.h"
#include "wrapper/cef_helpers.h"

#include <windows.h>
#include <windowsx.h>
#include <gl\gl.h>

#include <iostream>
#include <list>

HGLRC hRC = 0;
HDC hDC = 0;
GLuint gWidth = 800;
GLuint gHeight = 800;
GLuint gDepth = 4;
CefRect gPopupRect;
unsigned char* gPagePixels = nullptr;
unsigned char* gPopupPixels = nullptr;
bool gExitFlag = false;

// HTML form with SELECT elements
CefString gStartURL = "data:text/html;charset=utf-8;base64,PGh0bWw+DQo8aGVhZD4NCjxzdHlsZT4NCnNlbGVjdCwgaW5wdXQgew0KZm9udC1zaXplOiAzMHB4Ow0KZm9udC1mYW1pbHk6ICJDb3VyaWVyIE5ldyINCn0NCjwvc3R5bGU+DQo8L2hlYWQ+DQo8Ym9keT4NCjxpbnB1dCB0eXBlPSJ0ZXh0IiBpZD0ib3V0cHV0IiBwbGFjZWhvbGRlcj0iU2VsZWN0IGFuIG9wdGlvbiI+DQo8cD4NCjxzZWxlY3QgaWQ9InNlbCIgb25jaGFuZ2U9ImRpc3BsYXkoKSI+DQo8b3B0aW9uIHZhbHVlPSJ2YWx1ZTEiPlZhbHVlIDE8L29wdGlvbj4gDQo8b3B0aW9uIHZhbHVlPSJ2YWx1ZTIiPlZhbHVlIDI8L29wdGlvbj4NCjxvcHRpb24gdmFsdWU9InZhbHVlMyI+VmFsdWUgMzwvb3B0aW9uPg0KPG9wdGlvbiB2YWx1ZT0idmFsdWU0Ij5WYWx1ZSA0PC9vcHRpb24+DQo8b3B0aW9uIHZhbHVlPSJ2YWx1ZTUiPlZhbHVlIDU8L29wdGlvbj4NCjxvcHRpb24gdmFsdWU9InZhbHVlNiI+VmFsdWUgNjwvb3B0aW9uPg0KPG9wdGlvbiB2YWx1ZT0idmFsdWU3Ij5WYWx1ZSA3PC9vcHRpb24+DQo8b3B0aW9uIHZhbHVlPSJ2YWx1ZTgiPlZhbHVlIDg8L29wdGlvbj4NCjxvcHRpb24gdmFsdWU9InZhbHVlOSI+VmFsdWUgOTwvb3B0aW9uPg0KPC9zZWxlY3Q+DQo8c2NyaXB0Pg0KZnVuY3Rpb24gZGlzcGxheSgpIHsNCmRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJvdXRwdXQiKS52YWx1ZSA9IGRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJzZWwiKS52YWx1ZTsNCn0NCjwvc2NyaXB0Pg0KPC9ib2R5Pg0KPC9odG1sPg==";

/////////////////////////////////////////////////////////////////////////////////
//
class RenderHandler :
    public CefRenderHandler
{
    public:
        bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
        {
            CEF_REQUIRE_UI_THREAD();

            rect = CefRect(0, 0, gWidth, gHeight);
            return true;
        }

        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override
        {
            CEF_REQUIRE_UI_THREAD();

            // whole page was updated
            if (type == PET_VIEW)
            {
                std::cout << "OnPaint() for page: " << width << " x " << height << std::endl;

                // make a buffer for whole page if not there already and copy in pixels
                if (gPagePixels == nullptr)
                {
                    gPagePixels = new unsigned char[width * height * gDepth];
                    memset(gPagePixels, 0, width * height * gDepth);
                }
                memcpy(gPagePixels, buffer, width * height * gDepth);

                // if there is still a popup open, write it into the page too (it's pixels will have been
                // copied into it's buffer by a call to OnPaint with type of PET_POPUP earlier)
                if (gPopupPixels != nullptr)
                {
                    unsigned char* dst = gPagePixels + gPopupRect.y * gWidth * gDepth + gPopupRect.x * gDepth;
                    unsigned char* src = (unsigned char*)gPopupPixels;
                    while (src < (unsigned char*)gPopupPixels + gPopupRect.width * gPopupRect.height * gDepth)
                    {
                        memcpy(dst, src, gPopupRect.width * gDepth);
                        src += gPopupRect.width * gDepth;
                        dst += gWidth * gDepth;
                    }
                }

            }
            // popup was updated
            else if (type == PET_POPUP)
            {
                std::cout << "OnPaint() for popup: " << width << " x " << height << " at " << gPopupRect.x << " x " << gPopupRect.y << std::endl;

                // copy over the popup pixels into it's buffer
                // (popup buffer created in onPopupSize() as we know the size there)
                memcpy(gPopupPixels, buffer, width * height * gDepth);

                // copy over popup pixels into page pixels. We need this for when popup is changing (e.g. highlighting or scrolling)
                // when the containing page is not changing and therefore doesn't get an OnPaint update
                unsigned char* src = (unsigned char*)gPopupPixels;
                unsigned char* dst = gPagePixels + gPopupRect.y * gWidth * gDepth + gPopupRect.x * gDepth;
                while (src < (unsigned char*)gPopupPixels + gPopupRect.width * gPopupRect.height * gDepth)
                {
                    memcpy(dst, src, gPopupRect.width * gDepth);
                    src += gPopupRect.width * gDepth;
                    dst += gWidth * gDepth;
                }
            }

            // write the final composited buffer into our OpenGL texture
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gWidth, gHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, gPagePixels);
        }

        void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override
        {
            CEF_REQUIRE_UI_THREAD();
            std::cout << "CefRenderHandler::OnPopupShow(" << (show ? "true" : "false") << ")" << std::endl;

            if (! show)
            {
                delete gPopupPixels;
                gPopupPixels = nullptr;
                gPopupRect.Reset();
            }
        }

        void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override
        {
            CEF_REQUIRE_UI_THREAD();
            std::cout << "CefRenderHandler::OnPopupSize(" << rect.width << " x " << rect.height << ") at " << rect.x << ", " << rect.y << std::endl;

            gPopupRect = rect;

            if (gPopupPixels == nullptr)
            {
                gPopupPixels = new unsigned char[rect.width * rect.height * gDepth];
                memset(gPopupPixels, 0, rect.width * rect.height * gDepth);
            }
        }

        IMPLEMENT_REFCOUNTING(RenderHandler);
};

/////////////////////////////////////////////////////////////////////////////////
//
class BrowserClient :
    public CefClient,
    public CefLifeSpanHandler,
    public CefLoadHandler
{
    public:
        BrowserClient(RenderHandler* render_handler) :
            mRenderHandler(render_handler)
        {
        }

        CefRefPtr<CefRenderHandler> GetRenderHandler() override
        {
            return mRenderHandler;
        }

        CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
        {
            return this;
        }

        void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
        {
            CEF_REQUIRE_UI_THREAD();

            mBrowserList.push_back(browser);
        }

        void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
        {
            CEF_REQUIRE_UI_THREAD();

            BrowserList::iterator bit = mBrowserList.begin();
            for (; bit != mBrowserList.end(); ++bit)
            {
                if ((*bit)->IsSame(browser))
                {
                    mBrowserList.erase(bit);
                    break;
                }
            }

            if (mBrowserList.empty())
            {
                gExitFlag = true;
            }
        }

        CefRefPtr<CefLoadHandler> GetLoadHandler() override
        {
            return this;
        }

        void OnLoadStart(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         TransitionType transition_type) override
        {
            CEF_REQUIRE_UI_THREAD();

            if (frame->IsMain())
            {
                std::cout << "Loading started" << std::endl;
            }
        }

        void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                       CefRefPtr<CefFrame> frame,
                       int httpStatusCode) override
        {
            CEF_REQUIRE_UI_THREAD();

            if (frame->IsMain())
            {
                const std::string url = frame->GetURL();

                std::cout << "Load ended for URL: " << url << " with HTTP status code: " << httpStatusCode << std::endl;
            }
        }

        IMPLEMENT_REFCOUNTING(BrowserClient);

    private:
        CefRefPtr<CefRenderHandler> mRenderHandler;
        typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
        BrowserList mBrowserList;
};

/////////////////////////////////////////////////////////////////////////////////
//
class cefImpl :
    public CefApp
{
    public:
        bool cefImpl::init()
        {
            CefSettings settings;

            CefMainArgs args(GetModuleHandle(NULL));
            CefString(&settings.browser_subprocess_path) = "cef_opengl_win_host.exe";
            settings.multi_threaded_message_loop = false;

            if (CefInitialize(args, settings, this, NULL))
            {
                std::cout << "cefImpl: initialized okay" << std::endl;

                mRenderHandler = new RenderHandler();

                mBrowserClient = new BrowserClient(mRenderHandler);

                CefWindowInfo window_info;
                window_info.windowless_rendering_enabled = true;

                CefBrowserSettings browser_settings;
                browser_settings.windowless_frame_rate = 60;
                browser_settings.background_color = 0xffffffff;

                mBrowser = CefBrowserHost::CreateBrowserSync(window_info, mBrowserClient.get(), gStartURL, browser_settings, nullptr);

                return true;
            }

            std::cout << "cefImpl: Unable to initialize" << std::endl;
            return false;
        }

        void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override
        {
            if (process_type.empty())
            {
                command_line->AppendSwitch("disable-gpu");
                command_line->AppendSwitch("disable-gpu-compositing");
            }
        }

        void update()
        {
            CefDoMessageLoopWork();
        }

        void mouseButton(int x, int y, bool is_up)
        {
            if (mBrowser && mBrowser->GetHost())
            {
                mBrowser->GetHost()->SendFocusEvent(true);

                CefMouseEvent cef_mouse_event;
                cef_mouse_event.x = x;
                cef_mouse_event.y = y;

                CefBrowserHost::MouseButtonType btn_type = MBT_LEFT;
                int last_click_count = 1;

                mBrowser->GetHost()->SendMouseClickEvent(cef_mouse_event, btn_type, is_up, last_click_count);
            }
        }

        void mouseMove(int x, int y)
        {
            if (mBrowser && mBrowser->GetHost())
            {
                CefMouseEvent cef_mouse_event;
                cef_mouse_event.x = x;
                cef_mouse_event.y = y;

                bool mouse_leave = false;
                mBrowser->GetHost()->SendMouseMoveEvent(cef_mouse_event, mouse_leave);
            }
        }

        void requestExit()
        {
            if (mBrowser.get() && mBrowser->GetHost())
            {
                mBrowser->GetHost()->CloseBrowser(true);
            }
        }

        void shutdown()
        {
            mRenderHandler = NULL;
            mBrowserClient = NULL;
            mBrowser = NULL;

            CefShutdown();
        }

        IMPLEMENT_REFCOUNTING(cefImpl);

    private:
        CefRefPtr<RenderHandler> mRenderHandler;
        CefRefPtr<BrowserClient> mBrowserClient;
        CefRefPtr<CefBrowser> mBrowser;
};

cefImpl* gCefImpl = nullptr;

/////////////////////////////////////////////////////////////////////////////////
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
                                         PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
                                         16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0,
                                         PFD_MAIN_PLANE, 0, 0, 0, 0
                                       };
    switch (message)
    {
        case WM_CREATE:
        {
            hDC = GetDC(hWnd);
            GLuint pixel_format = ChoosePixelFormat(hDC, &pfd);
            SetPixelFormat(hDC, pixel_format, &pfd);
            hRC = wglCreateContext(hDC);
            wglMakeCurrent(hDC, hRC);

            RECT real_window_size;
            GetClientRect(hWnd, &real_window_size);
            gWidth = real_window_size.right + 1;
            gHeight = real_window_size.bottom + 1;

            glEnable(GL_TEXTURE_2D);
            GLuint gAppTexture = 0;
            glGenTextures(1, &gAppTexture);
            glBindTexture(GL_TEXTURE_2D, gAppTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gWidth, gHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glViewport(0, 0, gWidth, gHeight);
            glOrtho(0.0f, gWidth, gHeight, 0.0f, -1.0f, 1.0f);

            gCefImpl = new cefImpl();
            gCefImpl->init();
        }
        break;

        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if ((GLuint)x < gWidth && (GLuint)y < gHeight)
            {
                gCefImpl->mouseMove(x, y);
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            bool up = false;
            gCefImpl->mouseButton(x, y, up);
        }
        break;

        case WM_LBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            bool up = true;
            gCefImpl->mouseButton(x, y, up);
        }
        break;

        case WM_KEYDOWN:
        {
            if (wParam == 27)
            {
                SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
        }
        break;

        case WM_DESTROY:
        case WM_CLOSE:
            wglMakeCurrent(hDC, NULL);
            wglDeleteContext(hRC);
            ReleaseDC(hWnd, hDC);

            gCefImpl->requestExit();
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

/////////////////////////////////////////////////////////////////////////////////
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    AllocConsole();
    FILE* outputConsole;
    freopen_s(&outputConsole, "CON", "w", stdout);
    freopen_s(&outputConsole, "CON", "w", stderr);

    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "OpenGLWindowClass";

    RegisterClass(&wc);

    HWND hWnd = CreateWindow("OpenGLWindowClass", "CEF OpenGL Test",
                             WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                             100, 0, gWidth, gHeight,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    SetFocus(hWnd);
    wglMakeCurrent(hDC, hRC);

    MSG msg;
    while (!gExitFlag)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                return TRUE;
            }
        }

        gCefImpl->update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(1.0f, 0.0f);
            glVertex2d(gWidth, 0);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2d(0, 0);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2d(0, gHeight);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2d(gWidth, gHeight);
        }
        glEnd();

        SwapBuffers(hDC);
    }

    gCefImpl->shutdown();

    fclose(outputConsole);
    FreeConsole();

    return (int)msg.wParam;
}
