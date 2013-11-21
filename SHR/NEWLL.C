
DPOINT NewLatLong(double Lat,double Long,double Dist,double Direction)
{
 DPOINT LatLong;
//given:          Fixed 7 Oct 96 lda
//  Point N is at the North Pole
//  Point A is at the Given Lat,Long
//  Length of two sides of a triangle and the angle it forms
    double ans, cosc,cosb,cosa,sinb,sinc,sina,cosC,cosA;



    cosc = cos(Dist/6371100);
    sinc = sin(Dist/6371100);
    cosb = cos((90e0-Lat)/RADIAN);
    sinb = sin((90e0-Lat)/RADIAN);
    cosa = cos(Direction)*sinb*sinc+cosb*cosc;
    cosA = acos(cosa);
    cosA *= RADIAN;
  //  if((Direction < PY && Direction > HALFPI) ||
  //     (Direction > PIHALF))      
  //     LatLong.y =  cosA;
  //  else
       LatLong.y = 90e0 - cosA;   
    sina = acos(cosa);
    sina = sin(sina);
    cosC = (cosc-(cosb*cosa)) / (sinb*sina);
    cosC = acos(cosC);
    cosC *= RADIAN;
    if(Direction > PY)
      LatLong.x = Long - cosC;
    else
      LatLong.x = Long + cosC;  
    

  return LatLong;
}                        