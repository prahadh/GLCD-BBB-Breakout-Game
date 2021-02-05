
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>

#define SIG_BUTTON0 20
#define SIG_BUTTON1 21
#define SIG_BUTTON2 22
#define FRAME_UPDATE_TIME 0.03
#define PADDLE_DEBOUNCE_TIME 0.1
#define SCATTER_FRAMES 5

#define DEV "/dev/mylcd"

#define DISPLAY_OFF 0x3E
#define DISPLAY_ON  0x3F

#define LEFT_HALF  0x01
#define RIGHT_HALF 0x02

#define PAGE_0 0x00
#define PAGE_1 0x01
#define PAGE_2 0x02
#define PAGE_3 0x03
#define PAGE_4 0x04
#define PAGE_5 0x05
#define PAGE_6 0x06
#define PAGE_7 0x07

#define IOCTL_COMM 0
#define IOCTL_DATA 1
#define IOCTL_WIN  4

#define NO_BRICK -1

struct timeval curr_button_time,prev_button_time;
int init_screen = 1;
int game_over_screen = 0;

void mdelay(unsigned int );
void frame_clear(void); 							   
void glcd_col_sel(uint8_t );						   
void glcd_page_sel(uint8_t );						   
void display_frame(void);                             
void place_frame_obj(int ,int ,const uint8_t* ,int );  
void collision_detect(float* , float* , float* );      
void place_frame_image(int ,int ,int ,int ,uint8_t* ); 
void place_frame_bricks_init(void );                  
void update_frame_bricks(void );                       
void display_opening(void );						   
void display_closing(void );						   
void signal_handler0(int signum);					   
void signal_handler1(int signum);					   					
void signal_handler2(int signum);					   

static int brick_loc[2][56];
int pid;

