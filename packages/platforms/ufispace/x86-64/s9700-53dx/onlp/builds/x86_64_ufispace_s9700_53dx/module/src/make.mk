###############################################################################
#
#
#
###############################################################################

LIBRARY := x86_64_ufispace_s9700_53dx
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk
