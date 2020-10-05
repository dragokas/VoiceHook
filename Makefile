# Makefile
HX_SOURCEMOD = ../sourcemod
HX_SDK = ../hl2sdk-l4d2
HX_METAMOD = ../mmsource
#
# voicehook.ext.so
#
HX_INCLUDE = -I. \
	-I$(HX_SDK)/public \
	-I$(HX_SDK)/public/tier0 \
	-I$(HX_SDK)/public/tier1 \
	-I$(HX_METAMOD)/core \
	-I$(HX_METAMOD)/core/sourcehook \
	-I$(HX_SOURCEMOD)/public \
	-I$(HX_SOURCEMOD)/public/extensions \
	-I$(HX_SOURCEMOD)/public/amtl \
	-I$(HX_SOURCEMOD)/public/amtl/amtl \
	-I$(HX_SOURCEMOD)/sourcepawn/include
#
HX_QWERTY = -D_LINUX \
	-Dstricmp=strcasecmp \
	-D_stricmp=strcasecmp \
	-D_strnicmp=strncasecmp \
	-Dstrnicmp=strncasecmp \
	-D_snprintf=snprintf \
	-D_vsnprintf=vsnprintf \
	-D_alloca=alloca \
	-Dstrcmpi=strcasecmp \
	-Wall \
	-Werror \
	-Wno-switch \
	-Wno-unused \
	-msse \
	-DSOURCEMOD_BUILD \
	-DHAVE_STDINT_H \
	-m32 \
	-DNDEBUG \
	-O3 \
	-funroll-loops \
	-pipe \
	-fno-strict-aliasing \
	-fvisibility=hidden \
	-DCOMPILER_GCC \
	-mfpmath=sse

CPP_FLAGS = -Wno-non-virtual-dtor \
	-Wno-delete-non-virtual-dtor \
	-fvisibility-inlines-hidden \
	-fno-exceptions \
	-fno-rtti \
	-std=c++11
#
HX_SO = Release/smsdk_ext.o \
	Release/extension.o

#
HX_SE = -DSOURCE_ENGINE=9 \
	-DSE_EPISODEONE=1 \
	-DSE_DARKMESSIAH=2 \
	-DSE_ORANGEBOX=3 \
	-DSE_BLOODYGOODTIME=4 \
	-DSE_EYE=5 \
	-DSE_CSS=6 \
	-DSE_ORANGEBOXVALVE=7 \
	-DSE_LEFT4DEAD=8 \
	-DSE_LEFT4DEAD2=9 \
	-DSE_ALIENSWARM=10 \
	-DSE_PORTAL2=11 \
	-DSE_CSGO=12
#
all:
	mkdir -p Release
#
	gcc $(HX_INCLUDE) $(HX_QWERTY) $(CPP_FLAGS) $(HX_SE) -o Release/smsdk_ext.o -c $(HX_SOURCEMOD)/public/smsdk_ext.cpp
	gcc $(HX_INCLUDE) $(HX_QWERTY) $(CPP_FLAGS) $(HX_SE) -o Release/extension.o -c extension.cpp
#
	gcc $(HX_SO) -static-libgcc -shared -m32 -lm -ldl -o Release/voicehook.ext.so
#
	rm -rf Release/*.o