uint8_t breakit_image[] = {0, 0, 240, 248, 248, 248, 248, 120, 120, 248, 248, 240, 208, 16, 248, 248, 248, 248, 248, 248, 
248, 248, 248, 248, 208, 16, 248, 248, 248, 248, 248, 248, 248, 248, 248, 16, 16, 16, 152, 248, 248, 248, 248, 248, 248, 
144, 16, 56, 248, 248, 248, 240, 144, 248, 248, 248, 248, 120, 240, 96, 0, 0, 0, 255, 239, 255, 255, 255, 255, 60, 60, 
255, 255, 255, 231, 0, 255, 255, 255, 255, 255, 252, 252, 255, 255, 159, 15, 3, 255, 255, 255, 255, 255, 60, 60, 60, 0,
0, 0, 240, 255, 255, 255, 255, 223, 255, 255, 255, 248, 192, 255, 255, 255, 255, 255, 255, 255, 247, 249, 230, 193, 0, 
0, 0, 56, 127, 207, 223, 223, 223, 223, 222, 222, 223, 207, 207, 199, 192, 207, 223, 223, 223, 207, 195, 207, 31, 31, 
31, 140, 128, 143, 159, 31, 223, 223, 223, 223, 223, 207, 192, 206, 31, 31, 31, 207, 195, 195, 195, 207, 223, 223, 223, 
223, 207, 223, 223, 223, 205, 199, 207, 223, 223, 223, 223, 78, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 112, 120, 253, 
255, 127, 63, 31, 255, 240, 224, 128, 0, 255, 255, 255, 255, 0, 1, 1, 255, 255, 255, 255, 1, 1, 0, 240, 248, 124, 127, 
255, 255, 255, 255, 114, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 3, 7, 15, 30, 24, 48, 55, 63, 63, 63, 48, 48, 63, 63, 63, 63, 60, 28, 28, 3, 3, 0, 0, 0, 1, 3, 3, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t start_image[] = {63, 127, 9, 15, 7, 0, 63, 127, 9, 127, 127, 0, 63, 127, 105, 105, 97, 0, 34, 103, 109, 123, 50,
0, 38, 103, 125, 123, 50, 0, 70, 222, 186, 246, 102, 0, 2, 2, 254, 254, 2, 0, 252, 254, 54, 54, 254, 252, 0, 254, 254,
50, 254, 238, 0, 2, 254, 254, 254, 2}; 

uint8_t game_over_image[] = {0, 0, 240, 248, 248, 56, 24, 24, 24, 24, 24, 24,248, 248, 248, 0, 0, 0, 0, 0, 0, 0, 248, 248,
224, 0, 0, 0, 0, 0, 0, 248, 248, 192, 0, 0, 0, 0, 0, 240, 248, 240, 0, 0,0, 248, 248, 24, 24, 24, 24, 16, 0, 0, 0, 0, 0, 
0, 0, 0, 240, 248, 24, 24, 24, 24, 184, 248, 0, 0, 0, 248, 248, 0, 0, 0,0, 240, 248, 0, 0, 0, 248, 248, 24, 24, 24, 24, 
16, 0, 0, 248, 248, 248, 56, 56, 56, 56, 56, 56, 56, 248, 248, 240, 0,0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 7, 
31, 15, 0, 0, 0, 0, 0, 224, 255, 7, 1, 127, 252, 128, 0, 0, 0, 0,255, 255, 31, 252, 224, 0, 240, 127, 15, 255, 255, 0, 0, 
0, 255, 255, 112, 96, 96, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255,255, 0, 0, 0, 0, 255, 255, 0, 0, 0, 3, 255, 255, 0, 0, 254, 
255, 3, 0, 0, 0, 255, 255, 96, 96, 96, 96, 0, 0, 0, 255,255, 255, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 255, 
255, 255, 0, 0, 0, 0, 28, 28, 28, 252, 252, 252, 0, 0,0, 128, 252, 63, 13, 12, 12, 12, 15, 255, 224, 0, 0, 0, 255, 255, 
0, 1, 15, 31, 7, 0, 0, 255, 255, 0, 0, 0, 255, 255,192, 192, 192, 192, 128, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 128, 192, 
192, 192, 223, 255, 0, 0, 0, 0, 0, 63, 255, 255,63, 0, 0, 0, 0, 0, 255, 255, 192, 192, 192, 192, 128, 0, 0, 255, 255, 255, 
0, 0, 14, 126, 254, 254, 206, 15, 7, 7, 0, 0,0, 0, 0, 255, 255, 255, 128, 128, 128, 128, 128, 128, 128, 255, 255, 255, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 224,224, 96, 0, 224, 224, 96, 96, 224, 192, 0, 224, 128, 0, 224, 32, 0, 0, 0, 0, 128, 
224, 224, 0, 0, 192, 224, 96, 96, 224,224, 0, 0, 224, 224, 192, 0, 0, 224, 0, 224, 224, 128, 0, 224, 224, 0, 224, 96, 32, 
224, 224, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 255, 255, 255, 0, 0, 0, 0, 1, 15, 127, 254, 240, 128, 0, 0, 0, 0, 0, 1, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 0, 0, 127, 127, 4, 30, 119, 67, 0, 1, 127, 
127, 1, 0, 0, 0, 0, 96, 127, 25,31, 127, 96, 63, 127, 64, 68, 124, 124, 0, 124, 31, 17, 127, 112, 0, 127, 0, 127, 127, 7, 
62, 127, 127, 0, 0, 76, 79, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 7, 3, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 0, 0, 0};

const uint8_t brick_scatter_obj[8] = {0x08,0x01,0x10,0x04,0x21,0x00,0x22,0x08};
/*
    The frame contains 128x64 pixel data to be displayed periodically onto the GLCD
    The first parameter 128 (right most) corresponds to 128 columns of GLCD
    The second parameter 8 (left most) corresponds to 8 pages of GLCD
*/
uint8_t frame[8][128];

/*
    The brick object brick_obj contains data as to be written into GLCD so to display
    brick at location (0,0). Brick's center point being (4.5,2.5)
    Brick has a size of 8x4 pixels
*/

const uint8_t brick_obj[8]   = {0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F};
const int brick_width    = 4;
const int brick_length   = 8;
const float brick_centre_x = 4.5; // Actual center x value is 4.5
const float brick_centre_y = 2.5; // Actual center y value is 2.5
/*
    The ball object ball_obj contains data as to be written into GLCD so to display
    ball at location (0,0). Ball's center point being (2.5,2.5)
    Ball has a size of 4x4 pixels
*/

const uint8_t ball_obj[4]    = {0x1E,0x3F,0x3F,0x1E};

const int ball_width    = 6;
const int ball_length   = 4;
const float ball_centre_x = 2.5; // Actual center x value is 2.5
const float ball_centre_y = 3.5; // Actual center y value is 3.5


/*
    The paddle object paddle_obj contains data as to be written into GLCD so to display
    paddle at location (0,0). Paddle's center point being (9.5,1.5)
    paddle has a size of 16x2 pixels
*/

const uint8_t paddle_obj[16] = {0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03};

const int paddle_width    = 2;
const int paddle_length   = 16;
const float paddle_centre_x = 8.5; // Actual center x value is 8.5
const float paddle_centre_y = 1.5; // Actual center y value is 1.5

// Declare paddle position variables
float paddle_pos_x = 63;
float paddle_pos_y = 60;

int fd = -1;


int main(void)

{

  // initialize positions of ball and its directions
  float ball_pos_x = 63;
  float ball_pos_y = 50;

  /*
    The ball_dir array stores direction of ball as vector values.
    The first value is in i direction and second value in j direction.
  */
  float ball_dir[2] = {0.707,-0.707};

  // The variables are used to update ball position
  struct timeval curr_ball_time,prev_ball_time;
  prev_button_time.tv_usec = 0;
  prev_button_time.tv_sec = 0;
  prev_ball_time.tv_usec = 0;
  prev_ball_time.tv_sec = 0;

	// Initialize signals and its handlers for GPIO buttons 0, 1 and 2
	signal(SIG_BUTTON0,signal_handler0);
 	signal(SIG_BUTTON1,signal_handler1);
 	signal(SIG_BUTTON2,signal_handler2);

    // Open the file to initialize GPIO and GLCD hardwares
 	fd = open(DEV, O_RDWR);
    if(fd < 0)
    {	
      perror(DEV);
      exit(1);
    }

     // Display opening screen and disable special button interrupts
     display_opening();

     // Initiate insertion all bricks into the frame
     place_frame_bricks_init();

     /*
        The actual game is in this while loop. It is responsible for clearing the frame, updating ball position by
        ball's velocity and then placing paddle, unbroken bricks, new ball position and displaying frame.
        It also detects collision between ball and wall/paddle/brick and updates ball's behavior accordingly
        The frame updates atleast every 30ms by default
      */
    frame_clear();
    display_frame();

    while(1)
    {
       gettimeofday(&curr_ball_time, NULL);
       if(
           ( (double )curr_ball_time.tv_sec + (double )curr_ball_time.tv_usec/1000000 ) - 
           ( (double )prev_ball_time.tv_sec + (double )prev_ball_time.tv_usec/1000000 ) > FRAME_UPDATE_TIME ) 
       {
          prev_ball_time = curr_ball_time;

          ball_pos_x= ball_pos_x + ball_dir[0];
          ball_pos_y= ball_pos_y + ball_dir[1];
       }
          place_frame_obj((int )(paddle_pos_x - paddle_centre_x),(int )(paddle_pos_y - paddle_centre_y),paddle_obj,16);
          place_frame_obj((int )(ball_pos_x - ball_centre_x),(int )(ball_pos_y - ball_centre_y),ball_obj,4);

          update_frame_bricks();

          display_frame();
          frame_clear();

          collision_detect(&ball_pos_x, &ball_pos_y, ball_dir);
    }

}

/*
 collision_detect function is used to detect collision between ball and brick/paddle.
 Send the central coordinates of ball and paddle/brick as argument (Do no send the
 adjusted center adjusted coordinates).
 It uses relative distance measurements to detect collision and ball's direction is
 changed according to where it strikes the paddle/brick by using relative position between
 ball's center and paddle's/brick's center.
 floating point is used for accurate measurements.
*/
void collision_detect(float *ball_x, float *ball_y, float *ball_dir)
{
    // Calculate relative distance of paddle with respect to ball in x and y coordinates
    float abs_paddle_ball_y = fabs((*ball_y) - paddle_pos_y);
    float abs_paddle_ball_x = fabs((*ball_x) - paddle_pos_x);

    /*
      Check if ball collided with the wall and invert its y or x component
      accordingly
     */
    if( (*ball_x) > 125 )
    {
      ball_dir[0] = -fabs(ball_dir[0]);
    }

    if( (*ball_x) < 3 )
    {
      ball_dir[0] = fabs(ball_dir[0]);
    }

    if( (*ball_y) < 4 )
    {
      ball_dir[1] = fabs(ball_dir[1]);
    }

    /*
     If ball goes down below the screen, display game over screen.
     Re-initialize positions of paddle, ball and bricks. The direction
     of ball is also re-initialized.
     Enable special button interrupt and wait for special button to
     be pressed and restart game.
    */
    if( (*ball_y) > 65)
    {
        *ball_x = 63;
        *ball_y = 50;
        paddle_pos_y = 60;
        paddle_pos_x = 63;
        ball_dir[0] = 0.707;
        ball_dir[1] = -0.707;
        place_frame_bricks_init();
        /*
           display_closing is a blocking function which displays game over till special
           button is pressed
        */
        display_closing();
        return;
    }

    // Check if ball is touching the pad
    if( ( abs_paddle_ball_y<= 5 ) && ( abs_paddle_ball_x <= 11) )
    {
      /*
        If touching,
        the direction of ball is made parallel to the vector joining the paddle center to the ball
        center. The magnitude the vector is made unit after it is found
      */
      ball_dir[1] = ((*ball_y) - paddle_pos_y)/sqrt( abs_paddle_ball_y*abs_paddle_ball_y + abs_paddle_ball_x*abs_paddle_ball_x );
      ball_dir[0] = ((*ball_x) - paddle_pos_x)/sqrt( abs_paddle_ball_y*abs_paddle_ball_y + abs_paddle_ball_x*abs_paddle_ball_x );
    }


    float abs_brick_ball_y;
    float abs_brick_ball_x;
    static int scatter = 0;
    static int scatter_brick_pos_x, scatter_brick_pos_y;

    // Check if the ball is touching any of the brick.
    for(int block_iter = 0; block_iter < 56; block_iter++)
    {
        abs_brick_ball_y = fabs((*ball_y) - brick_loc[1][block_iter]);
        abs_brick_ball_x = fabs((*ball_x) - brick_loc[0][block_iter]);

        if( ( abs_brick_ball_y<= 6 ) && ( abs_brick_ball_x <= 7 ) && (brick_loc[1][block_iter]!= NO_BRICK) && (brick_loc[0][block_iter]!= NO_BRICK))
        {
            /*
                If touching this brick,
                the direction of ball is made parallel to the vector joining the brick center to the ball
                center. The magnitude the vector is made unit after it is found.
             */

            ball_dir[1] = ((*ball_y) - brick_loc[1][block_iter])/sqrt( abs_brick_ball_y*abs_brick_ball_y + abs_brick_ball_x*abs_brick_ball_x );
            ball_dir[0] = ((*ball_x) - brick_loc[0][block_iter])/sqrt( abs_brick_ball_y*abs_brick_ball_y + abs_brick_ball_x*abs_brick_ball_x );

            // The broken brick position is stored along with number of frames to show scatter object
            scatter = SCATTER_FRAMES;
            scatter_brick_pos_x = brick_loc[0][block_iter];
            scatter_brick_pos_y = brick_loc[1][block_iter];

            /*
              Make the brick disappear by inserting NO_BRICK value in its coordinates.
              Only one brick is made to vanish at a time. Hence break away from the for
              loop once touched.update number of times scatter to be displayed.
            */
            brick_loc[0][block_iter] = NO_BRICK;
            brick_loc[1][block_iter] = NO_BRICK;
            break;
        }

    }

    // If collision happens, place brick scatter below the broken brick for given number of frames (default is 5) as mentioned in for loop
    if(scatter > 0)
    {
        place_frame_obj((int )(scatter_brick_pos_x - brick_centre_x),(int )(scatter_brick_pos_y - brick_centre_y + 3),brick_scatter_obj,8);
        scatter--;
    }

}

/*
   place_frame_bricks_init function inserts bricks into frame
   Presently its fixed at 56 in number i.e 4 rows and 14
   columns of bricks spaced apart by 5 horizontally and
   apart by 9 vertically
 */
void place_frame_bricks_init(void )
{
    int brick_no = 0;
    for(int place_x = 6; place_x < 127 ; place_x+=9)
    {
        for(int place_y = 15; place_y < 31 ; place_y+=5)
        {
            brick_loc[0][brick_no] = place_x;
            brick_loc[1][brick_no] = place_y;
            brick_no++;
        }
    }
}

/*
   update_frame_bricks function displays only bricks which are still
   unbroken/untouched by ball by checking and displaying only those
   bricks which do not have NO_BRICK value.
 */
void update_frame_bricks(void )
{
    for(int block_iter = 0; block_iter < 56 ; block_iter++)
    {
            if( (brick_loc[0][block_iter] != NO_BRICK) && (brick_loc[1][block_iter] != NO_BRICK) )
            {
                place_frame_obj(brick_loc[0][block_iter] - (int )brick_centre_x,brick_loc[1][block_iter] - (int )brick_centre_y,brick_obj,8);
            }
    }
}

/*
   display_opening function displays the frame when the
   MCU is powered-on/reset. Displays BREAK IT logo with
   press start blinking.
   Must enable special button interrupt (GPIO PORT C 0x20 is default)
   before calling this function.
 */
void display_opening(void )
{
    for(int press_start_display = 1;init_screen == 1;)
    {
        frame_clear();
        place_frame_image(63,20,62,40,breakit_image);
        if(press_start_display == 1)
        {
            place_frame_image(63,50,30,16,start_image);
        }
        display_frame();
        mdelay(400);
        press_start_display = ~press_start_display;
    }

}

// Display game over frame
void display_closing(void )
{
    game_over_screen = 1;
    for(int game_over_display = 1; game_over_screen == 1 ;)
    {
        frame_clear();
        if(game_over_display == 1)
        {
        place_frame_image(63,33,107,40,game_over_image);
        }
        display_frame();
        mdelay(400);
        game_over_display = ~game_over_display;
    }
}


/*
    frame_clear function must be used before using the frame for any other purpose
    It fills the frame with zeros
*/
void frame_clear(void)
{
    for(int i = 0; i < 128; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            frame[j][i] = 0x00;
        }
    }
}

