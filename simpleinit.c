#include "happy_amiga_lib.h"
#include "character.h"
#include "tree.h"

#define WIDTH  320 /* 320 pixels wide (low resolution)             */
#define HEIGHT 200 /* 200 lines high (non interlaced NTSC display) */ 
#define DEPTH 3

UWORD color_table_1[] =
{
	0x05BC,0xF00, 0xF0D, 0xF0B,
	0x0F0, 0x0F0, 0x0F0, 0x0F0,
};
UWORD color_table_2[] = 
{
	0x0AAA,0x0AAA,0x0AAA,0x0AAA,
	0x0000,0x0000,0x0000,0x0000,
};
UWORD color_table_3[] = 
{
	0x00A4,0x0AAA,0x0AAA,0x0AAA,
	0x0000,0x0000,0x0000,0x0000,
};

bool running = TRUE;
int x,y = 0;
UBYTE value = 0;
UBYTE old_value = 0;
struct Image* dynamic;

void main() {
	// Prepare to open screen
	global_screen.width = WIDTH;
	global_screen.height = HEIGHT;
	
	// Open screen
	OpenView();

	// Create ViewPorts in the View
	AddViewPort(30, 0, 3, color_table_1);
	AddViewPort(100, 31, 3, treePaletteRGB4);
	AddViewPort(30, 132, 1, color_table_3);

	dynamic = LoadPBM("tree.dat");
	printf("image %d,%d,%d,%04X,%04X,%04X,%d,%d,%04X,%04X\n", dynamic->Width, dynamic->Height, dynamic->Depth,  dynamic->ImageData[2], dynamic->ImageData[98], dynamic->ImageData[623], dynamic->LeftEdge, dynamic->TopEdge, dynamic->PlanePick, dynamic->PlaneOnOff);
	printf("tree  %d,%d,%d,%04X,%04X,%04X,%d,%d,%04X,%04X\n", tree.Width, tree.Height, tree.Depth,  tree.ImageData[2], tree.ImageData[98], tree.ImageData[623], tree.LeftEdge, tree.TopEdge, tree.PlanePick, tree.PlaneOnOff);
	
	DrawImage(&(global_screen.view_port_list->next->rast_port), &tree, x, y);
	//CleanDrawImage(&(global_screen.view_port_list->next->rast_port), dynamic, x, y);
	// Main loop
	//for( loop = 0; loop < 600; loop++ ) {
	while (running) {
		old_value = value;
		value = Joystick( PORT2 );

  		if( value != old_value )
	    {
	      if( value & FIRE )
	      	running = FALSE;
	      if( value & RIGHT )
	        x += 4;
	      if( value & LEFT )
	        x -= 4;
	      if( value & DOWN )
	        y += 4;
	      if( value & UP )
	        y -= 4;
	    }
		ScrollRaster(&(global_screen.view_port_list->next->rast_port), x, y, 0, 0, 52, 58);

		UpdateView();
	}

	// Debug Output

	// Exit program
	CleanUp("THE END");
}
