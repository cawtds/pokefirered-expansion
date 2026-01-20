#include "global.h"
#include "gba/flash_internal.h"
#include "blit.h"
#include "decompress.h"
#include "dma3.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "m4a.h"
#include "malloc.h"
#include "palette.h"
#include "save.h"
#include "strings.h"
#include "text.h"
#include "window.h"

COMMON_DATA bool32 sIsInSaveFailedScreen = 0;

static EWRAM_DATA u16 sSaveType = SAVE_NORMAL;
static EWRAM_DATA u8 sSaveFailedScreenState = 0;
// static EWRAM_DATA struct HelpSystemVideoState sVideoState = {0};

static void CB2_SaveFailedScreen(void);
static void BlankPalettes(void);
static void UpdateMapBufferWithText(void);
static void ClearMapBuffer(void);
static void PrintTextOnSaveFailedScreen(const u8 *a0);
static bool32 TryWipeDamagedSectors(void);
static bool32 WipeSectors(u32 damagedSectors);
// from help_system_util.c
static void DecompressAndRenderGlyph(u8 fontId, u16 glyph, struct Bitmap *srcBlit, struct Bitmap *destBlit, u8 *destBuffer, u8 x, u8 y, u8 width, u8 height);
static void HelpSystemRenderText(u8 fontId, u8 * dest, const u8 * src, u8 x, u8 y, u8 width, u8 height);

static const u16 sSaveFailedScreenPals[] = INCBIN_U16("graphics/interface/save_failed_screen.gbapal");

void DoSaveFailedScreen(u8 saveType)
{
    sSaveType = saveType;
    sSaveFailedScreenState = 0;
    SetMainCallback2(CB2_SaveFailedScreen);
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_SaveFailedScreen(void)
{
    switch (sSaveFailedScreenState)
    {
    case 0:
        SetVBlankCallback(NULL);
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 128);
        sSaveFailedScreenState = 1;
        break;
    case 1:
        BlankPalettes();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        sSaveFailedScreenState = 2;
        break;
    case 2:
        RequestDma3Fill(0, (void *)BG_CHAR_ADDR(3), BG_CHAR_SIZE, DMA3_16BIT);
        RequestDma3Copy(sSaveFailedScreenPals, (void *)PLTT, 0x20, DMA3_16BIT);
        sSaveFailedScreenState = 3;
        break;
    case 3:
        ClearMapBuffer();
        PrintTextOnSaveFailedScreen(gText_SaveFailedCheckingBackup);
        UpdateMapBufferWithText();
        sSaveFailedScreenState = 4;
        break;
    case 4:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(3) | BGCNT_SCREENBASE(31));
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_BG0_ON);
        sSaveFailedScreenState = 5;
        break;
    case 5:
        if (TryWipeDamagedSectors() == TRUE)
        {
            gSaveAttemptStatus = SAVE_STATUS_OK;
            PrintTextOnSaveFailedScreen(gText_SaveCompletePressA);
        }
        else
        {
            gSaveAttemptStatus = SAVE_STATUS_ERROR;
            PrintTextOnSaveFailedScreen(gText_BackupMemoryDamaged);
        }
        sSaveFailedScreenState = 6;
        break;
    case 6:
        if (JOY_NEW(A_BUTTON))
        {
            SetGpuReg(REG_OFFSET_DISPCNT, 0);
            BlankPalettes();
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            SetVBlankCallback(VBlankCB);
            sSaveFailedScreenState = 7;
        }
        break;
    case 7:
        if (!UpdatePaletteFade())
        {
            if (gGameContinueCallback == NULL) // no callback exists, so do a soft reset.
            {
                DoSoftReset();
            }
            else
            {
                SetMainCallback2((MainCallback)gGameContinueCallback);
                gGameContinueCallback = NULL;
            }
            sSaveFailedScreenState = 0;
        }
        break;
    }
}

static void BlankPalettes(void)
{
    int i;
    for (i = 0; i < BG_PLTT_SIZE; i += sizeof(u16))
    {
        *((u16 *)(BG_PLTT + i)) = RGB_BLACK;
        *((u16 *)(OBJ_PLTT + i)) = RGB_BLACK;
    }
}

static void RequestDmaCopyFromScreenBuffer(u8 *buffer)
{
    RequestDma3Copy(buffer + 0x3800, (void *)BG_SCREEN_ADDR(31), 0x500, DMA3_16BIT);
}

static void RequestDmaCopyFromCharBuffer(u8 *buffer)
{
    RequestDma3Copy(buffer + 0x020, (void *)BG_CHAR_ADDR(3) + 0x20, 0x2300, DMA3_16BIT);
}

