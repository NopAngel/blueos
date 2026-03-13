#
#
#
#
#
#
#
#
#
#

Q 	= @


x86_64:
	$(Q)make -f Makefile.x86

i386:
	$(Q)make -f Makefile.i386 run

menuconfig:
	$(Q)make -f scripts/Makefile all

clean:
	$(Q)cd rust && cargo clean
	$(Q)find .  \
		\( -name '*.a' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o' -o -name '#*#' -o -name '*%' \
		-o -name GPATH -o -name GRTAGS -o -name GSYMS -o -name GTAGS \) \
		-type f -print | xargs rm -f
