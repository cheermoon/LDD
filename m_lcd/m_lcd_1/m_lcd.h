#ifndef M_LCD_H
#define M_LCD_H

struct lcd_regs {
		unsigned long	lcdcon1;
		unsigned long	lcdcon2;
		unsigned long	lcdcon3;
		unsigned long	lcdcon4;
		unsigned long	lcdcon5;
    		unsigned long	lcdsaddr1;
    		unsigned long	lcdsaddr2;
    		unsigned long	lcdsaddr3;
    		unsigned long	redlut;
    		unsigned long	greenlut;
    		unsigned long	bluelut;
		unsigned long	reserved[9];
    		unsigned long	dithmode;
		unsigned long	tpal;
		unsigned long	lcdintpnd;
		unsigned long	lcdsrcpnd;
		unsigned long	lcdintmsk;
		unsigned long	lpcsel;
};


#endif
