#include <Windows.h>
#include <windowsx.h>

POINT p_start;
POINT p_mouse;

enum Shape_Mode {
	LINE_MODE,
	RECT_MODE,
	ELLI_MODE,
};

enum Shape_Mode mode = LINE_MODE;

COLORREF brush_colour = RGB(255, 255, 255);
int brush_mode;
int brush_style;

COLORREF pen_colour;
int pen_width = 1;
int pen_style = PS_SOLID;

HBITMAP hbSavedDrawing;
HDC hMemDC;
BOOL has_saved = TRUE;

COLORREF ShowColourDialog(HWND hwnd)
{
	CHOOSECOLOR chooser;
	static COLORREF crCustomColours[16];

	ZeroMemory(&chooser, sizeof(chooser));

	chooser.lStructSize = sizeof(chooser);
	chooser.hwndOwner = hwnd;
	chooser.lpCustColors = (LPDWORD)crCustomColours;
	chooser.rgbResult = RGB(0, 255, 0);
	chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
	ChooseColor(&chooser);

	return chooser.rgbResult;
}
RECT last_painted_rect;
void paint_current_event(HWND hwnd, BOOL save_to_buffer){

	// current painting rectangle
	RECT crect;
	crect.left = min(p_mouse.x, p_start.x);
	crect.right = max(p_mouse.x, p_start.x);
	crect.top = min(p_mouse.y, p_start.y);
	crect.bottom = max(p_mouse.y, p_start.y);

	// last paint + current paint
	RECT rect;
	rect.left = min(crect.left, last_painted_rect.left);
	rect.right = max(crect.right, last_painted_rect.right);
	rect.top = min(crect.top, last_painted_rect.top);
	rect.bottom = max(crect.bottom, last_painted_rect.bottom);

	InvalidateRect(hwnd, &rect, FALSE);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	RECT clientAreaSize;
	GetClientRect(hwnd, &clientAreaSize);
	if (has_saved) {
		BitBlt(hdc, 0, 0, clientAreaSize.right, clientAreaSize.bottom, hMemDC, 0, 0, SRCCOPY);
	}


	HPEN hMyPen = CreatePen(pen_style, pen_width, pen_colour);
	HBRUSH hMyBrush;
	if (brush_mode == 0) {
		hMyBrush = CreateSolidBrush(brush_colour);
	}
	else if (brush_mode == 1) {
		hMyBrush = CreateHatchBrush(brush_style, brush_colour);
	}
	SelectObject(hdc, hMyPen);
	SelectObject(hdc, hMyBrush);

	//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	if (mode == LINE_MODE) {
		MoveToEx(hdc, p_start.x, p_start.y, 0);
		LineTo(hdc, p_mouse.x, p_mouse.y);
	}
	else if (mode == RECT_MODE) {
		Rectangle(hdc, crect.left, crect.top, crect.right, crect.bottom);
	}
	else if (mode == ELLI_MODE) {
		Ellipse(hdc, crect.left, crect.top, crect.right, crect.bottom);
	}

	DeleteObject(hMyPen);
	DeleteObject(hMyBrush);

	if (save_to_buffer) {
		RECT clientAreaSize;
		GetClientRect(hwnd, &clientAreaSize);
		//Save drawing surface from window (hdc) to the bitmap selected in hMemDC
		BitBlt(hMemDC, 0, 0, clientAreaSize.right, clientAreaSize.bottom, hdc, 0, 0, SRCCOPY);
		has_saved = TRUE;
	}
	EndPaint(hwnd, &ps);
	ValidateRect(hwnd, &rect);

	last_painted_rect.left = crect.left;
	last_painted_rect.right = crect.right+1;
	last_painted_rect.top = crect.top;
	last_painted_rect.bottom = crect.bottom+1;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
		case WM_CREATE:
		{

			HMENU hMenuBar = CreateMenu();
			HMENU hFileMenu1 = CreateMenu();
			HMENU hShapeMenu1 = CreateMenu();
			HMENU hPenMenu1 = CreateMenu();
			HMENU hThicknessMenu1 = CreateMenu();
			HMENU hBrushMenu1 = CreateMenu();
			HMENU hStyleMenu1 = CreateMenu();
			HMENU hHelpMenu1 = CreateMenu();

			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu1, L"&File");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hShapeMenu1, L"&Shape");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hPenMenu1, L"&Pen");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hBrushMenu1, L"&Brush");
			AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu1, L"&Help");

			AppendMenu(hFileMenu1, MF_STRING, 1000, L"&Exit");

			AppendMenu(hShapeMenu1, MF_STRING, 1001, L"&Line");
			AppendMenu(hShapeMenu1, MF_STRING, 1002, L"&Rectangle");
			AppendMenu(hShapeMenu1, MF_STRING, 1003, L"&Ellipse");

			AppendMenu(hPenMenu1, MF_POPUP, (UINT_PTR)hThicknessMenu1, L"&Thickness");
			AppendMenu(hPenMenu1, MF_SEPARATOR, 0, 0);
			AppendMenu(hPenMenu1, MF_STRING, 1005, L"&Colour");
			AppendMenu(hPenMenu1, MF_SEPARATOR, 0, 0);
			AppendMenu(hPenMenu1, MF_STRING, 1006, L"&Solid");
			AppendMenu(hPenMenu1, MF_STRING, 1007, L"&Dashed");

			AppendMenu(hThicknessMenu1, MF_STRING, 1008, L"&1");
			AppendMenu(hThicknessMenu1, MF_STRING, 1009, L"&2");
			AppendMenu(hThicknessMenu1, MF_STRING, 1010, L"&3");
			AppendMenu(hThicknessMenu1, MF_STRING, 1011, L"&4");
			AppendMenu(hThicknessMenu1, MF_STRING, 1012, L"&5");

			AppendMenu(hBrushMenu1, MF_STRING, 1013, L"&Colour");
			AppendMenu(hBrushMenu1, MF_POPUP, (UINT_PTR)hStyleMenu1, L"&Style");

			AppendMenu(hStyleMenu1, MF_STRING, 1014, L"&Solid");
			AppendMenu(hStyleMenu1, MF_STRING, 1015, L"&45-degree upward left-to-right hatch");
			AppendMenu(hStyleMenu1, MF_STRING, 1016, L"&Horizontal and vertical crosshatch");
			AppendMenu(hStyleMenu1, MF_STRING, 1017, L"&45-degree crosshatch");
			AppendMenu(hStyleMenu1, MF_STRING, 1018, L"&45-degree downward left-to-right hatch");
			AppendMenu(hStyleMenu1, MF_STRING, 1019, L"&Horizontal hatch");
			AppendMenu(hStyleMenu1, MF_STRING, 1020, L"&Vertical hatch");

			AppendMenu(hHelpMenu1, MF_STRING, 1021, L"&Help");
			AppendMenu(hHelpMenu1, MF_STRING, 1022, L"&About");

			SetMenu(hwnd, hMenuBar);
		}
		case WM_PAINT: {

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT clientAreaSize;
			GetClientRect(hwnd, &clientAreaSize);
			BitBlt(hdc, 0, 0, clientAreaSize.right, clientAreaSize.bottom, hMemDC, 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);

			break;
		}
		case WM_SETREDRAW:{
			break;
		}
		case WM_COMMAND:
		{
			int cmd = LOWORD(wParam);

			if (cmd == 1000)
			{
				SendMessage(hwnd, WM_CLOSE, 0, 0);
			}
			else if (cmd == 1001)
			{
				mode = LINE_MODE;
			}
			else if (cmd == 1002)
			{
				mode = RECT_MODE;
			}
			else if (cmd == 1003)
			{
				mode = ELLI_MODE;
			}
			else if (cmd == 1005)
			{
				pen_colour = ShowColourDialog(hwnd);
			}
			else if (cmd == 1006) {
				pen_style = PS_SOLID;
			}
			else if (cmd == 1007) {
				pen_style = PS_DASH;
			}
			else if (cmd >= 1008 && cmd <= 1012) {
				pen_width = cmd - 1007;
			}
			else if (cmd == 1013) {
				brush_colour = ShowColourDialog(hwnd);
			}
			else if (cmd == 1014) {
				brush_mode = 0;
			}
			else if (cmd == 1015) {
				brush_mode = 1;
				brush_style = HS_BDIAGONAL;
			}
			else if (cmd == 1016) {
				brush_mode = 1;
				brush_style = HS_CROSS;
			}
			else if (cmd == 1017) {
				brush_mode = 1;
				brush_style = HS_DIAGCROSS;
			}
			else if (cmd == 1018) {
				brush_mode = 1;
				brush_style = HS_FDIAGONAL;
			}
			else if (cmd == 1019) {
				brush_mode = 1;
				brush_style = HS_HORIZONTAL;
			}
			else if (cmd == 1020) {
				brush_mode = 1;
				brush_style = HS_VERTICAL;
			}
			else if (cmd == 1021) {
				MessageBox(0, L"Be creative. Press LMB to start the line, release it to draw the object.", L"Help", MB_OK);
			}
			else if (cmd == 1022) {
				MessageBox(0, L"A small program for painting.", L"About", MB_OK);
			}
		}

		case WM_ERASEBKGND:
		{

			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_MOUSEMOVE:
		{
			p_mouse.x = GET_X_LPARAM(lParam);
			p_mouse.y = GET_Y_LPARAM(lParam);
			if (p_start.x != -1 && p_start.y != -1) {
				//RedrawWindow(hwnd, &last_painted_rect, NULL, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
				paint_current_event(hwnd, FALSE);
			}
			break;
		}
		case WM_LBUTTONUP:
		{
			paint_current_event(hwnd, TRUE);

			// SendMessage(hwnd, WM_SETREDRAW, FALSE, NULL);

			p_start.x = -1;
			p_start.y = -1;

			break;
		}
		case WM_LBUTTONDOWN:
		{
			p_start.x = p_mouse.x;
			p_start.y = p_mouse.y;

			break;
		}
		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR params, int nCmdShow)
{
	const wchar_t g_szClassName[] = L"PaintCClass";
	MSG message;
	HWND hwnd;
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(0, L"Error Registering Window!", L"Error", MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_szClassName, L"Paint on C", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);

	if (NULL == hwnd)
	{
		MessageBox(0, L"Error Creating Window!", L"Error", MB_OK);
		return 0;
	}


	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	HDC hdcWindow = GetDC(hwnd);
	hMemDC = CreateCompatibleDC(hdcWindow); //We will use hMemDC to save window surface to the bitmap.
	hbSavedDrawing = CreateCompatibleBitmap(hdcWindow, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	SelectObject(hMemDC, hbSavedDrawing); //Select the bitmap to use by hMemDC.
	ReleaseDC(hwnd, hdcWindow); //hdcWindow is no longer needed
	paint_current_event(hwnd, TRUE);

	p_start.x = -1;
	p_start.y = -1;
	last_painted_rect.top = 0;
	last_painted_rect.bottom = 0;
	last_painted_rect.left = 0;
	last_painted_rect.right = 0;

	while (GetMessage(&message, NULL, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	return message.wParam;
}
