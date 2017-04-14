#include <intuition/intuition.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <exec/types.h>
#include <hardware/custom.h>
#include <hardware/cia.h>

#define CIAAPRA 0xBFE001 

#define FIRE   1
#define RIGHT  2
#define LEFT   4
#define DOWN   8
#define UP    16

#define PORT1 1
#define PORT2 2

extern struct Custom far custom;
struct CIA *cia = (struct CIA *) CIAAPRA;

typedef enum {false, true} bool; // define bool

typedef struct ViewPortList {
	struct ViewPort this_view_port;
  	struct RasInfo ras_info;
  	struct BitMap bit_map;
  	struct RastPort rast_port;

  	int yPos, depth, colors;

	struct ViewPortList* next;
};

struct Screen_data {
	struct View view;
	struct View *old_view;
	struct IntuitionBase *IntuitionBase;
  	struct GfxBase *GfxBase;

	struct ViewPortList* view_port_list;

	int width, height; 
  	bool initiated;
};

int loop;
UWORD *pointer;
struct ViewPortList* viewporttraverse;
struct ViewPortList* freeviewport;

void CleanDrawImage();
void InitLibraries();
struct ViewPortList* CreateVoidPort();
void AddViewPort();
void OpenView();
void UpdateView();
void CleanUp();
UBYTE Joystick();

struct Screen_data global_screen;

void CleanDrawImage( struct RastPort* rastport, struct Image* image, int imagex, int imagey) {
	//BltClear(rastport->BitMap, RASSIZE(image->Width, image->Height), 0);
	DrawImage(rastport, image, imagex, imagey);
}

struct ViewPortList* CreateViewPort(  struct ViewPortList* newleaf, // pointer to the new ViewPortList we want to create
                                      int viewportheight,           // height of the new ViewPort
                                      int yposviewport,             // YPosition on the screen of the new ViewPort
                                      int depth,
                                      int number,                   // Order of ViewPort (determined by AddViewPort)
                                      UWORD* color_table ) {        // Array with colors to be copied to the new Viewport

  newleaf = malloc(sizeof(struct ViewPortList));
  if (newleaf == NULL)
    CleanUp( "Could not allocate memory for new ViewPort!" );


  newleaf->colors = 2;
  for (loop = 1; loop < depth; loop++ ) // pow(2, depth)
    newleaf->colors = newleaf->colors * 2;

  newleaf->depth = depth;

  InitVPort( &(newleaf->this_view_port) );

  newleaf->this_view_port.DWidth = global_screen.width;
  newleaf->this_view_port.DHeight = viewportheight;
  newleaf->this_view_port.DxOffset = 0;
  newleaf->this_view_port.DyOffset = yposviewport;
  newleaf->this_view_port.RasInfo = &(newleaf->ras_info);
  newleaf->this_view_port.Modes = NULL;    // TODO make flags variable

  newleaf->next = NULL;
  newleaf->yPos = yposviewport;

  // 3. Get a colour map, link it to the ViewPort, and prepare it:
  newleaf->this_view_port.ColorMap = (struct ColorMap *) GetColorMap( newleaf->colors );
  if( newleaf->this_view_port.ColorMap == NULL )
    CleanUp( "Could NOT get a ColorMap!" );

  // Get a pointer to the colour map:
  pointer = (UWORD *) newleaf->this_view_port.ColorMap->ColorTable;

  // Set the colours
  for( loop = 0; loop < newleaf->colors; loop++ )
    *pointer++ = color_table[ loop ];

  // Prepare the BitMap:
  InitBitMap( &(newleaf->bit_map), depth, global_screen.width, viewportheight );

  // Allocate memory for the Raster: 
  for( loop = 0; loop < depth; loop++ )
  {
    newleaf->bit_map.Planes[ loop ] = (PLANEPTR) AllocRaster( global_screen.width, viewportheight );

    if( newleaf->bit_map.Planes[ loop ] == NULL )
      CleanUp( "Could NOT allocate enough memory for the raster!" );

    // Clear the display memory with help of the Blitter:
    BltClear( newleaf->bit_map.Planes[ loop ], RASSIZE( global_screen.width, viewportheight ), 0 );
  }

  // Prepare the RasInfo structure: 
  newleaf->ras_info.BitMap = &(newleaf->bit_map); 
  newleaf->ras_info.RxOffset = 0;         
  newleaf->ras_info.RyOffset = 0;//yposviewport;                                           
  newleaf->ras_info.Next = NULL;   

  // Prepare the RastPort, and give it a pointer to the BitMap.
  InitRastPort( &(newleaf->rast_port) );
  newleaf->rast_port.BitMap = &(newleaf->bit_map);
                                   
  return newleaf;
}