static void FillBgMapBufferRect(u16 baseBlock, u8 left, u8 top, u8 width, u8 height, u16 blockOffset)
{
    u16 i, j;
    u8 *buffer = Alloc(0x4000);

    for (i = top; i < top + height; i++)
    {
        for (j = left; j < left + width; j++)
        {
            *((u16 *)(buffer + 0x3800 + 64 * i + 2 * j)) = baseBlock;
            baseBlock += blockOffset;
        }
    }
    RequestDmaCopyFromScreenBuffer(buffer);
}

static void UpdateMapBufferWithText(void)
{
    FillBgMapBufferRect(0x001, 1, 5, 28, 10, 0x001);
}

static void ClearMapBuffer(void)
{
    FillBgMapBufferRect(0x000, 0, 0, 30, 20, 0x000);
}

static void PrintTextOnSaveFailedScreen(const u8 *str)
{
    u8 *buffer = Alloc(0x4000);

    GenerateFontHalfRowLookupTable(TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
    CpuFill16(PIXEL_FILL(1) | (PIXEL_FILL(1) << 8), buffer + 0x20, 0x2300);
    HelpSystemRenderText(2, buffer + 0x20, str, 2, 2, 28, 10);
    RequestDmaCopyFromCharBuffer(buffer);
    Free(buffer);
}

static bool32 TryWipeDamagedSectors(void)
{
    int i = 0;
    for (i = 0; gDamagedSaveSectors != 0 && i < 3; i++)
    {
        if (WipeSectors(gDamagedSaveSectors))
            return FALSE;
        HandleSavingData(sSaveType);
    }
    if (gDamagedSaveSectors != 0)
        return FALSE;
    return TRUE;
}

static bool16 VerifySectorWipe(u16 sector)
{
    u32 *saveDataBuffer = (void *)&gSaveDataBuffer;
    u16 i;

    ReadFlash(sector, 0, (u8 *)saveDataBuffer, SECTOR_SIZE);

    for (i = 0; i < SECTOR_SIZE / sizeof(u32); i++, saveDataBuffer++)
    {
        if (*saveDataBuffer != 0)
            return TRUE;
    }
    return FALSE;
}

static bool32 WipeSector(u32 sector)
{
    u16 i, j;
    bool8 failed = TRUE;

    // Attempt to wipe sector with an arbitrary attempt limit of 130
    for (i = 0; failed && i < 130; i++)
    {
        for (j = 0; j < SECTOR_SIZE; j++)
            ProgramFlashByte(sector, j, 0);

        failed = VerifySectorWipe(sector);
    }

    return failed;
}

static bool32 WipeSectors(u32 sectorBits)
{
    u32 i;

    for (i = 0; i < SECTORS_COUNT; i++)
        if ((sectorBits & (1 << i)) && !WipeSector(i))
            sectorBits &= ~(1 << i);

    if (sectorBits == 0)
        return FALSE;
    else
        return TRUE;
}


// remove this??
static void DecompressAndRenderGlyph(u8 fontId, u16 glyph, struct Bitmap *srcBlit, struct Bitmap *destBlit, u8 *destBuffer, u8 x, u8 y, u8 width, u8 height)
{
    if (fontId == FONT_SMALL)
        DecompressGlyph_Small(glyph, FALSE);
    else if (fontId == FONT_FEMALE)
        DecompressGlyph_Female(glyph, FALSE);
    else
        DecompressGlyph_Normal(glyph, FALSE);
    srcBlit->pixels = (u8*) gCurGlyph.gfxBufferTop;
    srcBlit->width = 16;
    srcBlit->height = 16;
    destBlit->pixels = destBuffer;
    destBlit->width = width * 8;
    destBlit->height = height * 8;
    BlitBitmapRect4Bit(srcBlit, destBlit, 0, 0, x, y, gCurGlyph.width, gCurGlyph.height, 0);
}

// remove this???
static void HelpSystemRenderText(u8 fontId, u8 * dest, const u8 * src, u8 x, u8 y, u8 width, u8 height)
{
    // fontId -> sp+24
    // dest -> sp+28
    // src -> r9
    // x -> sp+34
    // y -> r10
    // width -> sp+2C
    // height -> sp+30
    struct Bitmap srcBlit;
    struct Bitmap destBlit;
    u8 orig_x = x;
    u8 i = 0;
    s32 clearPixels = 0;

    while (1)
    {
        u16 curChar = *src;
        src++;
        switch (curChar)
        {
        case EOS:
            return;
        case CHAR_NEWLINE:
            x = orig_x;
            y += gCurGlyph.height + 1;
            break;
        case PLACEHOLDER_BEGIN:
            curChar = *src;
            src++;
            if (curChar == PLACEHOLDER_ID_PLAYER)
            {
                for (i = 0; i < 10; i++)
                {
                    if (gSaveBlock2Ptr->playerName[i] == EOS)
                    {
                        break;
                    }
                    DecompressAndRenderGlyph(fontId, gSaveBlock2Ptr->playerName[i], &srcBlit, &destBlit, dest, x, y, width, height);
                    // This is required to match a dummy [sp+#0x24] read here
                    if (fontId == FONT_SMALL)
                    {
                        x += gCurGlyph.width;
                    }
                    else
                    {
                        x += gCurGlyph.width + 0;
                    }
                }
            }
            else if (curChar == PLACEHOLDER_ID_STRING_VAR_1)
            {
                for (i = 0; ; i++)
                {
                    if (FlagGet(FLAG_SYS_NOT_SOMEONES_PC) == TRUE)
                    {
                        if (gString_Bill[i] == EOS)
                        {
                            break;
                        }
                        DecompressAndRenderGlyph(fontId, gString_Bill[i], &srcBlit, &destBlit, dest, x, y, width, height);
                    }
                    else
                    {
                        if (gString_Someone[i] == EOS)
                        {
                            break;
                        }
                        DecompressAndRenderGlyph(fontId, gString_Someone[i], &srcBlit, &destBlit, dest, x, y, width, height);
                    }
                    if (fontId == FONT_SMALL)
                    {
                        x += gCurGlyph.width;
                    }
                    else
                    {
                        x += gCurGlyph.width + 0;
                    }
                }
            }
            break;
        case CHAR_PROMPT_SCROLL:
        case CHAR_PROMPT_CLEAR:
            x = orig_x;
            y += gCurGlyph.height + 1;
            break;
        case EXT_CTRL_CODE_BEGIN:
            curChar = *src;
            src++;
            switch (curChar)
            {
            case EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW:
                src++;
                //fallthrough
            case EXT_CTRL_CODE_PLAY_BGM:
            case EXT_CTRL_CODE_PLAY_SE:
                src++;
                //fallthrough
            case EXT_CTRL_CODE_COLOR:
            case EXT_CTRL_CODE_HIGHLIGHT:
            case EXT_CTRL_CODE_SHADOW:
            case EXT_CTRL_CODE_PALETTE:
            case EXT_CTRL_CODE_FONT:
            case EXT_CTRL_CODE_PAUSE:
            case EXT_CTRL_CODE_ESCAPE:
            case EXT_CTRL_CODE_SHIFT_RIGHT:
            case EXT_CTRL_CODE_SHIFT_DOWN:
                src++;
            case EXT_CTRL_CODE_RESET_FONT:
            case EXT_CTRL_CODE_PAUSE_UNTIL_PRESS:
            case EXT_CTRL_CODE_WAIT_SE:
            case EXT_CTRL_CODE_FILL_WINDOW:
                break;
            case EXT_CTRL_CODE_CLEAR:
            case EXT_CTRL_CODE_SKIP:
                src++;
                break;
            case EXT_CTRL_CODE_CLEAR_TO:
            {
                clearPixels = *src + orig_x - x;

                if (clearPixels > 0)
                {
                    destBlit.pixels = dest;
                    destBlit.width = width * 8;
                    destBlit.height = height * 8;
                    FillBitmapRect4Bit(&destBlit, x, y, clearPixels, GetFontAttribute(fontId, FONTATTR_MAX_LETTER_HEIGHT), 0);
                    x += clearPixels;
                }
                src++;
                break;
            }
            case EXT_CTRL_CODE_MIN_LETTER_SPACING:
                src++;
                break;
            case EXT_CTRL_CODE_JPN:
            case EXT_CTRL_CODE_ENG:
                break;
            }
            break;
        case CHAR_KEYPAD_ICON:
            curChar = *src;
            src++;
            srcBlit.pixels = (u8 *)&gKeypadIconTiles[0x20 * GetKeypadIconTileOffset(curChar)];
            srcBlit.width = 0x80;
            srcBlit.height = 0x80;
            destBlit.pixels = dest;
            destBlit.width = width * 8;
            destBlit.height = height * 8;
            BlitBitmapRect4Bit(&srcBlit, &destBlit, 0, 0, x, y, GetKeypadIconWidth(curChar), GetKeypadIconHeight(curChar), 0);
            x += GetKeypadIconWidth(curChar);
            break;
        case CHAR_EXTRA_SYMBOL:
            curChar = *src + 0x100;
            src++;
            //fallthrough
        default:
            if (curChar == CHAR_SPACE)
            {
                if (fontId == FONT_SMALL)
                {
                    x += 5;
                }
                else
                {
                    x += 4;
                }
            }
            else
            {
                DecompressAndRenderGlyph(fontId, curChar, &srcBlit, &destBlit, dest, x, y, width, height);
                if (fontId == FONT_SMALL)
                {
                    x += gCurGlyph.width;
                }
                else
                {
                    x += gCurGlyph.width + 0;
                }
            }
            break;
        }
    }
}
