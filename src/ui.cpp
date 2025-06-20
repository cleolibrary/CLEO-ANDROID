#include "ui.h"
#include "armhook.h"
#include "utils.h"
#include "pattern.h"
#include "touch.h"
#include "strutils.h"
#include "memutils.h"

#ifndef ANDROID
#include "psplang.h"
#endif

namespace ui
{
	typedef void (*fn_Draw_Poly)(float, float, float, float, float, float, float, float, uint8_t *);
	fn_Draw_Poly draw_poly;

	enum eAlign
	{
		eAlignCenter,
		eAlignLeft,
		eAlignRight
	};

	enum eStyle
	{
		eStyleSimple,
		eStyleClassic,
		eStyleSpecial
	};

	typedef void (*fn_Print_String)(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style);
	fn_Print_String print_string;

	void on_draw();

	uint8_t menu_arrow_color[4];
	uint8_t menu_arrow_selected_alpha;
	uint8_t menu_active_item_font_color[4];
	uint8_t menu_selected_item_font_color[4];

#ifdef ANDROID

	namespace gta3
	{
		// @CSprite2d::SetVertices
		typedef void (*fn_CSprite2d__SetVertices)(float, float, float, float, float, float, float, float, uint8_t *, uint8_t *, uint8_t *, uint8_t *);
		fn_CSprite2d__SetVertices CSprite2d__SetVertices;

		// @RwRenderStateSet
		typedef int32_t (*fn_RwRenderStateSet)(int32_t state, int32_t val);
		fn_RwRenderStateSet RwRenderStateSet;

		// @RwIm2DRenderPrimitive
		typedef void (*fn_RwIm2DRenderPrimitive)(int32_t type, void *verts, int32_t count);
		fn_RwIm2DRenderPrimitive RwIm2DRenderPrimitive;

		// @CSprite2d::maVertices
		void *CSprite2d__maVertices;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)();
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw()
		{
			CHud__Draw_();
			on_draw();
		}

		// @CFont::Details
		#pragma pack(push, 1)
		struct _CFont__Details
		{
			uint32_t	m_dwColor;				// 0x00
			float		m_fScaleX;	 			// 0x04
			float		m_fScaleY;	 			// 0x08
			float		m_fSlant;	 			// 0x0C
			float		m_fSlantRefPointX;		// 0x10
			float		m_fSlantRefPointY;		// 0x14
			bool		m_bLeftJustify;			// 0x18
			bool		m_bCentre;				// 0x19
			bool		m_bRightJustify;		// 0x1A
			bool		m_bBackground;			// 0x1B
			bool		m_bBackGroundOnlyText;	// 0x1C
			bool		m_bProp;				// 0x1D
			uint16_t	m_bPad0;				// 0x1E
			float		m_fAlphaFade; 			// 0x20
			uint32_t	m_dwBackgroundColor;	// 0x24
			float		m_fWrapx;	 			// 0x28
			float		m_fCentreSize; 			// 0x2C
			float		m_fRightJustifyWrap;	// 0x30
			uint16_t	m_wFontStyle;			// 0x34
			uint16_t	m_wPad0;				// 0x36
			uint32_t	m_dwUnk3;				// 0x38
			uint16_t	m_wDropShadowPosition;	// 0x3C
			uint32_t	m_dwDropColor;			// 0x3E
			uint16_t	m_wPad1;				// 0x42
		} *CFont__Details;
		#pragma pack(pop)

		STRUCT_SIZE(_CFont__Details, 0x44);

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint16_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(float, float, uint16_t *);
		fn_CFont__PrintString CFont__PrintString;

		// @RsGlobal
		void *RsGlobal;

