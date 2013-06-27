/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * xrdp-ng interprocess communication protocol
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xrdp-ng/xrdp.h>

#define XRDP_ORDER_HEADER_LENGTH	6

int xrdp_write_header(wStream* s, int type, int length)
{
	Stream_Write_UINT16(s, type);
	Stream_Write_UINT32(s, length);
	return 0;
}

int xrdp_write_begin_update(wStream* s, XRDP_MSG_BEGIN_UPDATE* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_BEGIN_UPDATE, length);

	return 0;
}

int xrdp_write_end_update(wStream* s, XRDP_MSG_END_UPDATE* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_END_UPDATE, length);

	return 0;
}

int xrdp_write_opaque_rect(wStream* s, XRDP_MSG_OPAQUE_RECT* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 8;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_FILL_RECT, length);

	Stream_Write_UINT16(s, msg->nLeftRect);
	Stream_Write_UINT16(s, msg->nTopRect);
	Stream_Write_UINT16(s, msg->nWidth);
	Stream_Write_UINT16(s, msg->nHeight);

	return 0;
}

int xrdp_write_screen_blt(wStream* s, XRDP_MSG_SCREEN_BLT* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 12;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SCREEN_BLT, length);

	Stream_Write_UINT16(s, msg->nLeftRect);
	Stream_Write_UINT16(s, msg->nTopRect);
	Stream_Write_UINT16(s, msg->nWidth);
	Stream_Write_UINT16(s, msg->nHeight);
	Stream_Write_UINT16(s, msg->nXSrc);
	Stream_Write_UINT16(s, msg->nYSrc);

	return 0;
}

int xrdp_write_set_clip(wStream* s, XRDP_MSG_SET_CLIP* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 8;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SET_CLIP, length);
	Stream_Write_UINT16(s, msg->x);
	Stream_Write_UINT16(s, msg->y);
	Stream_Write_UINT16(s, msg->width);
	Stream_Write_UINT16(s, msg->height);

	return 0;
}

int xrdp_write_reset_clip(wStream* s, XRDP_MSG_RESET_CLIP* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_RESET_CLIP, length);

	return 0;
}

int xrdp_write_set_forecolor(wStream* s, XRDP_MSG_SET_FORECOLOR* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 4;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SET_FORECOLOR, length);
	Stream_Write_UINT32(s, msg->ForeColor);

	return 0;
}

int xrdp_write_set_backcolor(wStream* s, XRDP_MSG_SET_BACKCOLOR* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 4;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SET_BACKCOLOR, length);
	Stream_Write_UINT32(s, msg->BackColor);

	return 0;
}

int xrdp_write_set_rop2(wStream* s, XRDP_MSG_SET_ROP2* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 2;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SET_OPCODE, length);
	Stream_Write_UINT16(s, msg->bRop2);

	return 0;
}

int xrdp_write_set_pen(wStream* s, XRDP_MSG_SET_PEN* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 4;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SET_PEN, length);

	Stream_Write_UINT16(s, msg->PenStyle);
	Stream_Write_UINT16(s, msg->PenWidth);

	return 0;
}

int xrdp_write_line_to(wStream* s, XRDP_MSG_LINE_TO* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 8;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_DRAW_LINE, length);

	Stream_Write_UINT16(s, msg->nXStart);
	Stream_Write_UINT16(s, msg->nYStart);
	Stream_Write_UINT16(s, msg->nXEnd);
	Stream_Write_UINT16(s, msg->nYEnd);

	return 0;
}

int xrdp_write_set_cursor(wStream* s, int x, int y, char* cur_data, char* cur_mask)
{
	int size = 4 + 32 * (32 * 3) + 32 * (32 / 8);
	int length = XRDP_ORDER_HEADER_LENGTH + size;

	if (!s)
		return length;

	if (x < 0)
		x = 0;

	if (x > 31)
		x = 31;

	if (y < 0)
		y = 0;

	if (y > 31)
		y = 31;

	xrdp_write_header(s, XRDP_SERVER_SET_POINTER, length);
	Stream_Write_UINT16(s, x);
	Stream_Write_UINT16(s, y);
	Stream_Write(s, cur_data, 32 * (32 * 3));
	Stream_Write(s, cur_mask, 32 * (32 / 8));

	return 0;
}

int xrdp_write_set_cursor_ex(wStream* s, int x, int y, char* cur_data, char* cur_mask, int bpp)
{
	int BytesPerPixel = (bpp == 0) ? 3 : (bpp + 7) / 8;
	int size = 6 + 32 * (32 * BytesPerPixel) + 32 * (32 / 8);
	int length = XRDP_ORDER_HEADER_LENGTH + size;

	if (!s)
		return length;

	if (x < 0)
		x = 0;

	if (x > 31)
		x = 31;

	if (y < 0)
		y = 0;

	if (y > 31)
		y = 31;

	xrdp_write_header(s, XRDP_SERVER_SET_POINTER_EX, length);
	Stream_Write_UINT16(s, x);
	Stream_Write_UINT16(s, y);
	Stream_Write_UINT16(s, bpp);
	Stream_Write(s, cur_data, 32 * (32 * BytesPerPixel));
	Stream_Write(s, cur_mask, 32 * (32 / 8));

	return 0;
}

int xrdp_write_create_os_surface(wStream* s, int index, int width, int height)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 8;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_CREATE_OS_SURFACE, length);
	Stream_Write_UINT32(s, index);
	Stream_Write_UINT16(s, width);
	Stream_Write_UINT16(s, height);

	return 0;
}

int xrdp_write_switch_os_surface(wStream* s, int index)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 4;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SWITCH_OS_SURFACE, length);
	Stream_Write_UINT32(s, index);

	return 0;
}

int xrdp_write_delete_os_surface(wStream* s, int index)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 4;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_DELETE_OS_SURFACE, length);
	Stream_Write_UINT32(s, index);

	return 0;
}

int xrdp_write_paint_rect_os(wStream* s, int x, int y, int width, int height, int index, int srcx, int srcy)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 16;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_PAINT_RECT_OS, length);

	Stream_Write_UINT16(s, x);
	Stream_Write_UINT16(s, y);
	Stream_Write_UINT16(s, width);
	Stream_Write_UINT16(s, height);
	Stream_Write_UINT32(s, index);
	Stream_Write_UINT16(s, srcx);
	Stream_Write_UINT16(s, srcy);

	return 0;
}

int xrdp_write_set_hints(wStream* s, int hints, int mask)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 8;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_SET_HINTS, length);

	Stream_Write_UINT32(s, hints);
	Stream_Write_UINT32(s, mask);

	return 0;
}

int xrdp_write_create_framebuffer(wStream* s, XRDP_MSG_CREATE_FRAMEBUFFER* msg)
{
	int length = XRDP_ORDER_HEADER_LENGTH + 24;

	if (!s)
		return length;

	xrdp_write_header(s, XRDP_SERVER_CREATE_FRAMEBUFFER, length);

	Stream_Write_UINT32(s, msg->width);
	Stream_Write_UINT32(s, msg->height);
	Stream_Write_UINT32(s, msg->scanline);
	Stream_Write_UINT32(s, msg->segmentId);
	Stream_Write_UINT32(s, msg->bitsPerPixel);
	Stream_Write_UINT32(s, msg->bytesPerPixel);

	return 0;
}