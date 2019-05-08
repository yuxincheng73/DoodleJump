#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

volatile int pixel_buffer_start; // global variable
int blob_x;
int blob_y;

void disable_A9_interrupts (void);
void set_A9_IRQ_stack (void);
void config_GIC (void);
void config_KEYs (void);
void enable_A9_interrupts (void);
void pushbutton_ISR (void);
void config_interrupt (int, int);

void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void swap(int *xp, int *yp) ;
void draw_box( int x, int y, int color);
void draw_line(int x0, int x1, int y0, int y1, short int line_color);
void draw_blob(int x, int y, int color);
void draw_platforms(int x[], int y[]);
int move_blob(int* blob_x, int* blob_y, int count, int decr);
void move_blob_left(int *blob_x, int *blob_y);
void move_blob_right(int *blob_x, int *blob_y);
void move_blob_right_more(int *blob_x, int *blob_y);
void move_blob_left_more(int *blob_x, int *blob_y);
void draw_score(int score);
void draw_digits(int digit, int offset);
void clear_screen();

int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;

    //next gen stuff 
    disable_A9_interrupts (); // disable interrupts in the A9 processor
    set_A9_IRQ_stack (); // initialize the stack pointer for IRQ mode
    config_GIC (); // configure the general interrupt controller
    config_KEYs (); // configure KEYs to generate interrupts
    enable_A9_interrupts (); // enable interrupts in the A9 processor
    //while (1) // wait for an interrupt
        //;
    
    
   
    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
    // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	
    bool lose = false;
    int score = 0;
    int a[25], b[25];
    int count, decr, temp, pos_y;
    int init_pos_y, distance_jumped;
    // initialize location and direction of rectangles(not shown)
    int i, j;
    
    //randomize the platforms initially
    for(i = 0; i < 20; i++) {
        //don't draw off the screen
        a[i] = rand()%278;
        b[i] = rand()%210;
    }
            
    while(1) {
        
        if(lose) {
            //reset score if you lose
            score = 0;
        }
        
        temp = 0;
        //check for lowest platform
        for(i = 0; i < 20; i++) {
            if(b[i] >= temp) {
                temp = b[i];
                pos_y = i;
            }
        }
        
        //saves initial location of blob
        blob_x = a[pos_y];
        blob_y = b[pos_y];
        count = 0;
        decr = 3;
        int offset = 0;
        lose = false;

        while (!lose)
        {
            /* Erase any boxes and lines that were drawn in the last iteration */
            clear_screen();

            // code for drawing the boxes and lines (not shown)
            draw_platforms(a, b);
            
            // draw score
            draw_score(score);
            
            
            //NEED TO WRITE THIS IN A FUNCTION AFTER KEY0 CLICKED
            if(count <= 50) {
                
                //save blob_y position at the platform it bounced on
                if(count == 0) {
                    init_pos_y = blob_y - offset;
                }
                //blob jumps a max of 10
                count = move_blob(&blob_x, &blob_y, count, decr);
                if(count == 5) {
                     decr=2;
                }
                if(count == 35) {
                     decr=1;
                }
                if(count == 50) {
                     decr=-1;
                }
                draw_blob(blob_x + 14,blob_y - 15, 0x13220);
            }
            else if(count > 50 && count <= 150) {
 
                for(i = 0; i < 20; i++) {
                    //check if same y as platform
                    if(((blob_x + 23) >= a[i]) && (blob_x + 14 <= (a[i] + 40))) {

                        if(blob_y == b[i]) {
                            offset = 0;
                            //stop decrementing
                            decr = 0;
                            //count resets to 0 if button is pressed
                            count = 0;
                            //save final position of blob_y at new platform
                            distance_jumped = (init_pos_y - blob_y)/5;
                            
                            //reloads platforms
                            for(i = 0; i < 20; i++) {
                                //if the platform shifted down is within the vertical range, do it
                                if((b[i] + distance_jumped) < 210) {
                                    b[i] += distance_jumped;
                                }
                                else {
                                    //else replace it with one on the top
                                    a[i] = rand() % 280;
                                    b[i] = rand() % distance_jumped;
                                }
                            }
                        }
                        else if(blob_y + 1 == b[i]) {
                            offset = 1;
                            //stop decrementing
                            decr = 0;
                            //count resets to 0 if button is pressed
                            count = 0;
                            distance_jumped = (init_pos_y - blob_y)/5;
                            //if shift occurred due to passing top, don't calculate distance_jumped
                            if(distance_jumped < 0) {
                                distance_jumped = 0;
                            }
                            
                            //reloads platforms
                            for(i = 0; i < 20; i++) {
                                //if the platform shifted down is within the vertical range, do it
                                if((b[i] + distance_jumped) < 210) {
                                    b[i] += distance_jumped;
                                }
                                else {
                                    //else replace it with one on the top
                                    a[i] = rand() % 280;
                                    b[i] = rand() % distance_jumped;
                                }
                            }
                        }
                        else if(blob_y + 2 == b[i]) {
                            offset = 2;
                            //stop decrementing
                            decr = 0;
                            //count resets to 0 if button is pressed
                            count = 0;
                            distance_jumped = (init_pos_y - blob_y)/5;
                            
                            //reloads platforms
                            for(i = 0; i < 20; i++) {
                                //if the platform shifted down is within the vertical range, do it
                                if((b[i] + distance_jumped) < 210) {
                                    b[i] += distance_jumped;
                                }
                                else {
                                    //else replace it with one on the top
                                    a[i] = rand() % 280;
                                    b[i] = rand() % distance_jumped;
                                }
                            }
                        }
                    }
                }
                count = move_blob(&blob_x, &blob_y, count, decr);
                if(count == 65) {
                     decr=-2;
                }
                if(count == 95) {
                     decr=-3;
                }
                //stop by checking every time if it has touched a platform
                
                draw_blob(blob_x + 14,blob_y - 15 - offset, 0x13220);
            }
            //if the blob falls through the bottom
            if(blob_y > 239) {
                lose = true;
            }
            else if(blob_y < 20) {
                //passes the top
                distance_jumped = 180;
                blob_y += 180;
                for(i = 0; i < 20; i++) {
                    //if the platform shifted down is within the vertical range, do it
                    if((b[i] + distance_jumped) < 210) {
                        b[i] += distance_jumped;
                    }
                    else {
                        //else replace it with one on the top
                        a[i] = rand() % 280;
                        b[i] = rand() % distance_jumped;
                    }
                }
                score++;
            }
            wait_for_vsync(); // swap front and back buffers on VGA vertical sync
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        }
    }
}

