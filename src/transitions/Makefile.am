AM_CFLAGS = -Wall
AM_CPPFLAGS = -I${top_srcdir}/src
AM_LDFLAGS = -shared -module -avoid-version

plugin_LTLIBRARIES = fade.la spin.la vfade.la noise.la
DATAFILES = fade_files.dpl vfade_files.dpl noise_files.dpl

BUILT_SOURCES = $(DATAFILES:.dpl=.c)
CLEANFILES = $(DATAFILES:.dpl=.c) $(DATAFILES:.dpl=.h)

fade_la_SOURCES = fade.c fade_files.dpl fade.frag
vfade_la_SOURCES = vfade.c vfade_files.dpl vfade.frag
noise_la_SOURCES = noise.c noise_files.dpl noise.frag

EXTRA_DIST = default.c fsquad.vert

.dpl.c:
	@test x"$(@D)" = x || $(MKDIR_P) "$(@D)"
	$(AM_V_GEN)${DATAPACKER} -f $< -s $(dir $<) -d $(DEPDIR)/$(notdir $<).d -e $(basename $@).h -o $@

-include $(addprefix $(DEPDIR)/, $(DATAFILES:=.d))