		float trans_x(float x) { return x * *cast<int32_t *>(cast<uint32_t>(RsGlobal) + 12); }
		float trans_y(float y) { return y * *cast<int32_t *>(cast<uint32_t>(RsGlobal) + 16); }

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			CSprite2d__SetVertices(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
					 	 	 	 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty),
					 	 	 	 rgba, rgba, rgba, rgba);
			RwRenderStateSet(1, 0);
			RwRenderStateSet(7, 1);
			RwRenderStateSet(6, 0);
			RwRenderStateSet(8, 0);
			RwRenderStateSet(12, rgba[3] != 255);
			RwIm2DRenderPrimitive(5, CSprite2d__maVertices, 4);
			RwRenderStateSet(6, 1);
			RwRenderStateSet(8, 1);
			RwRenderStateSet(7, 2);
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{
			// save current font settings
			_CFont__Details	*fd = CFont__Details;
			_CFont__Details fdbcp = *fd;

			// set font align
			fd->m_bCentre = false;
			fd->m_bLeftJustify = false;
			fd->m_bRightJustify = false;
			switch (align)
			{
			case eAlignCenter:
				fd->m_bCentre = true;
				break;
			case eAlignLeft:
				fd->m_bLeftJustify = true;
				break;
			case eAlignRight:
				fd->m_bRightJustify = true;
				break;
			}
			// style
			switch (style)
			{
			case eStyleClassic:
				CFont__SetFontStyle(2);
				scalex *= 2.6;
				scaley *= 1.5;
				break;
			case eStyleSimple:
				CFont__SetFontStyle(2);
				scalex *= 2.4;
				scaley *= 1.3;
				break;
			case eStyleSpecial:
				CFont__SetFontStyle(0);
				break;
			}
			// scale
			fd->m_fScaleX = scalex;
			fd->m_fScaleY = scaley;
			// color
			fd->m_dwColor = *cast<uint32_t*>(rgba);
			// common
			fd->m_bBackground = false;
			fd->m_bProp = true;
			fd->m_wDropShadowPosition = 1;
			fd->m_dwDropColor = 0xFF000000;

			// print string
			CFont__PrintString(trans_x(x), trans_y(y), str);

			// restore font settings
			*fd = fdbcp;
		}

		void init()
		{
			// @CSprite2d::SetVertices
			CSprite2d__SetVertices = getsym<fn_CSprite2d__SetVertices>("_ZN9CSprite2d11SetVerticesEffffffffRK5CRGBAS2_S2_S2_");
			// @RwRenderStateSet
			RwRenderStateSet = getsym<fn_RwRenderStateSet>("_Z16RwRenderStateSet13RwRenderStatePv");
			// @RwIm2DRenderPrimitive
			RwIm2DRenderPrimitive = getsym<fn_RwIm2DRenderPrimitive>("_Z21RwIm2DRenderPrimitive15RwPrimitiveTypeP14RwOpenGLVertexi");
			// @CSprite2d::maVertices
			CSprite2d__maVertices = getsym<void *>("_ZN9CSprite2d10maVerticesE");

			// @CHud::Draw
			_CHud__Draw = getsym<fn_CHud__Draw>("_ZN4CHud4DrawEv");
			armhook::hook_thumb_func(_CHud__Draw, 4, CHud__Draw, &CHud__Draw_);
			// @CFont::Details
			CFont__Details = getsym<_CFont__Details*>("_ZN5CFont7DetailsE");
			// @CFont::SetFontStyle
			CFont__SetFontStyle = getsym<fn_CFont__SetFontStyle>("_ZN5CFont12SetFontStyleEs");
			// @CFont::PrintString
			CFont__PrintString = getsym<fn_CFont__PrintString>("_ZN5CFont11PrintStringEffPt");
			// @RsGlobal
			RsGlobal = getsym<void *>("RsGlobal");

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = {225, 216, 161, 255};
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t arrow_color[4] = { 55, 127, 175, 110 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 220;
		}
	}

	namespace vc
	{
		// @CSprite2d::Draw2DPolygon
		typedef void (*fn_CSprite2d__Draw2DPolygon)(float, float, float, float, float, float, float, float, uint8_t *);
		fn_CSprite2d__Draw2DPolygon CSprite2d__Draw2DPolygon;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)();
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw()
		{
			CHud__Draw_();
			on_draw();
		}

		// @CFont::Details
		#pragma pack(push, 1)
		struct _CFont__Details
		{
			uint32_t	m_dwColor;				// 0x00
			float		m_fScaleX;	 			// 0x04
			float		m_fScaleY;	 			// 0x08
			float		m_fSlant;	 			// 0x0C
			float		m_fSlantRefPointX;		// 0x10
			float		m_fSlantRefPointY;		// 0x14
			bool		m_bLeftJustify;			// 0x18
			bool		m_bCentre;				// 0x19
			bool		m_bRightJustify;		// 0x1A
			bool		m_bBackground;			// 0x1B
			bool		m_bBackGroundOnlyText;	// 0x1C
			bool		m_bProp;				// 0x1D
			bool		m_bUnk0;				// 0x1E
			bool		m_bFlash;				// 0x1F
			uint32_t	m_dwUnk1;				// 0x20
			float		m_fAlphaFade; 			// 0x24
			uint32_t	m_dwBackgroundColor;	// 0x28
			float		m_fWrapx;	 			// 0x2C
			float		m_fCentreSize; 			// 0x30
			float		m_fRightJustifyWrap;	// 0x34
			uint16_t	m_wFontStyle;			// 0x38
			bool		m_bFontStyle_unk;		// 0x3A
			bool		m_bUnk2;				// 0x3B
			uint32_t	m_dwUnk3;				// 0x3C
			uint16_t	m_wDropShadowPosition;	// 0x40
			uint32_t	m_dwDropColor;			// 0x42
			uint8_t		m_bUnk4[0x12];			// 0x46
		} *CFont__Details;
		#pragma pack(pop)

		STRUCT_SIZE(_CFont__Details, 0x58);

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint16_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(float, float, uint16_t *);
		fn_CFont__PrintString CFont__PrintString;

		// @RsGlobal
		void *RsGlobal;

		float trans_x(float x) { return x * *cast<int32_t *>(cast<uint32_t>(RsGlobal) + 12); }
		float trans_y(float y) { return y * *cast<int32_t *>(cast<uint32_t>(RsGlobal) + 16); }

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			CSprite2d__Draw2DPolygon(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
									 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty), rgba);
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{
			// save current font settings
			_CFont__Details	*fd = CFont__Details;
			_CFont__Details fdbcp = *fd;

			// set font align
			fd->m_bCentre = false;
			fd->m_bLeftJustify = false;
			fd->m_bRightJustify = false;
			switch (align)
			{
			case eAlignCenter:
				fd->m_bCentre = true;
				break;
			case eAlignLeft:
				fd->m_bLeftJustify = true;
				break;
			case eAlignRight:
				fd->m_bRightJustify = true;
				break;
			}
			// style
			switch (style)
			{
			case eStyleClassic:
				CFont__SetFontStyle(2);
				scalex *= 1.9;
				scaley *= 1.5;
				break;
			case eStyleSimple:
				CFont__SetFontStyle(2);
				scalex *= 1.8;
				scaley *= 1.3;
				break;
			case eStyleSpecial:
				CFont__SetFontStyle(0);
				break;
			}
			// scale
			fd->m_fScaleX = scalex;
			fd->m_fScaleY = scaley;
			// color
			fd->m_dwColor = *cast<uint32_t*>(rgba);
			// common
			fd->m_bBackground = false;
			fd->m_bFlash = false;
			fd->m_bProp = true;
			fd->m_wDropShadowPosition = 1;
			fd->m_dwDropColor = 0xFF000000;

			// print string
			CFont__PrintString(trans_x(x), trans_y(y), str);

			// restore font settings
			*fd = fdbcp;
		}

		void init()
		{
			// @CSprite2d::Draw2DPolygon
			CSprite2d__Draw2DPolygon = getsym<fn_CSprite2d__Draw2DPolygon>("_ZN9CSprite2d13Draw2DPolygonEffffffffRK5CRGBA");
			// @CHud::Draw
			_CHud__Draw = getsym<fn_CHud__Draw>("_ZN4CHud4DrawEv");
			armhook::hook_thumb_func(_CHud__Draw, 4, CHud__Draw, &CHud__Draw_);
			// @CFont::Details
			CFont__Details = getsym<_CFont__Details*>("_ZN5CFont7DetailsE");
			// @CFont::SetFontStyle
			CFont__SetFontStyle = getsym<fn_CFont__SetFontStyle>("_ZN5CFont12SetFontStyleEs");
			// @CFont::PrintString
			CFont__PrintString = getsym<fn_CFont__PrintString>("_ZN5CFont11PrintStringEffPt");
			// @RsGlobal
			RsGlobal = getsym<void *>("RsGlobal");

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = {225, 216, 161, 255};
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t arrow_color[4] = { 55, 127, 175, 110 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 220;
		}
	}

	namespace sa
	{
		// @CSprite2d::Draw2DPolygon
		typedef void (*fn_CSprite2d__Draw2DPolygon)(float, float, float, float, float, float, float, float, uint8_t *);
		fn_CSprite2d__Draw2DPolygon CSprite2d__Draw2DPolygon;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)();
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw()
		{
			CHud__Draw_();
			on_draw();
		}

		// @CFont::Details
		#pragma pack(push, 1)
		struct _CFont__Details
		{
			uint32_t	m_dwColor;				// 0x00
			float		m_fScaleX;	 			// 0x04
			float		m_fScaleY;	 			// 0x08
			float		m_fSlant;	 			// 0x0C
			float		m_fSlantRefPointX;		// 0x10
			float		m_fSlantRefPointY;		// 0x14
			bool		m_bLeftJustify;			// 0x18
			bool		m_bCentre;				// 0x19
			bool		m_bRightJustify;		// 0x1A
			bool		m_bBackground;			// 0x1B
			bool		m_bBackGroundOnlyText;	// 0x1C
			bool		m_bProp;				// 0x1D
			bool		m_bUnk0;				// 0x1E
			bool		m_bFlash;				// 0x1F
			float		m_fAlpha; 				// 0x20
			uint32_t	m_dwBackgroundColor;	// 0x24
			float		m_fWrapx;	 			// 0x28
			float		m_fCentreSize; 			// 0x2C
			float		m_fRightJustifyWrap;	// 0x30
			uint8_t		m_bFontStyle;			// 0x34
			uint8_t		m_bFontStyle_unk;		// 0x35
			uint8_t		m_bDropShadowPosition;	// 0x36
			uint32_t	m_dwDropColor;			// 0x37
			uint8_t		m_bEdge1;				// 0x3B
			uint8_t		m_bEdge2;				// 0x3C
			uint8_t		padding[3];				// 0x3D
		} *CFont__Details;
		#pragma pack(pop)

		STRUCT_SIZE(_CFont__Details, 0x40);

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint8_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(float, float, uint16_t *);
		fn_CFont__PrintString CFont__PrintString;

		// @RsGlobal
		void *RsGlobal;

		float trans_x(float x) { return x * *cast<int32_t *>(cast<uint32_t>(RsGlobal) + 4); }
		float trans_y(float y) { return y * *cast<int32_t *>(cast<uint32_t>(RsGlobal) + 8); }

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			CSprite2d__Draw2DPolygon(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
									 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty), rgba);
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{
			// utils::log("%x %d %d %d %d", rgba, rgba[0], rgba[1], rgba[2], rgba[3]);

			// save current font settings
			_CFont__Details	*fd = CFont__Details;
			_CFont__Details fdbcp = *fd;

			// set font align
			fd->m_bCentre = false;
			fd->m_bLeftJustify = false;
			fd->m_bRightJustify = false;
			switch (align)
			{
			case eAlignCenter:
				fd->m_bCentre = true;
				break;
			case eAlignLeft:
				fd->m_bLeftJustify = true;
				break;
			case eAlignRight:
				fd->m_bRightJustify = true;
				break;
			}
			// style
			switch (style)
			{
			case eStyleClassic:
				CFont__SetFontStyle(3);
				scalex *= 1.9;
				scaley *= 1.5;
				break;
			case eStyleSimple:
				CFont__SetFontStyle(2);
				scalex *= 1.4;
				scaley *= 1.3;
				break;
			case eStyleSpecial:
				CFont__SetFontStyle(0);
				break;
			}
			// scale
			fd->m_fScaleX = scalex;
			fd->m_fScaleY = scaley;
			// color
			fd->m_dwColor = *cast<uint32_t*>(rgba);
			// common
			fd->m_bBackground = false;
			fd->m_bFlash = false;
			fd->m_bProp = true;
			fd->m_bDropShadowPosition = 1;
			fd->m_bEdge1 = 0;
			fd->m_bEdge2 = 0;
			fd->m_dwDropColor = 0xFF000000;

			// print string
			CFont__PrintString(trans_x(x), trans_y(y), str);

			// restore font settings
			*fd = fdbcp;
		}

		void init()
		{
			// @CSprite2d::Draw2DPolygon
			CSprite2d__Draw2DPolygon = getsym<fn_CSprite2d__Draw2DPolygon>("_ZN9CSprite2d13Draw2DPolygonEffffffffRK5CRGBA");
			// @CHud::Draw
			_CHud__Draw = getsym<fn_CHud__Draw>("_ZN4CHud4DrawEv");
			armhook::hook_thumb_func(_CHud__Draw, 4, CHud__Draw, &CHud__Draw_);
			// @CFont::Details
			CFont__Details = getsym<_CFont__Details*>("_ZN5CFont7DetailsE");
			// @CFont::SetFontStyle
			CFont__SetFontStyle = getsym<fn_CFont__SetFontStyle>("_ZN5CFont12SetFontStyleEh");
			// @CFont::PrintString
			CFont__PrintString = getsym<fn_CFont__PrintString>("_ZN5CFont11PrintStringEffPt");
			// @RsGlobal
			RsGlobal = getsym<void *>("RsGlobal");

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = { 40, 235, 24, 255 };
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t arrow_color[4] = { 55, 127, 175, 100 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 180;
		}
	}