void AddViewPort( int height, 
                  int ypos, 
                  int depth,
                  UWORD* color_table ) {

  if (global_screen.view_port_list == NULL) {
    global_screen.view_port_list = CreateViewPort( global_screen.view_port_list, height, ypos, depth, 0, color_table );
    // Main view port
    global_screen.view.ViewPort = &(global_screen.view_port_list->this_view_port);
  }
  else {
    struct ViewPortList* lastleaf = global_screen.view_port_list;
    loop = 1;

    while (lastleaf->next != NULL) {
      lastleaf = lastleaf->next;
      loop++;
    }
    
    lastleaf->next = CreateViewPort( global_screen.view_port_list, height, ypos, depth, loop, color_table);
    lastleaf->this_view_port.Next = &(lastleaf->next->this_view_port);
  }
}

void InitLibraries() {
  if ( global_screen.initiated == FALSE ) {
    /* Open the Intuition library: */
    global_screen.IntuitionBase = (struct IntuitionBase *)
    OpenLibrary( "intuition.library", 0 );
    if( !global_screen.IntuitionBase )
      CleanUp( "Could NOT open the Intuition library!" );

    /* Open the Graphics library: */
    global_screen.GfxBase = (struct GfxBase *)
    OpenLibrary( "graphics.library", 0 );
    if( !global_screen.GfxBase )
      CleanUp( "Could NOT open the Graphics library!" );

    global_screen.initiated = TRUE;
  }
}

void OpenView() {

  InitLibraries(); // Loads IntuitionBase and GfxBase if they are not loaded already

 	// Save the current View, so we can restore it later:
	global_screen.old_view = global_screen.GfxBase->ActiView;


 	// Prepare the View structure
	InitView( &(global_screen.view) );
}

void UpdateView() {

  viewporttraverse = global_screen.view_port_list;
  do {
    MakeVPort( &(global_screen.view), &(viewporttraverse->this_view_port) );
    viewporttraverse = viewporttraverse->next;
  } while (viewporttraverse != NULL);
	
	MrgCop( &(global_screen.view) );
	LoadView( &(global_screen.view) );

  WaitTOF();  // Wait for end of drawing; limits speed
}

void CleanUp( char* message ) {
  // Restore old view
  LoadView( global_screen.old_view );

  /* Free automatically allocated display structures: */
  viewporttraverse = global_screen.view_port_list;
  if (global_screen.view_port_list != NULL) {

    do {
      freeviewport = viewporttraverse;
      viewporttraverse = viewporttraverse->next;
      FreeVPortCopLists( &(freeviewport->this_view_port) );
      free(freeviewport);
    } while (viewporttraverse != NULL);
  
	  /* Deallocate the display memory, BitPlane for BitPlane: */
    viewporttraverse = global_screen.view_port_list;
    do {  
    	for( loop = 0; loop < viewporttraverse->depth; loop++ )
      	if( viewporttraverse->bit_map.Planes[ loop ] )
        	FreeRaster( viewporttraverse->bit_map.Planes[ loop ], global_screen.width, viewporttraverse->this_view_port.DHeight );

    	/* Deallocate the ColorMap: */
    	if( viewporttraverse->this_view_port.ColorMap ) FreeColorMap( viewporttraverse->this_view_port.ColorMap );
      
      viewporttraverse = viewporttraverse->next;
    } while (viewporttraverse != NULL);
  }

  FreeCprList( global_screen.view.LOFCprList );

  /* Close the Graphics library: */
  if( global_screen.GfxBase ) CloseLibrary( global_screen.GfxBase );

  /* Close the Intuition library: */
  if( global_screen.IntuitionBase ) CloseLibrary( global_screen.IntuitionBase );

  /* Print the message and leave: */
  printf( "%s\n", message ); 
  exit();
}

UBYTE Joystick( port )
UBYTE port;
{
  UBYTE data = 0;
  UWORD joy;
  
  if( port == PORT1 )
  {
    /* PORT 1 ("MOUSE PORT") */
    joy = custom.joy0dat;
    data += !( cia->ciapra & 0x0040 ) ? FIRE : 0;
  }
  else
  {
    /* PORT 2 ("JOYSTICK PORT") */
    joy = custom.joy1dat;
    data += !( cia->ciapra & 0x0080 ) ? FIRE : 0;
  }

  data += joy & 0x0002 ? RIGHT : 0;
  data += joy & 0x0200 ? LEFT : 0;
  data += (joy >> 1 ^ joy) & 0x0001 ? DOWN : 0;
  data += (joy >> 1 ^ joy) & 0x0100 ? UP : 0;

  return( data );
}