/*
    glcd_page_sel function used to select one of the 8 rows (called pages)
    page argument ranges from 0 to 7
    It utilizes ioctl call
*/
void glcd_page_sel(uint8_t page)
{
    ioctl(fd,IOCTL_COMM,0xB8|page);
}

/*
    glcd_col_sel function used to select one of the 64 columns, each with 8 bit data
    col argument ranges from 0 to 63
    It utilizes ioctl call
*/
void glcd_col_sel(uint8_t col)
{
	ioctl(fd,IOCTL_COMM,0x40|col);
}

// mdelay uses usleep function to delay by given number of milliseconds
void mdelay(unsigned int n)
{
 usleep(n * 1000);
}

// display_frame function displays image data from frame variable onto GLCD   CHANGED !!!!

void display_frame(void)
{
  // y variable points to GLCD page, x variable points to column of GLCD
    for(int y = 0; y <8 ; y++)
    {
        for(int x =0; x<128 ; x++)
        {
            /*
              At start of left window, 
              Select left window, select page from given y variable
              and shift column to the start of window.
            */
            if(x == 0)
            {
                ioctl(fd,IOCTL_WIN,LEFT_HALF);
                glcd_page_sel(y);
                glcd_col_sel(0);
                ioctl(fd,IOCTL_COMM,DISPLAY_ON);
            }
            /*
              At start of right window, 
              Select right window, select page from given y variable
              and shift column to the start of window.
            */
            else if(x == 64)
            {
                ioctl(fd,IOCTL_WIN,RIGHT_HALF);
                glcd_page_sel(y);
                glcd_col_sel(0);
            }
            ioctl(fd,IOCTL_DATA,(unsigned long ) frame[y][x]);
        }
    }
  // Display the frame on GLCD only after every bit of frame has been written to GLCD RAM
  ioctl(fd,IOCTL_COMM,DISPLAY_ON);
}

