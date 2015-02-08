obj-y := msm_fb.o

obj-$(CONFIG_FB_MSM_LOGO) += logo.o
obj-$(CONFIG_FB_BACKLIGHT) += msm_fb_bl.o

ifeq ($(CONFIG_FB_MSM_MDP_HW),y)
# MDP
obj-y += mdp.o

obj-$(CONFIG_DEBUG_FS) += mdp_debugfs.o
#------------------------------------------------------------------------------------1-----
ifeq ($(CONFIG_FB_MSM_MDP40),y)
obj-y += mdp4_util.o
obj-y += mdp4_hsic.o
else
obj-y += mdp_hw_init.o
obj-y += mdp_ppp.o
ifeq ($(CONFIG_FB_MSM_MDP31),y)
obj-y += mdp_ppp_v31.o
else
obj-y += mdp_ppp_v20.o
endif
endif

ifeq ($(CONFIG_FB_MSM_OVERLAY),y)
obj-y += mdp4_overlay.o
obj-y += mdp4_overlay_lcdc.o
ifeq ($(CONFIG_FB_MSM_MIPI_DSI),y)
obj-y += mdp4_overlay_dsi_video.o
obj-y += mdp4_overlay_dsi_cmd.o
else
obj-y += mdp4_overlay_mddi.o
endif
else
obj-y += mdp_dma_lcdc.o
endif
#---------------------------------------------------------------------------------------1----
obj-$(CONFIG_FB_MSM_MDP303) += mdp_dma_dsi_video.o

ifeq ($(CONFIG_FB_MSM_DTV),y)
obj-y += mdp4_dtv.o
obj-y += mdp4_overlay_dtv.o
endif

obj-y += mdp_dma.o
obj-y += mdp_dma_s.o
obj-y += mdp_vsync.o
obj-y += mdp_cursor.o
obj-y += mdp_dma_tv.o
obj-$(CONFIG_ARCH_MSM7X27A) += msm_dss_io_7x27a.o
obj-$(CONFIG_ARCH_MSM8X60) += msm_dss_io_8x60.o
obj-$(CONFIG_ARCH_MSM8960) += msm_dss_io_8960.o

# EBI2
obj-$(CONFIG_FB_MSM_EBI2) += ebi2_lcd.o

# LCDC
obj-$(CONFIG_FB_MSM_LCDC) += lcdc.o

# LVDS
obj-$(CONFIG_FB_MSM_LVDS) += lvds.o

# MDDI
msm_mddi-objs := mddi.o mddihost.o mddihosti.o
obj-$(CONFIG_FB_MSM_MDDI) += msm_mddi.o

# External MDDI
msm_mddi_ext-objs := mddihost_e.o mddi_ext.o
obj-$(CONFIG_FB_MSM_EXTMDDI) += msm_mddi_ext.o

# MIPI gereric
msm_mipi-objs := mipi_dsi.o mipi_dsi_host.o
obj-$(CONFIG_FB_MSM_MIPI_DSI) += msm_mipi.o

# TVEnc
obj-$(CONFIG_FB_MSM_TVOUT) += tvenc.o
ifeq ($(CONFIG_FB_MSM_OVERLAY),y)
obj-$(CONFIG_FB_MSM_TVOUT) += mdp4_overlay_atv.o
endif

# MSM FB Panel
obj-y += msm_fb_panel.o

obj-$(CONFIG_FB_MSM_LCDC_DNJ_OTM8018B_WVGA_IPS) += lcdc_dnj_otm8018b_wvga_ips.o

obj-$(CONFIG_FB_MSM_TVOUT) += tvout_msm.o

ccflags-y := -I$(src)/mhl
obj-$(CONFIG_FB_MSM_HDMI_MHL_8334) += mhl-8334.o
mhl-8334-objs  += mhl/mhl_8334.o
mhl-8334-objs  += mhl/mhl_i2c_utils.o

obj-$(CONFIG_FB_MSM_EXTMDDI_SVGA) += mddi_ext_lcd.o

obj-$(CONFIG_FB_MSM_WRITEBACK_MSM_PANEL) += mdp4_wfd_writeback_panel.o
obj-$(CONFIG_FB_MSM_WRITEBACK_MSM_PANEL) += mdp4_wfd_writeback.o
obj-$(CONFIG_FB_MSM_WRITEBACK_MSM_PANEL) += mdp4_overlay_writeback.o

obj-$(CONFIG_MSM_VIDC_1080P) += vidc/
obj-$(CONFIG_MSM_VIDC_720P) += vidc/
#--------------------------------------------------------------------------------------------------
else
obj-$(CONFIG_FB_MSM_EBI2) += ebi2_host.o
obj-$(CONFIG_FB_MSM_EBI2) += ebi2_lcd.o
obj-y += msm_fb_panel.o
obj-$(CONFIG_FB_MSM_EBI2_EPSON_S1D_QVGA_PANEL) += ebi2_epson_s1d_qvga.o
endif

clean:
	rm *.o .*cmd
