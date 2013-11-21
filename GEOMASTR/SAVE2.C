      Lines->rad = lpDxf2GM->FL_VALUE[1] * ftm; // in meters 
      lpDxf2GM->SWEEP = lpDxf2GM->ANGLE[2] - lpDxf2GM->ANGLE[1]; //the curves length in radians
      Sweep = lpDxf2GM->SWEEP * lpDxf2GM->Radian;
      if(Sweep <= 0) 
            Sweep += TWOPI;
      Lines->lngth = -1e0* Sweep * Lines->rad;  //in radians
      if(fabs(Lines->lngth) > TWOPI*Lines->rad - 1e-5) 
        lpDxf2GM->POLY_CLOSE == TRUE;
      else
        lpDxf2GM->POLY_CLOSE == FALSE;
        
      Lines->azm  = lpDxf2GM->ANGLE[1] * lpDxf2GM->Radian; //rad -> PC azimuth
      Lines->eazm = lpDxf2GM->ANGLE[2] * lpDxf2GM->Radian; //rad -> PT azimuth
      azdf = AZDF(Lines->azm,Lines->eazm,Lines->lngth);
      if(azdf > Sweep + 1e-5 || azdf < Sweep - 1e-5)
           Lines->lngth *= -1e0;
      
      Lines->rx = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm;
      Lines->ry = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm; 
      //here we calculate the coords of the PC
      Lines->x1 = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm + Lines->rad * cos(Lines->azm);
      Lines->y1 = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm + Lines->rad * sin(Lines->azm);

      //Here we calculate the coords of the PT
      Lines->x2 = (lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1]) * ftm + Lines->rad * cos(Lines->eazm);
      Lines->y2 = (lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2]) * ftm + Lines->rad * sin(Lines->eazm);

    //  lpDxf2GM->XY[3][1] = lpDxf2GM->ZZZ[1];
    //  lpDxf2GM->XY[3][3] = lpDxf2GM->ZZZ[1]; 

          
  /*    lpDxf2GM->RADIUS = lpDxf2GM->FL_VALUE[1];
      lpDxf2GM->SWEEP = lpDxf2GM->ANGLE[2] - lpDxf2GM->ANGLE[1];
       if(lpDxf2GM->SWEEP <= 0) lpDxf2GM->SWEEP = lpDxf2GM->SWEEP + 36e1;
      lpDxf2GM->STANG  = lpDxf2GM->ANGLE[1];
      lpDxf2GM->ENDANG = lpDxf2GM->STANG + lpDxf2GM->SWEEP;
      lpDxf2GM->SWEEP = lpDxf2GM->STANG + (lpDxf2GM->SWEEP / 2e0);

//C      IF (ENDANG-STANG<0.) ENDANG = ENDANG + 360.
//C      IF (ENDANG-STANG>360.) ENDANG = ENDANG - 360.
      lpDxf2GM->STANG  = lpDxf2GM->STANG * lpDxf2GM->Radian;
      lpDxf2GM->SWEEP  = lpDxf2GM->SWEEP * lpDxf2GM->Radian;
      lpDxf2GM->ENDANG = lpDxf2GM->ENDANG * lpDxf2GM->Radian;
      lpDxf2GM->XY[1][1] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS*cos(lpDxf2GM->STANG);
      lpDxf2GM->XY[2][1] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS*sin(lpDxf2GM->STANG);
      lpDxf2GM->XY[1][2] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS*cos(lpDxf2GM->SWEEP);
      lpDxf2GM->XY[2][2] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS*sin(lpDxf2GM->SWEEP);
      lpDxf2GM->XY[1][3] = lpDxf2GM->XXX[1] + lpDxf2GM->OFFSET[1] + lpDxf2GM->RADIUS*cos(lpDxf2GM->ENDANG);
      lpDxf2GM->XY[2][3] = lpDxf2GM->YYY[1] + lpDxf2GM->OFFSET[2] + lpDxf2GM->RADIUS*sin(lpDxf2GM->ENDANG);
      lpDxf2GM->XY[3][1] = lpDxf2GM->ZZZ[1];
      lpDxf2GM->XY[3][3] = lpDxf2GM->ZZZ[1];   */
