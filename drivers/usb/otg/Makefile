#
# OTG infrastructure and transceiver drivers
#

ccflags-$(CONFIG_USB_DEBUG)		:= -DDEBUG
ccflags-$(CONFIG_USB_GADGET_DEBUG)	+= -DDEBUG

# infrastructure
obj-$(CONFIG_USB_OTG_UTILS)	+= otg.o

# transceiver drivers
obj-$(CONFIG_USB_MSM_OTG)	+= msm_otg.o