#endif

	namespace lcs
	{
		// @CSprite2d::Draw2DPolygon
		typedef void (*fn_CSprite2d__Draw2DPolygon)(float, float, float, float, float, float, float, float, uint8_t *);
		fn_CSprite2d__Draw2DPolygon CSprite2d__Draw2DPolygon;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)();
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw()
		{
			CHud__Draw_();
			on_draw();
		}

		// @CFont::Details
		#pragma pack(push, 1)
		struct _CFont__Details
		{
			uint32_t	m_dwColor;				// 0x00
			float		m_fScaleX;	 			// 0x04
			float		m_fScaleY;	 			// 0x08
			float		m_fSlant;	 			// 0x0C
			float		m_fSlantRefPointX;		// 0x10
			float		m_fSlantRefPointY;		// 0x14
			bool		m_bLeftJustify;			// 0x18
			bool		m_bCentre;				// 0x19
			bool		m_bRightJustify;		// 0x1A
			bool		m_bBackground;			// 0x1B
			bool		m_bBackGroundOnlyText;	// 0x1C
			bool		m_bProp;				// 0x1D
			bool		m_bUnk0;				// 0x1E
			bool		m_bFlash;				// 0x1F
			uint32_t	m_dwUnk1;				// 0x20
			float		m_fAlphaFade; 			// 0x24
			uint32_t	m_dwBackgroundColor;	// 0x28
			float		m_fWrapx;	 			// 0x2C
			float		m_fCentreSize; 			// 0x30
			float		m_fRightJustifyWrap;	// 0x34
			uint16_t	m_wFontStyle;			// 0x38
			bool		m_bFontStyle_unk;		// 0x3A
			bool		m_bUnk2;				// 0x3B
			uint32_t	m_dwUnk3;				// 0x3C
			uint16_t	m_wDropShadowPosition;	// 0x40
			uint16_t	m_wUnkPad;				// 0x42
			uint32_t	m_dwDropColor;			// 0x44
			uint32_t	m_dwUnk4[4];			// 0x48
			uint32_t	m_dwOutlineColor;		// 0x58
			uint32_t	m_bOutlineOn;			// 0x5C
			uint32_t	m_dwNewLineAdd;			// 0x60
#ifdef ANDROID
			float		m_fIconScale; 			// 0x64
			float		m_fIconYCorrection;		// 0x68
			float		m_fIconXCorrection;		// 0x6C
#endif
		} *CFont__Details;
		#pragma pack(pop)

