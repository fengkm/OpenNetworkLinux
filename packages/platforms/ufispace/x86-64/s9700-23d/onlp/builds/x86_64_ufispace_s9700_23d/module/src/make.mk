###############################################################################
#
#
#
###############################################################################

LIBRARY := x86_64_ufispace_s9700_23d
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk
