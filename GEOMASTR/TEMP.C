    case WM_CHAR:
		switch (wParam)
		{
			case VK_F12:
				goto LoadGFMenu;
		 	default:
{
#if ENABLETRACE
GSSiExitProg (438);
#endif
			return DefWindowProc(hWnd, Message, wParam, lParam);
}       
		}
		break;