#ifdef ANDROID
		STRUCT_SIZE(_CFont__Details, 0x70);
#else
		STRUCT_SIZE(_CFont__Details, 0x64);
#endif

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint16_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(float, float, uint16_t *, float *);
		fn_CFont__PrintString CFont__PrintString;

#ifdef ANDROID
		float trans_x(float x) { return x * 640.0; }
		float trans_y(float y) { return y * 448.0; }
#else
		float trans_x(float x) { return x * 480.0; }
		float trans_y(float y) { return y * 272.0; }
#endif

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			CSprite2d__Draw2DPolygon(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
									 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty), rgba);
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{
			// save current font settings
			_CFont__Details	*fd = CFont__Details;
			_CFont__Details fdbcp = *fd;

			// set font align
			fd->m_bCentre = false;
			fd->m_bLeftJustify = false;
			fd->m_bRightJustify = false;
			switch (align)
			{
			case eAlignCenter:
				fd->m_bCentre = true;
				break;
			case eAlignLeft:
				fd->m_bLeftJustify = true;
				break;
			case eAlignRight:
				fd->m_bRightJustify = true;
				break;
			}
			// style
			switch (style)
			{
			case eStyleClassic:
				CFont__SetFontStyle(2);
#ifdef ANDROID
				scalex *= 1.2f;
				scaley *= 1.3f;
#else
				scalex *= 0.8f;
				scaley *= 0.72f;
#endif
				break;
			case eStyleSimple:
			case eStyleSpecial:
				CFont__SetFontStyle(2);
#ifdef ANDROID
				scalex *= 1.1f;
				scaley *= 1.05f;
#else				
				scalex *= 0.75f;
				scaley *= 0.65f;
#endif
				break;
			}

			// scale
			fd->m_fScaleX = scalex;
			fd->m_fScaleY = scaley;
			// color
			fd->m_dwColor = *cast<uint32_t*>(rgba);
			// common
			fd->m_bBackground = false;
			fd->m_bFlash = false;
			fd->m_bProp = true;
			fd->m_wDropShadowPosition = 1;
			fd->m_dwDropColor = 0xFF000000;

#ifndef ANDROID
			if (psplang::is_lcs_rus1())
			{
				CFont__SetFontStyle(1);
				scalex *= 0.92; 
				scaley *= 0.83;
				if (style == eStyleClassic)
					y -= 0.01;
				fd->m_fScaleX = scalex;
				fd->m_fScaleY = scaley;
				fd->m_wDropShadowPosition = 0;
				fd->m_dwDropColor = 0xFF000000;
			}
#endif

			// print string
			CFont__PrintString(trans_x(x), trans_y(y), str, NULL);

			// restore font settings
			*fd = fdbcp;
		}

		void init()
		{
#ifdef ANDROID
			// @CSprite2d::Draw2DPolygon
			CSprite2d__Draw2DPolygon = getsym<fn_CSprite2d__Draw2DPolygon>("_ZN9CSprite2d13Draw2DPolygonEffffffffRK5CRGBA");
			// @CHud::Draw
			_CHud__Draw = getsym<fn_CHud__Draw>("_ZN4CHud4DrawEv");
			armhook::hook_arm_func(_CHud__Draw, 4, CHud__Draw, &CHud__Draw_);
			// @CFont::Details
			CFont__Details = getsym<_CFont__Details*>("_ZN5CFont7DetailsE");
			// @CFont::SetFontStyle
			CFont__SetFontStyle = getsym<fn_CFont__SetFontStyle>("_ZN5CFont12SetFontStyleEs");
			// @CFont::PrintString
			CFont__PrintString = getsym<fn_CFont__PrintString>("_ZN5CFont11PrintStringEffPtPi");
#else
			uint32_t addr;

			#define PATTERN_NOT_FOUND {	utils::log("required ui pattern %d not found!", __LINE__); exit(1); }
			#define FIND_PATTERN(...) if (!__FindPatternAddress(addr, __VA_ARGS__)) PATTERN_NOT_FOUND;			

			#define READ_ADDR(addr1, addr2) ((static_cast<uint32_t>(*cast<uint16_t *>(addr1)) << 16) | *cast<uint16_t *>(addr2))
			#define READ_ADDR_INDIRECT(addr1, addr2) (READ_ADDR(addr1, addr2) - 0x10000)

			// @CSprite2d::Draw2DPolygon
			FIND_PATTERN("02 03 0C 46 82 03 0E 46 25 20 00 02 42 13 0D 46");
			CSprite2d__Draw2DPolygon = cast<fn_CSprite2d__Draw2DPolygon>(addr - 0x1C);

			// @CHud::Draw
			FIND_PATTERN("8C 07 B4 E7 90 07 B6 E7 94 07 B8 E7 98 07 BA E7");
			_CHud__Draw = cast<fn_CHud__Draw>(addr - 0x0C);
			armhook::hook_mips_func(_CHud__Draw, 8, CHud__Draw, &CHud__Draw_);

			// @CFont::Details
			FIND_PATTERN("25 28 80 00 00 00 A4 90 ?? ?? 06 3C ?? ?? C4 A0");
			CFont__Details = cast<_CFont__Details*>(READ_ADDR_INDIRECT(addr + 8, addr + 12));

			// @CFont::SetFontStyle
			FIND_PATTERN("00 2C 04 00 ?? ?? 04 3C 03 2C 05 00");
			CFont__SetFontStyle = cast<fn_CFont__SetFontStyle>(addr);

			// @CFont::PrintString
			FIND_PATTERN("90 FF BD 27 40 00 B4 E7 44 00 B6 E7 50 00 B0 AF");
			CFont__PrintString = cast<fn_CFont__PrintString>(addr);
#endif

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = {40, 235, 24, 255};
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t color_a[4] = {225, 216, 161, 255};
			memcpy(menu_active_item_font_color, color_a, sizeof(color_a));
			uint8_t arrow_color[4] = { 55, 127, 175, 110 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 220;
		}
	}