// code for subroutines (not shown)
void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    register int status;
    *pixel_ctrl_ptr = 1; //start the synchronization process
    status = *(pixel_ctrl_ptr + 3);
    while((status & 0x01 ) != 0){
        status = *(pixel_ctrl_ptr + 3);
    }
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

int move_blob(int *blob_x, int *blob_y, int count, int decr) {
	*blob_y -= decr;
	//plot_pixel(*blob_x, *blob_y, 0xFF0000);
	count++;
	return count;
}

void move_blob_left(int *blob_x, int *blob_y) {
	*blob_x -= 3;
	return;
}

void move_blob_right(int *blob_x, int *blob_y) {
	*blob_x += 3;
	return;
}

void move_blob_left_more(int *blob_x, int *blob_y) {
	*blob_x -= 8;
	return;
}

void move_blob_right_more(int *blob_x, int *blob_y) {
	*blob_x += 8;
	return;
}

//code for drawing the blob
void draw_blob(int x, int y, int color) {
    //draw 3x3 square
    //    plot_pixel(x, y,color);
    //    plot_pixel(x+1, y, color);
    //    plot_pixel(x+2, y, color);
    //    plot_pixel(x, y+1,color);
    //    plot_pixel(x, y+2, color);
    //    plot_pixel(x+1, y+1, color);
    //    plot_pixel(x+2, y+2, color);
    //    plot_pixel(x+2, y+1, color);
    //    plot_pixel(x+1, y+2, color);
    
    //draw the semicircle part
    plot_pixel(x+4, y, color);
    plot_pixel(x+5, y, color);
    plot_pixel(x+2, y+1, color);
    plot_pixel(x+3, y+1, color);
    plot_pixel(x+6, y+1, color);
    plot_pixel(x+7, y+1, color);
    plot_pixel(x, y+2, color);
    plot_pixel(x+1, y+2, color);
    plot_pixel(x+8, y+2, color);
    plot_pixel(x+9, y+2, color);
    
    int i;
    int j;
    for(i = 0; i < 10; i++) {
        for(j = 3; j < 13; j++) {
            plot_pixel(x + i, y + j, color);
        }
    }
    
	//draw legs
    plot_pixel(x, y+13, color);
    plot_pixel(x+1, y+13, color);
    plot_pixel(x+8, y+13, color);
    plot_pixel(x+9, y+13, color);
    plot_pixel(x, y+14, color);
    plot_pixel(x+1, y+14, color);
    plot_pixel(x+8, y+14, color);
    plot_pixel(x+9, y+14, color);
	
	//draw beak
	plot_pixel(x+9, y+3, color);
	plot_pixel(x+10, y+3, color);
	plot_pixel(x+11, y+3, color);
	plot_pixel(x+12, y+3, color);
	plot_pixel(x+12, y+3, color);
	plot_pixel(x+12, y+3, color);
	plot_pixel(x+9, y+4, color);
	plot_pixel(x+10, y+4, color);
	plot_pixel(x+11, y+4, color);
	plot_pixel(x+12, y+4, color);
	plot_pixel(x+12, y+4, color);
	plot_pixel(x+12, y+4, color);
	
}