/*
   place_frame_obj function places given array of pixel data into frame depending on given
   coordinates (x,y)
*/

void place_frame_obj(int x, int y,const uint8_t* pix_data, int pix_data_len)
{
    int frame_col          = (x & 0x7F);               // column information is present in first 7 bits of x
    int frame_page         = ((y >> 3) & 0x07);        // row(page) of frame information is present in 5-3 bits of y
    uint8_t pixel_shift    = (uint8_t )(y & 0x07);     // The first 3 bits of y contains information about how many pixels to shift down

    // All the data to be placed in frame are ORed, to merge objects in same field.
    for(uint8_t iter = 0;iter < pix_data_len; iter++)
    {
        /*
            Shift each pixel data to the left (or downwards with respect to GLCD) by value pixel_shift and input inside 
            frame in the location mentioned in frame_page and frame_col
        */
        frame[frame_page][frame_col+iter] |= (*(pix_data+iter) << pixel_shift);
        /*
            The other part of the pixel data which was shifted out before in previous statement is loaded into the next 
            page by incrementing page and right shifting the pixel data by (8 - pixel_shift).
            Boundary checking is also done to ensure incremented page is within frame of GLCD, if not then do not load.
        */
        if(( pixel_shift > 0 ) && ( ( frame_page + 1) < 8) )
            frame[frame_page + 1][frame_col+iter] |= (*(pix_data+iter) >> (8-pixel_shift));
    }

}

