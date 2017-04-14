#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>


/* NOTE! We must include the file "gfxmacros.h" inorder to be able to */
/* use the function (macro) SetDrPt().                                */


#define WIDTH  320 /* 320 pixels wide (low resolution)                */
#define HEIGHT 200 /* 200 lines high (non interlaced NTSC display)    */ 
#define DEPTH    2 /* 2 BitPlanes should be used, gives four colours. */
#define COLOURS  4 /* 2^2 = 4                                         */


struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;


struct View my_view;
struct View *my_old_view;
struct ViewPort my_view_port;
struct RasInfo my_ras_info;
struct BitMap my_bit_map;
struct RastPort my_rast_port;

char* string;// = "Hello!";
int i, mainloop, sincounter = 0;


UWORD my_color_table[] =
{
  0x000, /* Colour 0, Black */
  0xF00, /* Colour 1, Red   */
  0xDDD, /* Colour 2, Whiteish */
  0x00F  /* Colour 3, Blue  */
};


/* The coordinates for the PolyDraw() function: (Creates a small box) */
WORD coordinates[] =
{
  100, 10,
  140, 10,
  140, 50,
  100, 50,
  100, 10
};

BYTE sintable[256]={
     50,51,52,53,54,56,57,58,59,60,62,63,64,65,66,68,
     69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,
     85,86,87,87,88,89,90,90,91,92,92,93,94,94,95,95,
     96,96,97,97,97,98,98,98,99,99,99,99,99,99,99,99,
     99,99,99,99,99,99,99,99,98,98,98,98,97,97,96,96,
     96,95,95,94,93,93,92,92,91,90,89,89,88,87,86,85,
     85,84,83,82,81,80,79,78,77,76,75,74,73,72,70,69,
     68,67,66,65,63,62,61,60,59,57,56,55,54,53,51,50,
     49,48,46,45,44,43,42,40,39,38,37,36,34,33,32,31,
     30,29,27,26,25,24,23,22,21,20,19,18,17,16,15,14,
     14,13,12,11,10,10,9,8,7,7,6,6,5,4,4,3,
     3,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,1,1,1,2,2,2,3,3,
     4,4,5,5,6,7,7,8,9,9,10,11,12,12,13,14,
     15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
     31,33,34,35,36,37,39,40,41,42,43,45,46,47,48,50
};


void clean_up();
void main();