#ifndef ANDROID
	namespace vcs
	{
		// @CSprite2d::Draw2DPolygon
		typedef void (*fn_CSprite2d__Draw2DPolygon)(float, float, float, float, float, float, float, float, uint8_t *);
		fn_CSprite2d__Draw2DPolygon CSprite2d__Draw2DPolygon;

		// @CFont::SetFontStyle
		typedef void (*fn_CFont__SetFontStyle)(uint32_t);
		fn_CFont__SetFontStyle CFont__SetFontStyle;

		// @CFont::Unknown_1
		typedef void (*fn_CFont__SetFontPreset1)();
		fn_CFont__SetFontPreset1 CFont__SetFontPreset1;

		// @CFont::SetUnkFloat
		typedef void (*fn_CFont__SetFontScale)(float);
		fn_CFont__SetFontScale CFont__SetFontScale;

		// @CFont::SetFontAlign
		enum eFontAlign
		{
			faNone = 0,
			faLeft = 1,
			faCenter = 2,
			faRight = 4
		};
		typedef void (*fn_CFont__SetFontAlign)(eFontAlign);
		fn_CFont__SetFontAlign CFont__SetFontAlign;

		// @CFont::SetColor
		typedef void (*fn_CFont__SetColor)(uint8_t *rgba);
		fn_CFont__SetColor CFont__SetColor;

		// @CFont::PrintString
		typedef void (*fn_CFont__PrintString)(uint16_t *, int, int);
		fn_CFont__PrintString CFont__PrintString;

		// @CHud::Draw
		typedef void (*fn_CHud__Draw)(void *);
		fn_CHud__Draw _CHud__Draw, CHud__Draw_;
		void CHud__Draw(void *thiz)
		{
			CHud__Draw_(thiz);
			on_draw();
		}

		float trans_x(float x) { return x * 480.0; }
		float trans_y(float y) { return y * 272.0; }

		void _draw_poly(float topleftx, float toplefty, float toprightx, float toprighty,
					  float bottomleftx, float bottomlefty, float bottomrightx, float bottomrighty, uint8_t *rgba)
		{
			CSprite2d__Draw2DPolygon(trans_x(topleftx), trans_y(toplefty), trans_x(toprightx), trans_y(toprighty),
									 trans_x(bottomleftx), trans_y(bottomlefty), trans_x(bottomrightx), trans_y(bottomrighty), rgba);
		}

		void _print_string(uint16_t *str, float x, float y, eAlign align, float scalex, float scaley, uint8_t *rgba, eStyle style)
		{

			eFontAlign font_align = faNone;
			switch (align)
			{
			case eAlignCenter:
				font_align = faCenter;
				break;
			case eAlignLeft:
				font_align = faLeft;
				break;
			case eAlignRight:
				font_align = faRight;
				break;
			}

			int font_style = 0;
			switch (style)
			{
			case eStyleClassic:
				font_style = 2;
				scaley *= 0.72f * 0.4f;
				break;
			case eStyleSimple:				
				if (scalex == 0.4f && scaley == 0.8f) // page index
				{
					font_style = 2;
					font_align = faNone;
					x -= 0.02;
					scaley *= 0.67f * 0.4f;
				} else
				{
					font_style = 2;
					scaley *= 0.67f * 0.3f;
				}				
				break;
			case eStyleSpecial:
				font_style = 0;
				break;
			}

			if (psplang::is_vcs_rus1())
			{
				font_style = 1;
				if (style == eStyleClassic)	scaley *= 1.8; else
				if (style == eStyleSimple)	scaley *= 1.6;
			}

			CFont__SetFontStyle(font_style);
			CFont__SetFontPreset1();
			CFont__SetFontScale(scaley);			
			CFont__SetFontAlign(font_align);
			CFont__SetColor(rgba);

			CFont__PrintString(str, (int)trans_x(x), (int)trans_y(y));
		}

		void init()
		{
			uint32_t addr;

			// @CSprite2d::Draw2DPolygon
			FIND_PATTERN("42 13 0D 46 25 28 00 02 82 03 0E 46 25 30 00 02");
			CSprite2d__Draw2DPolygon = cast<fn_CSprite2d__Draw2DPolygon>(addr - 0x18);

			// @CHud::Draw
			FIND_PATTERN("E0 FF BD 27 ?? ?? 85 93 00 00 B0 AF 25 80 80 00"); // reads gp related addr
			_CHud__Draw = cast<fn_CHud__Draw>(addr);
			armhook::hook_mips_func(_CHud__Draw, 8, CHud__Draw, &CHud__Draw_);

			// @CFont::PrintString
			FIND_PATTERN("25 A0 40 00 25 20 40 02 25 28 20 02 25 30 00 02");
			CFont__PrintString = cast<fn_CFont__PrintString>(addr - 0x38);

			#define READ_REL3_ADDR(offset) memutils::mem_read_mips_jmp(cast<ptr>(addr) + offset)

			FIND_PATTERN("25 28 C0 02 ?? ?? ?? ?? 02 00 04 34");

			// @CFont::SetFontStyle
			CFont__SetFontStyle = cast<fn_CFont__SetFontStyle>(READ_REL3_ADDR(4));

			// @CFont::SetFontPreset1
			CFont__SetFontPreset1 = cast<fn_CFont__SetFontPreset1>(READ_REL3_ADDR(0x0C));

			// @CFont::SetFontAlign
			CFont__SetFontAlign = cast<fn_CFont__SetFontAlign>(READ_REL3_ADDR(0x14));
			
			FIND_PATTERN("06 A3 00 46 00 00 04 92 02 00 05 92");

			// @CFont::SetFontScale
			CFont__SetFontScale = cast<fn_CFont__SetFontScale>(READ_REL3_ADDR(-4));

			// @CFont::SetColor
			CFont__SetColor = cast<fn_CFont__SetColor>(READ_REL3_ADDR(0x34));

			draw_poly = _draw_poly;
			print_string = _print_string;

			uint8_t color[4] = {40, 235, 24, 255};
			memcpy(menu_selected_item_font_color, color, sizeof(color));
			uint8_t color_a[4] = {225, 216, 161, 255};
			memcpy(menu_active_item_font_color, color_a, sizeof(color_a));
			uint8_t arrow_color[4] = { 55, 127, 175, 110 };
			memcpy(menu_arrow_color, arrow_color, sizeof(arrow_color));
			menu_arrow_selected_alpha = 220;
		}
	}
