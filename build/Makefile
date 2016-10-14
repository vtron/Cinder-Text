TOP_DIR        := /Users/stevevarga/Desktop/Cinder/blocks/Cinder-Typography/build/source/freetype-2.6.5
OBJ_DIR        := /Users/stevevarga/Desktop/Cinder/blocks/Cinder-Typography/build
OBJ_BUILD      := $(OBJ_DIR)
DOC_DIR        := $(OBJ_DIR)/reference
FT_LIBTOOL_DIR := $(OBJ_DIR)
ifndef FT2DEMOS
  include $(TOP_DIR)/Makefile
else
  TOP_DIR_2 := $(TOP_DIR)/../ft2demos
  PROJECT   := freetype
  CONFIG_MK := $(OBJ_DIR)/config.mk
  include $(TOP_DIR_2)/Makefile
endif
