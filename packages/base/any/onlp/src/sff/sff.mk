
###############################################################################
#
# Inclusive Makefile for the sff module.
#
# Autogenerated 2017-05-22 21:57:32.679978
#
###############################################################################
sff_BASEDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(sff_BASEDIR)module/make.mk
include $(sff_BASEDIR)module/auto/make.mk
include $(sff_BASEDIR)module/src/make.mk
include $(sff_BASEDIR)utest/_make.mk

