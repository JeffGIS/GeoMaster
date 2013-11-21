    case GF_CLOSE: 
        RemoveGraphicsFunction (hWnd);
    	break;
        
		PostMessage(hWnd, GF_CLOSE,0, 0L); 
