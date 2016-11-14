ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := first/first.o first_params/first_params.o accel/accel.o

else
# normal makefile
FIRST_DIR = first
FIRST_PARAM_DIR = first_params
ACCEL_DIR = accel

all: $(FIRST_DIR) $(FIRST_PARAM_DIR) $(ACCEL_DIR)

.PHONY: all $(FIRST_DIR) $(FIRST_PARAM_DIR) $(ACCEL_DIR) clean

$(FIRST_DIR):
	$(MAKE) -C $(KDIR) M=$$PWD/$@/

$(FIRST_PARAM_DIR):
	$(MAKE) -C $(KDIR) M=$$PWD/$@/

$(ACCEL_DIR):
	$(MAKE) -C $(KDIR) M=$$PWD/$@/


clean: clean_first clean_first_params clean_accel

clean_first:
	@rm -rf \
	$(FIRST_DIR)/*.ko \
	$(FIRST_DIR)/*.o \
	$(FIRST_DIR)/*.mod.* \
	$(FIRST_DIR)/*.order \
	$(FIRST_DIR)/*.symvers
	@echo "make : removed first files"

clean_first_params:
	@rm -rf \
	$(FIRST_PARAM_DIR)/*.ko \
	$(FIRST_PARAM_DIR)/*.o \
	$(FIRST_PARAM_DIR)/*.mod.* \
	$(FIRST_PARAM_DIR)/*.order \
	$(FIRST_PARAM_DIR)/*.symvers
	@echo "make : removed first-params files"

clean_accel:
	@rm -rf \
	$(ACCEL_DIR)/*.ko \
	$(ACCEL_DIR)/*.o \
	$(ACCEL_DIR)/*.mod.* \
	$(ACCEL_DIR)/*.order \
	$(ACCEL_DIR)/*.symvers
	@echo "make : removed accel files"
endif
