################################################################################
#
# sbp_udp_bridge
#
################################################################################

SBP_UDP_BRIDGE_VERSION = 0.1
SBP_UDP_BRIDGE_SITE = "${BR2_EXTERNAL}/package/sbp_udp_bridge/src"
SBP_UDP_BRIDGE_DEPENDENCIES = czmq libsbp libpiksi 
SBP_UDP_BRIDGE_SITE_METHOD = local

define SBP_UDP_BRIDGE_BUILD_CMDS
    $(MAKE) CC=$(TARGET_CC) LD=$(TARGET_LD) -C $(@D) all
endef

define SBP_UDP_BRIDGE_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/sbp_udp_bridge $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