#endif

	// arrow

	static uint16_t unistr[512];
	uint16_t *ansi_to_unicode(LPCSTR str, bool localize = false)
	{		
#ifndef ANDROID
		std::string localized;
		if (localize)
		{
			localized = psplang::localize(str);
			str = localized.c_str();
		}
#endif
		strutils::wstr_from_ansi(unistr, str);
		return unistr;
	}

	// draws arrow, length is 0..1
	void draw_arrow(float length)
	{
		uint8_t arrow_color[4] = { 255, 10, 10, 190 };
		float width = 0.08, top = 0.15, left = 0.5 - width / 2, h = length / 1.75;
		draw_poly(left, top, left + width, top, left, top + h, left + width, top + h, arrow_color);
		float aextent = 0.027, awidth = 0.12;
		draw_poly(left - aextent, top + h, left + width + aextent, top + h, 0.5, top + h + awidth, 0.5, top + h + awidth, arrow_color);
		uint8_t font_color[4] = { 252, 250, 250, 255 };
#ifdef ANDROID
		print_string(ansi_to_unicode("OPEN CLEO MENU", true), 0.5, top - 0.05, eAlignCenter, 0.65, 1.5, font_color, eStyleClassic);
#else
		print_string(ansi_to_unicode("OPEN CLEO MENU", true), 0.5, top - 0.05, eAlignCenter, 0.85, 2.1, font_color, eStyleClassic);
		print_string(ansi_to_unicode("PSP: PRESS START", true), 0.5, top + 0.1, eAlignCenter, 0.85, 2.1, font_color, eStyleClassic);
		print_string(ansi_to_unicode("TO OPEN GAME MENU HOLD START", true), 0.5, top + 0.19, eAlignCenter, 0.5, 1.3, font_color, eStyleClassic);
#endif
	}

	uint32_t arrow_draw_until_time;		// time in ticks til arrow must be drawn
	uint32_t arrow_frames;				// indicates arrow current draw stage

	// shows arrow, called from scripts
	void show_arrow()
	{
		static bool bShowArrow = true;	// arrow shows only at start
		if (bShowArrow)
		{
			bShowArrow = false;
			const uint32_t arrow_draw_time = 7000;
			arrow_draw_until_time = utils::get_tick_count() + arrow_draw_time;
			arrow_frames = 0;
		}
	}

	// hides arrow, called from scripts
	void hide_arrow()
	{
		arrow_draw_until_time = 0;
		arrow_frames = 0;
	}

	// menu

	std::string menu_name;								// title
	std::string menu_close_name;						// close button title
	std::vector<wide_string> menu_items;				// item names
	int32_t menu_active_page = -1;						// active page index, -1 when menu is not active
	int32_t menu_active_item = -1;						// active item index in menu_items
	int32_t menu_page_count;							// total page count, set on create
	int32_t menu_selected_item;							// selected item index in menu_items, updated on touch handle
	uint32_t menu_selected_item_time;					// time in ticks when menu_selected_item was selected
	const int32_t menu_items_per_page = 8;				// max items per page
	int32_t menu_arrow_left_highlight_frames_left;		// frames to draw left arrow highlighted
	int32_t menu_arrow_right_highlight_frames_left;		// frames to draw right arrow highlighted
	int32_t menu_item_highlight_frames_left;			// frames to draw selected item highlighted
	const int32_t menu_highlight_frames = 3;			// frame count for highlight to be active
	// updated on draw
	float menu_close_points[4];							// close button rect coords					[x1,y1,x2,y2]
	float menu_arrow_left_points[8];					// left arrow coords for checking touches 	[x1,y1,x2,y2] * 4
	float menu_arrow_right_points[8];					// right arrow coords for checking touches 	[x1,y1,x2,y2] * 4
	float menu_item_points[menu_items_per_page * 4];	// item coords for checking touches 		[x1,y1,x2,y2] * menu_items_per_page
	uint32_t menu_item_points_current;					// current page item count
	uint32_t hud_draw_time;

	// creates menu, called from scripts
	void create_menu(std::string name, std::string closename, std::vector<wide_string> &items)
	{
		menu_name = name;
		menu_close_name = closename;
		menu_items = items;
		menu_active_page = 0;
		menu_active_item = 0;
		menu_selected_item = -1;
		menu_selected_item_time = 0;
		menu_arrow_left_highlight_frames_left = 0;
		menu_arrow_right_highlight_frames_left = 0;
		menu_item_highlight_frames_left = 0;
		menu_page_count = menu_items.size() / menu_items_per_page;
		if (menu_items.size() % menu_items_per_page)
			menu_page_count++;
	}

	// get menu touched item index
	// resets touch state
	// returns -1 if no menu touch occured, -2 for close button, 0 and above for item indexes
	// maxtime - touch valid time
	int32_t get_menu_touched_item_index(uint32_t maxtime)
	{
		if (utils::get_tick_count() < menu_selected_item_time + maxtime)
		{
			// utils::log("menu_selected_item %d", menu_selected_item);
			menu_selected_item_time = 0; // reset touch state
			return menu_selected_item;
		}
		return -1;
	}

	// deletes menu, called from scripts or on restart
	void delete_menu()
	{
		menu_active_page = -1;
		menu_active_item = -1;
		menu_selected_item = -1;
		menu_selected_item_time = 0;
		menu_page_count = 0;
	}

	void set_menu_active_item_index(int32_t index)
	{
		if (index >= 0 && index < menu_items.size())
		{
			menu_active_item = index;
			menu_active_page = menu_active_item / menu_items_per_page;
		}
	}

	int32_t get_menu_active_item_index()
	{
		return menu_active_item;
	}	


