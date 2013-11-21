// this is used by polyarea.c 

//     Common:  PolyCom.h //
typedef struct 
      {
        long               num_cens,          curr_refn,
        hit_line,          curr_line,         next_line,
        i_count,           i_start,           min_valid_line,
        west_hit_line,     min_new_rec_num,   max_90_rec,
        num_new_rec,       i_meet,            max_y, 
        min_x,             max_x,             min_y,
        poly_ids[16],      num_polys,         got_a_hit,       
        num_sides,         ifirst_start,      stat,
        trunum,            out_notes,         min_start,
        loc,               island_ref,        l,
        num_orig_sides,    max_num_sides_allowed;

        float  short_line, going_west,  going_east, trans;
                     
        double
        azimuth,             directions[64],      dminx,
        start_azm,           curr_azm,            delta_azim,
        next_azm,            rx,                  ry,
        offset,              real_x,              real_y,
        delta_x,             delta_y,             tol_fac,
        from_x[12],          from_y[12],          to_x[12],
        hit_x,               hit_y,               to_y[12],
        sign,                drotlen,             dis11,
        dis12,               dis21,               dis22,
        newdir,              taz,                 dazs[64],
        old_azm,             smallest_change,     az3;          

        BOOL       
        with_the_grain[64],  got_min,     good_one,  
        the_grain,           new_one,     shift, 
        first,               first_grain, debugging,
        old_grain,           close,       deletions ;
        
        HGLOBAL hGlobals[12];
        
        short
        possible_ways[64],     j2,             nldaloc,
        num_nodes,             lda_loc,        last_node,
        table_id,              umdb_id,        last_num,
        tru_num_areas,         num_exclusions, num_lines,
        poss_orig[64],         datloc[12],     base_rec,
        igood,                 iskip,          inext,
        next_orig,             k, j,           next ;
        
        HGLOBAL id1, id2, id3;
		HGLOBAL bid, bid2, bid3;
        unsigned short         link_desc;
        char    
        file_name[128], keyword[24], node_key[22],
        rec[256];
        
  short st,  i, ifirst, ns,     line_num;
  double az8 ,      az1,      az2,     
         azm[2],    x1[2],    y1[2],   dazm[64],      cx,    cy,
         this_azm,  ld2b,     r8off;
  short  itype,     grain,    old_line,
         the_rec,   his_node, num_here,   ineed1,    ineed2,    
         num_this_node; 
        
        
      } PolyCom1;
         
typedef PolyCom1 far *lpPolyCom1;         

/****************************************************/
// from here down is the multiple item struct
typedef struct
      {
        double  iazm, //0 for straight line, else the deflection        
                from_x_array,    
                from_y_array, 
                to_x_array,
                to_y_array;
        short   line_desc[3], //[left area; right area; the line desc of itself
                WorkLoc;  //location of parent line info in PolyCom1
        BOOL   valid;
      } PolyCom2;
typedef PolyCom2 *lpPolyCom2;         


// the following structure contains elements
// used to hold everything found by PolyArea
typedef struct
      { 
        short all_sides, //location of line info in PolyCom2
              orig_lines;//location of line info in PolyCom1
        char  all_dir;
      } AllSides;
typedef AllSides far *lpAS;

// the following structure contains line description of the
// original lines (I.E., the whole lines... not those segmented
// during the intersect all with all process.

typedef struct
      { 
        double   ldax1, lday1, ldax2, lday2,
                 ldaazm,  ldalngth, ldarad;
        unsigned short 
                 poly_origs, type, ldadesc;
        long     ldaref;                
        short    valid,  //may become invalid if a deadend  
                 line_desc,
                 ldarec,
                 intxs;
      } WorkSpace;
typedef WorkSpace  *lpWS;

typedef struct
      {
        unsigned short Poly2Loc,
                       Poly1Loc;
        char           direction;
      } TempArea;
typedef TempArea *lpTA;                               
                       

typedef struct
      { 
         unsigned short node_array;
      } NodeArray;
      
typedef NodeArray far *lpNA;      
         

typedef struct
   {
     short
           AreaDesc,//Area Number of the first line 
           i_count,  //number of sides of the area
           j,        //loc of hit_line in all_sides
           num_excl;    //number of exclusions
     long  CenX, 
           CenY, 
           CenRefn;
     double
           min_x, //for the area
           min_y, //for the area
           max_x, //for the area
           max_y; //for the area
   }  gen_info;

typedef gen_info far *lpGenInfo; 

typedef struct
     {
       short   nPartners;
       lpWS    LBG[32], //Line Before Going
               LACB[32];//Line After Coming Back
     } Links;
typedef Links far *lpLinks;                 
                 