void main()
{
  UWORD *pointer;
  int loop;
  

  /* Open the Intuition library: */
  IntuitionBase = (struct IntuitionBase *)
    OpenLibrary( "intuition.library", 0 );
  if( !IntuitionBase )
    clean_up( "Could NOT open the Intuition library!" );

  /* Open the Graphics library: */
  GfxBase = (struct GfxBase *)
    OpenLibrary( "graphics.library", 0 );
  if( !GfxBase )
    clean_up( "Could NOT open the Graphics library!" );


  /* Save the current View, so we can restore it later: */
  my_old_view = GfxBase->ActiView;


  /* 1. Prepare the View structure, and give it a pointer to */
  /*    the first ViewPort:                                  */
  InitView( &my_view );
  my_view.ViewPort = &my_view_port;


  /* 2. Prepare the ViewPort structure, and set some important values: */
  InitVPort( &my_view_port );
  my_view_port.DWidth = WIDTH;         /* Set the width.                */
  my_view_port.DHeight = HEIGHT;       /* Set the height.               */
  my_view_port.RasInfo = &my_ras_info; /* Give it a pointer to RasInfo. */
  my_view_port.Modes = NULL;           /* Low resolution.               */


  /* 3. Get a colour map, link it to the ViewPort, and prepare it: */
  my_view_port.ColorMap = (struct ColorMap *) GetColorMap( COLOURS );
  if( my_view_port.ColorMap == NULL )
    clean_up( "Could NOT get a ColorMap!" );

  /* Get a pointer to the colour map: */
  pointer = (UWORD *) my_view_port.ColorMap->ColorTable;

  /* Set the colours: */
  for( loop = 0; loop < COLOURS; loop++ )
    *pointer++ = my_color_table[ loop ];


  /* 4. Prepare the BitMap: */
  InitBitMap( &my_bit_map, DEPTH, WIDTH, HEIGHT );

  /* Allocate memory for the Raster: */ 
  for( loop = 0; loop < DEPTH; loop++ )
  {
    my_bit_map.Planes[ loop ] = (PLANEPTR) AllocRaster( WIDTH, HEIGHT );
    if( my_bit_map.Planes[ loop ] == NULL )
      clean_up( "Could NOT allocate enough memory for the raster!" );

    /* Clear the display memory with help of the Blitter: */
    //BltClear( my_bit_map.Planes[ loop ], RASSIZE( WIDTH, HEIGHT ), 0 );
  }

  
  /* 5. Prepare the RasInfo structure: */
  my_ras_info.BitMap = &my_bit_map; /* Pointer to the BitMap structure.  */
  my_ras_info.RxOffset = 0;         /* The top left corner of the Raster */
  my_ras_info.RyOffset = 0;         /* should be at the top left corner  */
                                    /* of the display.                   */
  my_ras_info.Next = NULL;          /* Single playfield - only one       */
                                    /* RasInfo structure is necessary.   */

  /* 6. Create the display: */
  MakeVPort( &my_view, &my_view_port );
  MrgCop( &my_view );


  /* 7. Prepare the RastPort, and give it a pointer to the BitMap. */
  InitRastPort( &my_rast_port );
  my_rast_port.BitMap = &my_bit_map;
  

  /* 8. Show the new View: */
  LoadView( &my_view );


  SetDrMd( &my_rast_port, JAM1 ); /* Use FgPen only. */
  SetAPen( &my_rast_port, 2 );    /* FgPen: Green    */
  SetBPen( &my_rast_port, 1 );    /* BgPen: Red      */

  /* Write "Hello!" with FgPen (green), do not change the background: */
  string = "Here is a long sentence";

  for (mainloop = 0; mainloop < 400; mainloop++) {
    for( loop = 0; loop < DEPTH; loop++ )
    {
      BltClear( my_bit_map.Planes[ loop ], RASSIZE( WIDTH, HEIGHT ), 0 );
    } 

    for (i = 0; i < strlen(string); i++) {
      //Move( &my_rast_port, 30+sintable[sincounter+(i*4)]/8, 10+(i*7) );
      Move( &my_rast_port, 30+(i*7), 30+sintable[sincounter+(i*4)]/8 );
      Text( &my_rast_port, &string[i], 1 );
    }
    for (i = 0; i < strlen(string); i++) {
      Move( &my_rast_port, 30+sintable[sincounter+(i*4)]/8, 30+(i*7) );
      //Move( &my_rast_port, 30+(i*7), 10+sintable[sincounter+(i*4)]/8 );
      Text( &my_rast_port, &string[i], 1 );
    }

    sincounter += 1;
    if (sincounter >= 256 - (strlen(string)*4)) sincounter = 0;

    MakeVPort( &my_view, &my_view_port );
    MrgCop( &my_view );
    LoadView( &my_view );
    WaitTOF();  // Wait for end of drawing; limits speed
  }

  /* Write "Hello!" with FgPen and change background to BgPen: */
  /* (Green text on red background.)                           */
  //SetDrMd( &my_rast_port, JAM2 );
  //Move( &my_rast_port, 30, 20 );
  //Text( &my_rast_port, "Hello!", 6 );

  /* Inversed JAM1. Black text on green background: */
  //SetDrMd( &my_rast_port, JAM1|INVERSVID );
  //Move( &my_rast_port, 30, 30 );
  //Text( &my_rast_port, "Hello!", 6 );

  /* Inversed JAM2. Red text on black background: */
  //SetDrMd( &my_rast_port, JAM2|INVERSVID );
  //Move( &my_rast_port, 30, 40 );
  //Text( &my_rast_port, "Hello!", 6 );

  /* Print the text in red with a green shadow: */
  /* JAM1, green text background unchanged (black): */
  //SetDrMd( &my_rast_port, JAM1 );
  //Move( &my_rast_port, 30, 50 );
  //Text( &my_rast_port, "Hello!", 6 );
  /* Change FgPen to red: */
  //SetAPen( &my_rast_port, 1 );
  //Move( &my_rast_port, 31, 51 );
  //Text( &my_rast_port, "Hello!", 6 );

  /* Wait 20 seconds: */
  //Delay( 50 * 3 );


  /* 9. Restore the old View: */
  LoadView( my_old_view );


  /* Free all allocated resources and leave. */
  clean_up( "THE END" );
}


/* Returns all allocated resources: */
void clean_up( message )
STRPTR message;
{
  int loop;

  /* Free automatically allocated display structures: */
  FreeVPortCopLists( &my_view_port );
  FreeCprList( my_view.LOFCprList );
  
  /* Deallocate the display memory, BitPlane for BitPlane: */
  for( loop = 0; loop < DEPTH; loop++ )
    if( my_bit_map.Planes[ loop ] )
      FreeRaster( my_bit_map.Planes[ loop ], WIDTH, HEIGHT );

  /* Deallocate the ColorMap: */
  if( my_view_port.ColorMap ) FreeColorMap( my_view_port.ColorMap );

  /* Close the Graphics library: */
  if( GfxBase ) CloseLibrary( GfxBase );

  /* Close the Intuition library: */
  if( IntuitionBase ) CloseLibrary( IntuitionBase );

  /* Print the message and leave: */
  printf( "%s\n", message ); 
  exit();
}
