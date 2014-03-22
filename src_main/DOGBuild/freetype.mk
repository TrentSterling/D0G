DOG_C_DEFINES := FT2_BUILD_LIBRARY
DOG_C_INCLUDES := "$(DOG_SRC_MAIN)/common/freetype"
DOG_SRC_FILES := \
  vgui2/vgui_surfacelib/freetype/autofit/autofit.c \
  vgui2/vgui_surfacelib/freetype/base/ftbase.c \
  vgui2/vgui_surfacelib/freetype/base/ftbbox.c \
  vgui2/vgui_surfacelib/freetype/base/ftbdf.c \
  vgui2/vgui_surfacelib/freetype/base/ftbitmap.c \
  vgui2/vgui_surfacelib/freetype/base/ftdebug.c \
  vgui2/vgui_surfacelib/freetype/base/ftgasp.c \
  vgui2/vgui_surfacelib/freetype/base/ftglyph.c \
  vgui2/vgui_surfacelib/freetype/base/ftgxval.c \
  vgui2/vgui_surfacelib/freetype/base/ftinit.c \
  vgui2/vgui_surfacelib/freetype/base/ftlcdfil.c \
  vgui2/vgui_surfacelib/freetype/base/ftmm.c \
  vgui2/vgui_surfacelib/freetype/base/ftotval.c \
  vgui2/vgui_surfacelib/freetype/base/ftpatent.c \
  vgui2/vgui_surfacelib/freetype/base/ftpfr.c \
  vgui2/vgui_surfacelib/freetype/base/ftstroke.c \
  vgui2/vgui_surfacelib/freetype/base/ftsynth.c \
  vgui2/vgui_surfacelib/freetype/base/ftsystem.c \
  vgui2/vgui_surfacelib/freetype/base/fttype1.c \
  vgui2/vgui_surfacelib/freetype/base/ftxf86.c \
  vgui2/vgui_surfacelib/freetype/base/ftwinfnt.c \
  vgui2/vgui_surfacelib/freetype/bdf/bdf.c \
  vgui2/vgui_surfacelib/freetype/bzip2/ftbzip2.c \
  vgui2/vgui_surfacelib/freetype/cache/ftcache.c \
  vgui2/vgui_surfacelib/freetype/cff/cff.c \
  vgui2/vgui_surfacelib/freetype/cid/type1cid.c \
  vgui2/vgui_surfacelib/freetype/gzip/ftgzip.c \
  vgui2/vgui_surfacelib/freetype/lzw/ftlzw.c \
  vgui2/vgui_surfacelib/freetype/pcf/pcf.c \
  vgui2/vgui_surfacelib/freetype/pfr/pfr.c \
  vgui2/vgui_surfacelib/freetype/psaux/psaux.c \
  vgui2/vgui_surfacelib/freetype/pshinter/pshinter.c \
  vgui2/vgui_surfacelib/freetype/psnames/psnames.c \
  vgui2/vgui_surfacelib/freetype/raster/raster.c \
  vgui2/vgui_surfacelib/freetype/sfnt/sfnt.c \
  vgui2/vgui_surfacelib/freetype/smooth/smooth.c \
  vgui2/vgui_surfacelib/freetype/truetype/truetype.c \
  vgui2/vgui_surfacelib/freetype/type1/type1.c \
  vgui2/vgui_surfacelib/freetype/type42/type42.c \
  vgui2/vgui_surfacelib/freetype/winfonts/winfnt.c
DOG_STATIC_LIBRARY := true