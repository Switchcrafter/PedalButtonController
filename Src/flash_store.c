/*
 * ----------------------------------------------------------------------------------
 * @file  		: flash_store.c
 * @brief 		: Flash reading/writing  library
 *
 * ----------------------------------------------------------------------------------
 *	 Copyright (c) 2016 Dmitry Skulkin <dmitry.skulkin@gmail.com>					*
 *																					*
 *	Permission is hereby granted, free of charge, to any person obtaining a copy	*
 *	of this software and associated documentation files (the "Software"), to deal	*
 *	in the Software without restriction, including without limitation the rights	*
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell		*
 *	copies of the Software, and to permit persons to whom the Software is			*
 *	furnished to do so, subject to the following conditions:						*
 *																					*
 *	The above copyright notice and this permission notice shall be included in all	*
 *	copies or substantial portions of the Software.									*
 *																					*
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR		*
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,		*
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE		*
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER			*
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,	*
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE	*
 *	SOFTWARE.																		*
 * ----------------------------------------------------------------------------------
 * */

#include <flash_store.h>


extern struct pin_conf pins[USEDPINS];
extern struct axis_conf axises[AXISES];
extern uint8_t POV_config;
volatile extern uint16_t Rot_Press_Time;
volatile extern uint16_t Rot_Debounce_Time;
volatile extern uint16_t Button_Debounce_Time;
volatile extern uint16_t Button_Press_time;
volatile extern uint16_t RotSwitch_Press_Time;
volatile extern uint8_t USB_Product_String_Unique[10];
volatile extern uint8_t USB_Serial_Number_Unique[13];
volatile extern uint8_t USB_polling_interval;


uint16_t * get_lastpage_addr(uint16_t * flash_size_reg_addr) {
	uint16_t flash_size;

	flash_size = *flash_size_reg_addr;

	return (uint16_t *)(FLASH_BASE + (flash_size-1)*1024);

}


void get_config(void) {
	uint16_t * curradr;
	uint8_t tmp_low,tmp_high;

	curradr = get_lastpage_addr((uint16_t *)FLASHSIZEREG);

	for (uint8_t i=0;i<USEDPINS;i++) {
		pins[i].pin_type = (uint8_t)*curradr;
		curradr++;
	}
	for (uint8_t i=0;i<AXISES;i++) {
		axises[i].calib_min_lowbyte = (uint8_t)*curradr;
		curradr++;
		axises[i].calib_min_hibyte = (uint8_t)*curradr;
		curradr++;
		axises[i].calib_max_lowbyte = (uint8_t)*curradr;
		curradr++;
		axises[i].calib_max_hibyte = (uint8_t)*curradr;
		curradr++;
		axises[i].special = (uint8_t)*curradr;
		curradr++;
		axises[i].calib_min=(axises[i].calib_min_hibyte << 8) | axises[i].calib_min_lowbyte;
		axises[i].calib_max=(axises[i].calib_max_hibyte << 8) | axises[i].calib_max_lowbyte;
	}
	POV_config = (uint8_t)*curradr;
	curradr++;

	tmp_low=(uint8_t)*curradr; curradr++; tmp_high=(uint8_t)*curradr; curradr++;
	Rot_Press_Time=(tmp_high<<8)+tmp_low;

	tmp_low=(uint8_t)*curradr; curradr++; tmp_high=(uint8_t)*curradr; curradr++;
	Rot_Debounce_Time=(tmp_high<<8)+tmp_low;

	tmp_low=(uint8_t)*curradr; curradr++; tmp_high=(uint8_t)*curradr; curradr++;
	Button_Debounce_Time=(tmp_high<<8)+tmp_low;

//	tmp_low=(uint8_t)*curradr; curradr++; tmp_high=(uint8_t)*curradr; curradr++;
//	Button_Press_time=(tmp_high<<8)+tmp_low;
	USB_polling_interval=(uint8_t)*curradr; curradr++;
	curradr++;

	tmp_low=(uint8_t)*curradr; curradr++; tmp_high=(uint8_t)*curradr; curradr++;
	RotSwitch_Press_Time=(tmp_high<<8)+tmp_low;

	for (uint8_t i=0; i<10; i++) {
		USB_Product_String_Unique[i]=(uint8_t)*curradr; curradr++;
	}

	for (uint8_t i=0; i<10; i++) {
		USB_Serial_Number_Unique[i+2] = (uint8_t)*curradr; curradr++;
	}
}