//draw the platforms
void draw_platforms(int x[], int y[]) {
    int i;
    short int platform_color = 0x00FF00;
    
    //draw platforms
    for(i = 0; i < 20; i++) {
        draw_line(x[i], x[i] + 40, y[i], y[i], platform_color);
        draw_line(x[i], x[i] + 40, y[i] + 1, y[i] + 1, platform_color);
        draw_line(x[i], x[i] + 40, y[i] + 2, y[i] + 2, platform_color);
    }
}

void draw_line(int x0, int x1, int y0, int y1, short int line_color){
    bool steep = abs(y1-y0)>abs(x1-x0);
    if(steep){
        swap(&x0,&y0);
        swap(&x1,&y1);
    }
    
    if(x0>x1){
        swap(&x0,&x1);
        swap(&y0,&y1);
    }
    
    int del_x = x1 - x0;
    int del_y = abs(y1 -y0);
    int error = -(del_x /2);
    
    int y = y0;
    int x;
    
    int y_step;
    
    if(y0 < y1){
        y_step = 1;
    }
    
    else{
        y_step = -1;
    }
    
    for(x = x0; x <= x1; ++x){
        if(steep){
            plot_pixel(y,x,line_color);
        }
        else {
            plot_pixel(x,y,line_color);
        }
        error = error + del_y;
        
        if(error > 0){
            y = y + y_step;
            error = error - del_x;
        }
    }
    return;
    
}

void draw_score(int score) {
    int hundreds = score / 100;
    int tens = (score % 100) / 10;
    int ones = score % 10;
    int digit;
    draw_digits(ones, 0);
    draw_digits(tens, 12);
    draw_digits(hundreds, 24);

    //draw levels word
    
}

void draw_digits(int digit, int offset) {
    
    if(digit == 0) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 235, 235, 0xFFFFFF);
        draw_line(306 - offset, 306 - offset, 217, 235, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
    }
    if(digit == 1) {
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
    }
    if(digit == 2) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 235, 235, 0xFFFFFF);
        draw_line(306 - offset, 306 - offset, 226, 235, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 226, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
    if(digit == 3) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 235, 235, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
    if(digit == 4) {
        draw_line(306 - offset, 306 - offset, 217, 226, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
    if(digit == 5) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 235, 235, 0xFFFFFF);
        draw_line(306 - offset, 306 - offset, 217, 226, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 226, 235, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
    if(digit == 6) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 235, 235, 0xFFFFFF);
        draw_line(306 - offset, 306 - offset, 217, 235, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 226, 235, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
    if(digit == 7) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
    }
    if(digit == 8) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 235, 235, 0xFFFFFF);
        draw_line(306 - offset, 306 - offset, 217, 235, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
    if(digit == 9) {
        draw_line(306 - offset, 315 - offset, 217, 217, 0xFFFFFF);
        draw_line(306 - offset, 306 - offset, 217, 226, 0xFFFFFF);
        draw_line(315 - offset, 315 - offset, 217, 235, 0xFFFFFF);
        draw_line(306 - offset, 315 - offset, 226, 226, 0xFFFFFF);
    }
}

void clear_screen(){
    int x,y;
    for( x = 0 ; x < 320; ++x){
        for( y = 0; y < 240; ++y){
            plot_pixel(x,y,0);
        }
    }
}

void config_KEYs()
{
    volatile int * KEY_ptr = (int *) 0xFF200050; // KEY base address
    *(KEY_ptr + 2) = 0xF; // enable interrupts for all four KEYs
}

// Define the IRQ exception handler
void __attribute__ ((interrupt)) __cs3_isr_irq (void)
{
    // Read the ICCIAR from the CPU Interface in the GIC
    int interrupt_ID = *((int *) 0xFFFEC10C);
    if (interrupt_ID == 73) // check if interrupt is from the KEYs
        pushbutton_ISR ();
    else
        while (1); // if unexpected, then stay here
    // Write to the End of Interrupt Register (ICCEOIR)
    *((int *) 0xFFFEC110) = interrupt_ID;
}

// Define the remaining exception handlers
void __attribute__ ((interrupt)) __cs3_reset (void)
{
    while(1);
}
void __attribute__ ((interrupt)) __cs3_isr_undef (void)
{
    while(1);
}
void __attribute__ ((interrupt)) __cs3_isr_swi (void)
{
    while(1);
}
void __attribute__ ((interrupt)) __cs3_isr_pabort (void)
{
    while(1);
}
void __attribute__ ((interrupt)) __cs3_isr_dabort (void)
{
    while(1);
}
void __attribute__ ((interrupt)) __cs3_isr_fiq (void)
{
    while(1);
}

/*
 * Turn off interrupts in the ARM processor
 */
void disable_A9_interrupts(void)
{
    int status = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps]"r"(status));
}

