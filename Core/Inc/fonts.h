/*
 * fonts.h
 *
 *  Created on: Aug 29, 2022
 *      Author: oleksii_khanin
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include "stdint.h"

typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;

//Font lib.
extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;
extern FontDef CuteFont16x21;
extern FontDef ArialNarrow8x12;
extern FontDef ProggyMono12x26;
extern FontDef ArialNarrow15x19;
extern FontDef ArialNarrow10x13;
extern FontDef ArialNarrow12x16;


#endif /* INC_FONTS_H_ */
