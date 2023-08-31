###################################################################
# krnllink自动化编译配置文件 Makefile #
# 彭东 #
###################################################################
MAKEFLAGS =
include krnlbuidcmd.mk
include krnlobjs.mk
.PHONY : all everything build_kernel
all: build_kernel
build_kernel:everything
#$(LDER_EXC_BIN)
everything :
	$(LD) $(LDFLAGS) -o $(KERNL_MK_ELFF_FILE) $(BUILD_MK_LINK_OBJS)
	$(OBJCOPY) $(OJCYFLAGS) $(KERNL_MK_ELFF_FILE) $(KERNL_MK_BINF_FILE)
#$(BOOT_EXC_ELF) $(BOOT_EXC_BIN)
$(BOOT_EXC_ELF): $(LMOSEM_LINK_OBJS)
	$(LD) $(LDFLAGS) -o $(BOOT_EXC_ELF) $(LMOSEM_LINK_OBJS)
	@echo 'LD -	M] 正在构建...' $@
$(BOOT_EXC_BIN):