/*
 * Initialize the banked stack pointer register for IRQ mode
 */
void set_A9_IRQ_stack(void)
{
    int stack, mode;
    stack = 0xFFFFFFFF - 7; // top of A9 onchip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
    /* set banked stack pointer */
    asm("mov sp, %[ps]" : : [ps] "r" (stack));
    /* go back to SVC mode before executing subroutine return! */
    mode = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r" (mode));
}
/*
 * Turn on interrupts in the ARM processor
 */
void enable_A9_interrupts(void)
{
    int status = 0b01010011;
    asm("msr cpsr, %[ps]" : : [ps]"r"(status));
}

/*
 * Configure the Generic Interrupt Controller (GIC)
 */
void config_GIC(void)
{
    config_interrupt (73, 1); // configure the FPGA KEYs interrupt (73)
    // Set Interrupt Priority Mask Register (ICCPMR). Enable all priorities
    *((int *) 0xFFFEC104) = 0xFFFF;
    // Set the enable in the CPU Interface Control Register (ICCICR)
    *((int *) 0xFFFEC100) = 1;
    // Set the enable in the Distributor Control Register (ICDDCR)
    *((int *) 0xFFFED000) = 1;
}

/*
 * Configure registers in the GIC for an individual Interrupt ID. We
 * configure only the Interrupt Set Enable Registers (ICDISERn) and
 * Interrupt Processor Target Registers (ICDIPTRn). The default (reset)
 * values are used for other registers in the GIC
 */
void config_interrupt (int N, int CPU_target)
{
    int reg_offset, index, value, address;
    /* Configure the Interrupt Set-Enable Registers (ICDISERn).
     * reg_offset = (integer_div(N / 32) * 4; value = 1 << (N mod 32) */
    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    /* Using the address and value, set the appropriate bit */
    *(int *)address |= value;
    /* Configure the Interrupt Processor Targets Register (ICDIPTRn)
     * reg_offset = integer_div(N / 4) * 4; index = N mod 4 */
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;
    /* Using the address and value, write to (only) the appropriate byte */
    *(char *)address = (char) CPU_target;
}

/********************************************************************
 * Pushbutton - Interrupt Service Routine
 *
 * This routine checks which KEY has been pressed. It writes to HEX0
 *******************************************************************/
void pushbutton_ISR( void )
{
    /* KEY base address */
    volatile int *KEY_ptr = (int *) 0xFF200050;
    /* HEX display base address */
    volatile int *HEX3_HEX0_ptr = (int *) 0xFF200020;
    int press, HEX_bits;
    press = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press; // Clear the interrupt
    if (press & 0x1) { // KEY0
        //key0 should make the blob move right
        move_blob_right(&blob_x, &blob_y);
        HEX_bits = 0b00111111;
    }
    else if (press & 0x2) { // KEY1
        //key1 should make the blob move left
        move_blob_left(&blob_x, &blob_y);
        HEX_bits = 0b00000110;
    }
    else if (press & 0x4) {// KEY2
        move_blob_right_more(&blob_x, &blob_y);
        HEX_bits = 0b01011011;
    }
    else {// press & 0x8, which is KEY3
        move_blob_left_more(&blob_x, &blob_y);
        HEX_bits = 0b01001111;
    }
    *HEX3_HEX0_ptr = HEX_bits;
    return;
}