void erase_flash(void) {
	 /* Authorize the FPEC Access */
	  FLASH->KEYR = FLASH_KEY1;
	  FLASH->KEYR = FLASH_KEY2;

	  /* Clear 2 page */
	  FLASH->CR |= FLASH_CR_PER; /* Page erase */
	  FLASH->AR = (uint32_t)get_lastpage_addr((uint16_t *)FLASHSIZEREG);
	  FLASH->CR|= FLASH_CR_STRT; /* Start erase */
	  while ((FLASH->SR & FLASH_SR_BSY) != 0 ) /* Wait end of eraze */
	    ;
	  FLASH->CR &= ~FLASH_CR_PER; /* Page erase end */
	  FLASH->CR |= FLASH_CR_LOCK; /* Lock the flash back */
}

void write_flash(void) {
  uint16_t * curradr;

  /* Authorize the FPEC Access */
  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;


  curradr = get_lastpage_addr((uint16_t *)FLASHSIZEREG);

  FLASH->CR |= FLASH_CR_PG; /* Programm the flash */

  for (uint8_t i=0;i<USEDPINS;i++) {
    *curradr = pins[i].pin_type;
     while ((FLASH->SR & FLASH_SR_BSY) != 0 )
      ;
     curradr++;
   }
	for (uint8_t i=0;i<AXISES;i++) {
		*curradr = axises[i].calib_min_lowbyte;
		while ((FLASH->SR & FLASH_SR_BSY) != 0 )
		      ;
		     curradr++;
		*curradr = axises[i].calib_min_hibyte;
		while ((FLASH->SR & FLASH_SR_BSY) != 0 )
		      ;
		     curradr++;
		*curradr = axises[i].calib_max_lowbyte;
		while ((FLASH->SR & FLASH_SR_BSY) != 0 )
		      ;
		     curradr++;
		*curradr = axises[i].calib_max_hibyte;
		while ((FLASH->SR & FLASH_SR_BSY) != 0 )
		      ;
		     curradr++;
		*curradr = axises[i].special;
		while ((FLASH->SR & FLASH_SR_BSY) != 0 )
		      ;
		     curradr++;
	}
	*curradr = POV_config;
	curradr++;

	*curradr=LOBYTE(Rot_Press_Time); curradr++;
	*curradr=HIBYTE(Rot_Press_Time); curradr++;

	*curradr=LOBYTE(Rot_Debounce_Time); curradr++;
	*curradr=HIBYTE(Rot_Debounce_Time); curradr++;

	*curradr=LOBYTE(Button_Debounce_Time); curradr++;
	*curradr=HIBYTE(Button_Debounce_Time); curradr++;

//	*curradr=LOBYTE(Button_Press_time); curradr++;
//	*curradr=HIBYTE(Button_Press_time); curradr++;
	*curradr=USB_polling_interval; curradr++;
	curradr++;

	*curradr=LOBYTE(RotSwitch_Press_Time); curradr++;
	*curradr=HIBYTE(RotSwitch_Press_Time); curradr++;

	for (uint8_t i=0; i<10; i++) {
		*curradr=USB_Product_String_Unique[i]; curradr++;
	}

	for (uint8_t i=0; i<10; i++) {
		*curradr=USB_Serial_Number_Unique[i+2]; curradr++;
	}


  FLASH->CR &= ~FLASH_CR_PG; /* Reset the flag back !!!! */
  FLASH->CR |= FLASH_CR_LOCK; /* Lock the flash back */
}
