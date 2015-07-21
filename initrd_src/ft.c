/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
//#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H


#define WIDTH   80
#define HEIGHT  24


/* origin is the upper left corner */
unsigned char image[HEIGHT][WIDTH];


/* Replace this function with something useful. */

void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;


  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;

      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}


void
show_image( void )
{
  int  i, j;


  for ( i = 0; i < HEIGHT; i++ )
  {
    for ( j = 0; j < WIDTH; j++ )
      putchar( image[i][j] == 0 ? ' '
                                : image[i][j] < 128 ? '+'
                                                    : '*' );
    putchar( '\n' );
  }
}


int
main( int     argc,
      char**  argv )
{
  FT_Library    library;
  FT_Face       face;

  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;

  char*         filename;
  char*         text;

  double        angle;
  int           target_height;
  int           n, num_chars;


  if ( argc != 2 )
  {
    fprintf ( stderr, "usage: %s sample-text\n", argv[0] );
    exit( 1 );
  }

  filename      = "/ubm.ttf";                           /* first argument     */
  text          = argv[1];                           /* second argument    */
  num_chars     = strlen( text );
  angle         = 0;      /* use 25 degrees     */
  target_height = HEIGHT;

  error = FT_Init_FreeType( &library );              /* initialize library */
  fprintf(stderr, "error %X\n", error);
  /* error handling omitted */
	
	int fd = open(filename, 0);
	int sz = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	void *buf = malloc(sz + 20);
	read(fd, buf, 100);

	error = FT_New_Memory_Face(library, buf, sz, 0, &face); /* create face object */
	fprintf(stderr, "error %X\n", error);

  /* use 50pt at 100dpi */
  error = FT_Set_Char_Size( face, 50 * 64, 0,
                            50, 0 );                /* set character size */
  fprintf(stderr, "set\n");
  /* error handling omitted */

  slot = face->glyph;

  /* set up matrix */
  matrix.xx = (FT_Fixed)( 1 * 0x10000L );
  matrix.xy = (FT_Fixed)(-0 * 0x10000L );
  matrix.yx = (FT_Fixed)( 0 * 0x10000L );
  matrix.yy = (FT_Fixed)( 1 * 0x10000L );

  /* the pen position in 26.6 cartesian space coordinates; */
  /* start at (300,200) relative to the upper left corner  */
  pen.x = 0;
  pen.y = ( target_height - 24 ) * 64;

fprintf(stderr, "Get Ready\n");
  for ( n = 0; n < num_chars; n++ )
  {
    /* set transformation */
    FT_Set_Transform( face, &matrix, &pen );
    fprintf(stderr, "Trans\n");

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, text[n], FT_LOAD_RENDER );
    fprintf(stderr, "Load\n");
    if ( error )
      continue;                 /* ignore errors */

    /* now, draw to our target surface (convert position) */
    draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );
     fprintf(stderr, "draw\n");

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }

  show_image();

  FT_Done_Face    ( face );
  FT_Done_FreeType( library );

  return 0;
}

/* EOF */
