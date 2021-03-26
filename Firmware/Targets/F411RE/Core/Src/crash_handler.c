/*
 * CrashDump.c
 *
 *  Created on: 25 Mar 2021
 *      Author: Lidders
 */

#include <assert.h>
#include "CrashCatcher.h"
#include "main.h"
#include "SEGGER_RTT.h"


static void dumpHexDigit(uint8_t nibble)
{
    static const char hexToASCII[] = "0123456789ABCDEF";

    assert( nibble < 16 );
    SEGGER_RTT_PutChar(0, hexToASCII[nibble]);
}

static void dumpByteAsHex(uint8_t byte)
{
    dumpHexDigit(byte >> 4);
    dumpHexDigit(byte & 0xF);
}

static void dumpBytes(const uint8_t* pMemory, size_t elementCount)
{
    size_t i;

    for (i = 0 ; i < elementCount ; i++)
    {
        /* Only dump 16 bytes to a single line before introducing a line break. */
        if (i != 0 && (i & 0xF) == 0)
            SEGGER_RTT_WriteString(0, "\r\n");

        dumpByteAsHex(*pMemory++);
    }
}

static void dumpHalfwords(const uint16_t* pMemory, size_t elementCount)
{
    size_t i;

    for (i = 0 ; i < elementCount ; i++)
    {
        uint16_t val = *pMemory++;
        /* Only dump 8 halfwords to a single line before introducing a line break. */
        if (i != 0 && (i & 0x7) == 0)
            SEGGER_RTT_WriteString(0, "\r\n");
        dumpBytes((uint8_t*)&val, sizeof(val));
    }
}

static void dumpWords(const uint32_t* pMemory, size_t elementCount)
{
    size_t i;

    for (i = 0 ; i < elementCount ; i++)
    {
        uint32_t val = *pMemory++;
        /* Only dump 4 words to a single line before introducing a line break. */
        if (i != 0 && (i & 0x3) == 0)
            SEGGER_RTT_WriteString(0, "\r\n");

        dumpBytes((uint8_t*)&val, sizeof(val));
    }
}

/* Called at the beginning of crash dump. You should provide an implementation which prepares for the dump by opening
   a dump file, prompting the user to begin a crash dump, or whatever makes sense for your scenario. */
void CrashCatcher_DumpStart(const CrashCatcherInfo* pInfo)
{
	HAL_GPIO_WritePin(LED_ERR_GPIO_Port, LED_ERR_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
//    SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal for OpenFFBoard\r\n");
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    SEGGER_RTT_WriteString(0, "\r\n\r\n###CRASH###\r\n");
}

/* Called to obtain an array of regions in memory that should be dumped as part of the crash.  This will typically
   be all RAM regions that contain volatile data.  For some crash scenarios, a user may decide to also add peripheral
   registers of interest (ie. dump some ethernet registers when you are encountering crashes in the network stack.)
   If NULL is returned from this function, the core will only dump the registers. */
const CrashCatcherMemoryRegion* CrashCatcher_GetMemoryRegions(void)
{
    static const CrashCatcherMemoryRegion regions[] = {
    		{0x10000000, 0x1000FFFF, CRASH_CATCHER_BYTE}, // 64K CCM RAM
    		{0X20000000, 0x2001FFFF, CRASH_CATCHER_BYTE} // 128K SRAM
    };
    return regions;
}

/* Called to dump the next chunk of memory to the dump (this memory may point to register contents which has been copied
   to memory by CrashCatcher already.  The element size will be 8-bits, 16-bits, or 32-bits.  The implementation should
   use reads of the specified size since some memory locations may only support the indicated size. */
void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount)
{
    switch (elementSize)
    {
        case CRASH_CATCHER_BYTE:
            dumpBytes(pvMemory, elementCount);
            break;
        case CRASH_CATCHER_HALFWORD:
            dumpHalfwords(pvMemory, elementCount);
            break;
        case CRASH_CATCHER_WORD:
            dumpWords(pvMemory, elementCount);
            break;
    }
    SEGGER_RTT_WriteString(0, "\r\n");
}

/* Called at the end of crash dump. You should provide an implementation which cleans up at the end of dump. This could
   include closing a dump file, blinking LEDs, infinite looping, and/or returning CRASH_CATCHER_TRY_AGAIN if
   CrashCatcher should prepare to dump again incase user missed the first attempt. */
CrashCatcherReturnCodes CrashCatcher_DumpEnd(void)
{
	HAL_Delay(100);
	SEGGER_RTT_WriteString(0, "###END###\r\n");
	while (1) {

	}
    return CRASH_CATCHER_EXIT;
}