/*
   place_frame_image function utilizes place_frome_obj function, the position coordinates (x,y)  
   and width & height of image to place image onto frame variable
*/

void place_frame_image(int x,int y,int img_width,int img_height,uint8_t* lin_image)
{
    int img_mid_x = (img_width+1)/2;
    int img_mid_y = (img_height+1)/2;
    for(int i = 0; i<(img_height/8);i++)
    {
        place_frame_obj(x - img_mid_x,y - img_mid_y + i*8,(lin_image + i*img_width),img_width);
    }
}

// signal_handler0 function increments paddle position
void signal_handler0(int signum)
{
	gettimeofday(&curr_button_time, NULL);
	if(
       ( (double )curr_button_time.tv_sec + (double )curr_button_time.tv_usec/1000000 ) - 
       ( (double )prev_button_time.tv_sec + (double )prev_button_time.tv_usec/1000000 ) > PADDLE_DEBOUNCE_TIME )
    { 
    	prev_button_time = curr_button_time;
		if( (paddle_pos_x< 118) )
    		paddle_pos_x+=7;
    }
	
}

// signal_handler1 function controls start and game over screens

void signal_handler1(int signum)
{
	gettimeofday(&curr_button_time, NULL);
	if(
       ( (double )curr_button_time.tv_sec + (double )curr_button_time.tv_usec/1000000 ) - 
       ( (double )prev_button_time.tv_sec + (double )prev_button_time.tv_usec/1000000 ) > PADDLE_DEBOUNCE_TIME )
    { 
    	prev_button_time = curr_button_time;
		if(init_screen)
        	init_screen = 0;
    	if(game_over_screen)
        	game_over_screen = 0;
    }

}

// signal_handler2 function decrements paddle position

void signal_handler2(int signum)
{
	gettimeofday(&curr_button_time, NULL);
	if(
       ( (double )curr_button_time.tv_sec + (double )curr_button_time.tv_usec/1000000 ) - 
       ( (double )prev_button_time.tv_sec + (double )prev_button_time.tv_usec/1000000 ) > PADDLE_DEBOUNCE_TIME )
    { 
    	prev_button_time = curr_button_time;
		if( ( paddle_pos_x> 15) )
			paddle_pos_x-=7;
	}
}