#ifdef ANDROID

	// checks if coords are inside properly defined rect
	bool coords_inside_rect(float x, float y, float x1, float y1, float x2, float y2)
	{
		return (x >= x1 && y >= y1 && x <= x2 && y <= y2);
	}

	// checks if coords are approximately inside 2d poly with 4 verts
	bool coords_approx_inside_rect(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
	{
		float center_x = (x1 + x2 + x3 + x4) / 4, center_y = (y1 + y2 + y3 + y4) / 4;
		float radius = sqrt((x1 - center_x) * (x1 - center_x) + (y1 - center_y) * (y1 - center_y));
		radius += radius / 2;

		float dist = sqrt((x - center_x) * (x - center_x) + (y - center_y) * (y - center_y));
		return dist < radius;
	}

	// handles touches, called in @touch::touch_event()
	void handle_touch(float x, float y)
	{
		// check if menu is active
		if (menu_active_page == -1) return;
		// items
		for (int32_t i = 0; i < menu_item_points_current; i++)
		{
			if (coords_inside_rect(x, y, menu_item_points[i * 4], menu_item_points[i * 4 + 1], menu_item_points[i * 4 + 2], menu_item_points[i * 4 + 3]))
			{
				menu_selected_item = menu_active_page * menu_items_per_page + i;
				menu_selected_item_time = utils::get_tick_count();
				menu_item_highlight_frames_left = menu_highlight_frames;
				return;
			}
		}
		// close button
		if (coords_inside_rect(x, y, menu_close_points[0], menu_close_points[1], menu_close_points[2], menu_close_points[3]))
		{
			menu_selected_item = -2;
			menu_selected_item_time = utils::get_tick_count();
			return;
		}
		// arrows
		if (menu_page_count > 1)
		{
			if (coords_approx_inside_rect(x, y, menu_arrow_left_points[0], menu_arrow_left_points[1], menu_arrow_left_points[2], menu_arrow_left_points[3],
												menu_arrow_left_points[4], menu_arrow_left_points[5], menu_arrow_left_points[6], menu_arrow_left_points[7]))
			{
				if (menu_active_page-- == 0)
					menu_active_page = menu_page_count - 1;
				menu_active_item = menu_active_page * menu_items_per_page; // compat
				menu_arrow_left_highlight_frames_left = menu_highlight_frames;
				return;
			}
			if (coords_approx_inside_rect(x, y, menu_arrow_right_points[0], menu_arrow_right_points[1], menu_arrow_right_points[2], menu_arrow_right_points[3],
												menu_arrow_right_points[4], menu_arrow_right_points[5], menu_arrow_right_points[6], menu_arrow_right_points[7]))
			{
				if (menu_active_page++ == menu_page_count - 1)
					menu_active_page = 0;
				menu_active_item = menu_active_page * menu_items_per_page; // compat
				menu_arrow_right_highlight_frames_left = menu_highlight_frames;
				return;
			}
		}
	}

#else

	void handle_psp_controls()
	{
		if (menu_active_page == -1 || menu_active_item == -1 || hud_draw_time + 150 < utils::get_tick_count()) return;

		bool up = menu_page_count > 0 ? touch::psp_control_pressed(CTRL_UP) || touch::psp_control_pressed(CTRL_STICK_UP) : false;
		bool down = menu_page_count > 0 ? touch::psp_control_pressed(CTRL_DOWN) || touch::psp_control_pressed(CTRL_STICK_DOWN) : false;
		bool cross = menu_page_count > 0 ? touch::psp_control_pressed(CTRL_CROSS) : false;
		bool right = menu_page_count > 1 ? touch::psp_control_pressed(CTRL_RIGHT) || touch::psp_control_pressed(CTRL_STICK_RIGHT) : false;
		bool left = menu_page_count > 1 ? touch::psp_control_pressed(CTRL_LEFT) || touch::psp_control_pressed(CTRL_STICK_LEFT) : false;
		bool circle = touch::psp_control_pressed(CTRL_CIRCLE) || touch::menu_button_pressed_timed(1900);

		int32_t start_item = menu_active_page * menu_items_per_page;
		int32_t menu_items_this_page = (start_item + menu_items_per_page > menu_items.size()) ? menu_items.size() - start_item : menu_items_per_page;

		if (up)
		{
			touch::psp_control_disable(CTRL_UP, 150);
			touch::psp_control_disable(CTRL_STICK_UP, 150);
			menu_active_item--;
			if (menu_active_item < start_item)
				menu_active_item = start_item + menu_items_this_page - 1;
		} else
		if (down)
		{
			touch::psp_control_disable(CTRL_DOWN, 150);
			touch::psp_control_disable(CTRL_STICK_DOWN, 150);
			menu_active_item++;
			if (menu_active_item == start_item + menu_items_this_page)
				menu_active_item = start_item;
		} else
		if (right || left)
		{
			if (right)
			{
				touch::psp_control_disable(CTRL_RIGHT, 200);
				touch::psp_control_disable(CTRL_STICK_RIGHT, 200);
				menu_arrow_right_highlight_frames_left = menu_highlight_frames;
				menu_active_page++;
				if (menu_active_page == menu_page_count)
					menu_active_page = 0;	
			} else
			{
				touch::psp_control_disable(CTRL_LEFT, 200);
				touch::psp_control_disable(CTRL_STICK_LEFT, 200);
				menu_arrow_left_highlight_frames_left = menu_highlight_frames;
				menu_active_page--;
				if (menu_active_page == -1)
					menu_active_page = menu_page_count - 1;
			}
			int32_t current_page_item = menu_active_item % menu_items_per_page;
			start_item = menu_active_page * menu_items_per_page;
			menu_active_item = start_item + current_page_item;
			if (menu_active_item >= menu_items.size())
				menu_active_item = menu_items.size() - 1;
		} else
		if (cross)
		{
			touch::psp_control_disable(CTRL_CROSS, 200);
			menu_selected_item = menu_active_item;
			menu_selected_item_time = utils::get_tick_count();
			menu_item_highlight_frames_left = menu_highlight_frames;
		} else
		if (circle)
		{
			touch::psp_control_disable(CTRL_CIRCLE, 200);
			menu_selected_item = -2;
			menu_selected_item_time = utils::get_tick_count();
		}
	}

#endif

	// menu must be active in order this to be called
	void draw_menu()
	{
		// draw rect
		float width = 0.3, height = 0.8, top = 0.12, left = 0.5 - width / 2;
		uint8_t rect_color[4] = { 55, 127, 175, 150 };
		draw_poly(left, top, left + width, top, left, top + height, left + width, top + height, rect_color);
		// draw title
		uint8_t font_color[4] = { 252, 250, 250, 255 };
		print_string(ansi_to_unicode(menu_name.c_str()), 0.5, top - 0.03, eAlignCenter, 0.8, 1.8, font_color, eStyleClassic);
#ifdef ANDROID
		// draw close button
		menu_close_points[0] = 0.2;
		menu_close_points[1] = 0.87;
		menu_close_points[2] = menu_close_points[0] + 0.1;
		menu_close_points[3] = menu_close_points[1] + 0.1;
		//draw_poly(menu_close_points[0], menu_close_points[1], menu_close_points[2], menu_close_points[1],
		//		  menu_close_points[0], menu_close_points[3], menu_close_points[2], menu_close_points[3], rect_color);
		print_string(ansi_to_unicode(menu_close_name.c_str()), menu_close_points[0], menu_close_points[1], eAlignLeft, 0.6, 1.6, font_color, eStyleClassic);
#endif
		// draw page num if needed
		if (menu_page_count > 1)
		{
			char str[32];
			sprintf(str, "%d of %d", menu_active_page + 1, menu_page_count);
			print_string(ansi_to_unicode(str), 0.61, top + 0.04, eAlignCenter, 0.4f, 0.8f, font_color, eStyleSimple);
		}
		// draw items
		if (menu_active_page < 0 || menu_active_page > menu_page_count - 1)
			menu_active_page = 0;
		int32_t start_item = menu_active_page * menu_items_per_page;
		int32_t menu_items_this_page = (start_item + menu_items_per_page > menu_items.size()) ? menu_items.size() - start_item : menu_items_per_page;
		top = 0.2;
		menu_item_points_current = menu_items_this_page;
		for (int32_t i = 0; i < menu_items_this_page; i++)
		{
			uint8_t font_color[4] = { 252, 250, 250, 255 };
			if (menu_selected_item == start_item + i && menu_item_highlight_frames_left)
			{
				memcpy(font_color, menu_selected_item_font_color, sizeof(font_color));
				menu_item_highlight_frames_left--;
			}

#ifndef ANDROID
			if (menu_active_item == start_item + i && !menu_item_highlight_frames_left) // menu_selected_item != menu_active_item
				memcpy(font_color, menu_active_item_font_color, sizeof(font_color));
#endif

			print_string((uint16_t *)menu_items[start_item + i].c_str(), 0.5, top, eAlignCenter, 0.5, 1.5, font_color, eStyleSimple);
			top += 0.01;
			menu_item_points[i * 4] = 0.5 - width / 2.5;
			menu_item_points[i * 4 + 1] = top;
			top += 0.07;
			menu_item_points[i * 4 + 2] = 0.5 + width / 2.5;
			menu_item_points[i * 4 + 3] = top;
			top += 0.01;
		}
		// draw arrows if needed
		if (menu_page_count > 1)
		{
			// left arrow
			float awidth = 0.08, aheight = 0.2, aextent = 0.015, right = left + width;
			uint8_t arrow_left_color[4];
			memcpy(arrow_left_color, menu_arrow_color, sizeof(menu_arrow_color));
			if (menu_arrow_left_highlight_frames_left)
			{
				arrow_left_color[3] = menu_arrow_selected_alpha;
				menu_arrow_left_highlight_frames_left--;
			}
			menu_arrow_left_points[0] = left - awidth - aextent;
			menu_arrow_left_points[1] = 0.5;
			menu_arrow_left_points[2] = left - aextent;
			menu_arrow_left_points[3] = 0.5 - aheight / 2;
			menu_arrow_left_points[4] = left - aextent;
			menu_arrow_left_points[5] = 0.5 + aheight / 2;
			menu_arrow_left_points[6] = left - awidth - aextent;
			menu_arrow_left_points[7] = 0.5;
			draw_poly(menu_arrow_left_points[0], menu_arrow_left_points[1], menu_arrow_left_points[2], menu_arrow_left_points[3],
					  menu_arrow_left_points[6], menu_arrow_left_points[7],
					  menu_arrow_left_points[4], menu_arrow_left_points[5],
					  arrow_left_color);
			// right arrow
			uint8_t arrow_right_color[4];
			memcpy(arrow_right_color, menu_arrow_color, sizeof(menu_arrow_color));
			if (menu_arrow_right_highlight_frames_left)
			{
				arrow_right_color[3] = menu_arrow_selected_alpha;
				menu_arrow_right_highlight_frames_left--;
			}
			menu_arrow_right_points[0] = right + awidth + aextent;
			menu_arrow_right_points[1] = 0.5;
			menu_arrow_right_points[2] = right + aextent;
			menu_arrow_right_points[3] = 0.5 - aheight / 2;
			menu_arrow_right_points[4] = right + aextent;
			menu_arrow_right_points[5] = 0.5 + aheight / 2;
			menu_arrow_right_points[6] = right + awidth + aextent;
			menu_arrow_right_points[7] = 0.5;
			draw_poly(menu_arrow_right_points[0], menu_arrow_right_points[1], menu_arrow_right_points[2], menu_arrow_right_points[3],
					  menu_arrow_right_points[6], menu_arrow_right_points[7],
					  menu_arrow_right_points[4], menu_arrow_right_points[5],
					  arrow_right_color);
		}
	}

	// common
	void on_draw()
	{
		// arrow
		uint32_t time = utils::get_tick_count();
		hud_draw_time = time;
		if (time < arrow_draw_until_time || arrow_frames > 0)
		{
			const int32_t arrow_frames_max = 40, arrow_freeze_frames_max = 20;

			draw_arrow(arrow_frames < arrow_frames_max ? (float)arrow_frames / (float)arrow_frames_max : 1.0);

			if (arrow_frames++ == arrow_frames_max + arrow_freeze_frames_max)
				arrow_frames = 0;
		}
		// menu
		if (menu_active_page != -1)
			draw_menu();
	}

	void init()
	{
		switch (core::GetGame())
		{
#ifdef ANDROID
		case core::GTA3:
			gta3::init();
			break;
		case core::GTAVC:
			vc::init();
			break;
		case core::GTASA:
			sa::init();
			break;
#endif
		case core::GTALCS:
			lcs::init();
			break;
#ifndef ANDROID
		case core::GTAVCS:
			vcs::init();
			break;
#endif
		}
	}
}
