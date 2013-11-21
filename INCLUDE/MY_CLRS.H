#include "stdafx.h"
//class CColorDialog : public CDialog
class my_color : public CColorDialog
{
public:
  virtual int DoModal();
  COLORREF color();
  ~my_color();
};  