/**
 *!
 * \file        b_drv_ssd1289.c
 * \version     v0.0.1
 * \date        2020/03/02
 * \author      Bean(notrynohigh@outlook.com)
 *******************************************************************************
 * @attention
 *
 * Copyright (c) 2020 Bean
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************
 */

/*Includes ----------------------------------------------*/
#include "drivers/inc/b_drv_ssd1289.h"

/**
 * \addtogroup BABYOS
 * \{
 */

/**
 * \addtogroup B_DRIVER
 * \{
 */

/**
 * \addtogroup SSD1289
 * \{
 */

/**
 * \defgroup SSD1289_Private_TypesDefinitions
 * \{
 */

/**
 * \}
 */

/**
 * \defgroup SSD1289_Private_Defines
 * \{
 */
#ifndef LCD_X_SIZE
#define LCD_X_SIZE 240
#endif

#ifndef LCD_Y_SIZE
#define LCD_Y_SIZE 320
#endif
/**
 * \}
 */

/**
 * \defgroup SSD1289_Private_Macros
 * \{
 */

/**
 * \}
 */

/**
 * \defgroup SSD1289_Private_Variables
 * \{
 */
HALIF_KEYWORD bSSD1289_HalIf_t bSSD1289_HalIf = HAL_SSD1289_IF;
bSSD1289_Driver_t              bSSD1289_Driver;
/**
 * \}
 */

/**
 * \defgroup SSD1289_Private_FunctionPrototypes
 * \{
 */

/**
 * \}
 */

/**
 * \defgroup SSD1289_Private_Functions
 * \{
 */

static void _bLcdWriteData(uint16_t dat)
{
    if (bSSD1289_HalIf.if_type == LCD_IF_TYPE_RWADDR)
    {
        ((bLcdRWAddress_t *)bSSD1289_HalIf._if.rw_addr)->dat = dat;
    }
    if (bSSD1289_HalIf.if_type == LCD_IF_TYPE_IO)
    {
        bHalGpioWritePin(bSSD1289_HalIf._if._io.rs.port, bSSD1289_HalIf._if._io.rs.pin, 1);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.rd.port, bSSD1289_HalIf._if._io.rd.pin, 1);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.cs.port, bSSD1289_HalIf._if._io.cs.pin, 0);
        bHalGpioWritePort(bSSD1289_HalIf._if._io.data.port, dat);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.wr.port, bSSD1289_HalIf._if._io.wr.pin, 0);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.wr.port, bSSD1289_HalIf._if._io.wr.pin, 1);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.cs.port, bSSD1289_HalIf._if._io.cs.pin, 1);
    }
}

static void _bLcdWriteCmd(uint16_t cmd)
{
    if (bSSD1289_HalIf.if_type == LCD_IF_TYPE_RWADDR)
    {
        ((bLcdRWAddress_t *)bSSD1289_HalIf._if.rw_addr)->reg = cmd;
    }
    else if (bSSD1289_HalIf.if_type == LCD_IF_TYPE_IO)
    {
        bHalGpioWritePin(bSSD1289_HalIf._if._io.rs.port, bSSD1289_HalIf._if._io.rs.pin, 0);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.rd.port, bSSD1289_HalIf._if._io.rd.pin, 1);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.cs.port, bSSD1289_HalIf._if._io.cs.pin, 0);
        bHalGpioWritePort(bSSD1289_HalIf._if._io.data.port, cmd);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.wr.port, bSSD1289_HalIf._if._io.wr.pin, 0);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.wr.port, bSSD1289_HalIf._if._io.wr.pin, 1);
        bHalGpioWritePin(bSSD1289_HalIf._if._io.cs.port, bSSD1289_HalIf._if._io.cs.pin, 1);
    }
}

static void _SSD1289WriteReg(uint16_t cmd, uint16_t dat)
{
    _bLcdWriteCmd(cmd);
    _bLcdWriteData(dat);
}

static void _bSSD1289SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    _SSD1289WriteReg(0x0044, ((x2 << 8) | x1));
    _SSD1289WriteReg(0x0045, y1);
    _SSD1289WriteReg(0x0046, y2);
    _SSD1289WriteReg(0x004E, x1);
    _SSD1289WriteReg(0x004F, y1);
    _bLcdWriteCmd(0x0022);
}

/***********************************************************************driver interface*******/

static int _bSSD1289FillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int      i   = 0;
    uint16_t tmp = 0;
    if (x1 > x2)
    {
        tmp = x1;
        x1  = x2;
        x2  = tmp;
    }
    if (y1 > y2)
    {
        tmp = y1;
        y1  = y2;
        y2  = tmp;
    }

    if (x2 >= LCD_X_SIZE || y2 >= LCD_Y_SIZE)
    {
        return -1;
    }

    _bSSD1289SetWindow(x1, y1, x2, y2);

    for (i = 0; i < ((x2 - x1 + 1) * (y2 - y1 + 1)); i++)
    {
        _bLcdWriteData(color);
    }
    return 0;
}

static int _bSSD1289Ctl(bSSD1289_Driver_t *pdrv, uint8_t cmd, void *param)
{
    int             retval = -1;
    bLcdRectInfo_t *pinfo  = (bLcdRectInfo_t *)param;
    switch (cmd)
    {
        case bCMD_FILL_RECT:
            if (param == NULL)
            {
                return -1;
            }
            retval = _bSSD1289FillRect(pinfo->x1, pinfo->y1, pinfo->x2, pinfo->y2, pinfo->color);
            break;
        default:
            break;
    }
    return retval;
}

static int _bSSD1289Write(bSSD1289_Driver_t *pdrv, uint32_t addr, uint8_t *pbuf, uint32_t len)
{
    uint16_t     x      = addr % LCD_X_SIZE;
    uint16_t     y      = addr / LCD_X_SIZE;
    bLcdWrite_t *pcolor = (bLcdWrite_t *)pbuf;
    if (y >= LCD_Y_SIZE || pbuf == NULL || len < sizeof(bLcdWrite_t))
    {
        return -1;
    }
    _bSSD1289SetWindow(x, y, x, y);
    _bLcdWriteData(pcolor->color);
    return 2;
}

/**
 * \}
 */

/**
 * \addtogroup SSD1289_Exported_Functions
 * \{
 */
int bSSD1289_Init()
{
    _SSD1289WriteReg(0x0007, 0x0021);
    _SSD1289WriteReg(0x0000, 0x0001);
    _SSD1289WriteReg(0x0007, 0x0023);
    _SSD1289WriteReg(0x0010, 0x0000);
    _SSD1289WriteReg(0x0007, 0x0033);
    _SSD1289WriteReg(0x0011, 0x6838);
    _SSD1289WriteReg(0x0002, 0x0600);
    _SSD1289WriteReg(0x0001, 0x2B3F);

    bSSD1289_Driver.status  = 0;
    bSSD1289_Driver.init    = bSSD1289_Init;
    bSSD1289_Driver.close   = NULL;
    bSSD1289_Driver.read    = NULL;
    bSSD1289_Driver.ctl     = _bSSD1289Ctl;
    bSSD1289_Driver.open    = NULL;
    bSSD1289_Driver.write   = _bSSD1289Write;
    bSSD1289_Driver._hal_if = (void *)&bSSD1289_HalIf;
    return 0;
}

bDRIVER_REG_INIT(bSSD1289_Init);

/**
 * \}
 */

/**
 * \}
 */

/**
 * \}
 */

/**
 * \}
 */

/************************ Copyright (c) 2019 Bean *****END OF FILE****/
