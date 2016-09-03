/*
 * Copyright (C) 2016 Nickolay Semendyaev <agent00791@gmail.com>
 *
 * Sample console app for Broadcom FM Radio.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <system/audio.h>
#include <tinyalsa/asoundlib.h>
#include "include/fmradio.h"
#include "include/v4l2_ioctl.h"

#define INFO(fmt, ...) fprintf(stderr, "[INFO] " fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, "[ERR]  " fmt, ##__VA_ARGS__)
static int fd = -1;

//Sony play-fm switches.
#define PRI_MI2S_LOOPBACK_Volume "PRI MI2S LOOPBACK Volume"
#define SLIMBUS_0_RX_Port_Mixer_PRI_MI2S_TX "SLIMBUS_0_RX Port Mixer PRI_MI2S_TX"
#define SLIMBUS_DL_HL_Switch "SLIMBUS_DL_HL Switch"

static int initMixer() {
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    struct mixer *mixer = mixer_open(0);

    if (mixer == NULL) {
        ERR("Error opening mixer 0");
        return -1;
    }

//PRI_MI2S_LOOPBACK_Volume
 ctl = mixer_get_ctl_by_name(mixer, PRI_MI2S_LOOPBACK_Volume);
    if (ctl == NULL) {
        mixer_close(mixer);
        ERR("%s: Could not find %s\n", __func__, PRI_MI2S_LOOPBACK_Volume);
        return -ENODEV;
    }

    type = mixer_ctl_get_type(ctl);
    if (type != MIXER_CTL_TYPE_INT) {
        ERR("%s: %s is not supported\n", __func__, PRI_MI2S_LOOPBACK_Volume);
        mixer_close(mixer);
        return -ENOTTY;
    }
    mixer_ctl_set_value(ctl, 0, 20);
//SLIMBUS_0_RX_Port_Mixer_PRI_MI2S_TX
   ctl = mixer_get_ctl_by_name(mixer, SLIMBUS_0_RX_Port_Mixer_PRI_MI2S_TX);
    if (ctl == NULL) {
        mixer_close(mixer);
        ERR("%s: Could not find %s\n", __func__, SLIMBUS_0_RX_Port_Mixer_PRI_MI2S_TX);
        return -ENODEV;
    }

    type = mixer_ctl_get_type(ctl);
    if (type != MIXER_CTL_TYPE_BOOL) {
        ERR("%s: %s is not supported\n", __func__, SLIMBUS_0_RX_Port_Mixer_PRI_MI2S_TX);
        mixer_close(mixer);
        return -ENOTTY;
    }
    mixer_ctl_set_value(ctl, 0, 1);
//SLIMBUS_DL_HL_Switch
 ctl = mixer_get_ctl_by_name(mixer, SLIMBUS_DL_HL_Switch);
    if (ctl == NULL) {
        mixer_close(mixer);
        ERR("%s: Could not find %s\n", __func__, SLIMBUS_DL_HL_Switch);
        return -ENODEV;
    }

    type = mixer_ctl_get_type(ctl);
    if (type != MIXER_CTL_TYPE_BOOL) {
        ERR("%s: %s is not supported\n", __func__, SLIMBUS_DL_HL_Switch);
        mixer_close(mixer);
        return -ENOTTY;
    }
    mixer_ctl_set_value(ctl, 0, 1);
    mixer_close(mixer);
    return 0;
}

static void open_device()
{
        INFO("Try open Fm device\n");
        fd = open_dev(DEFAULT_DEVICE);
        INFO("Fm device opened\n");
        INFO("Init mixers\n");
        initMixer();
        INFO("Mixers inited\n");
        close_dev(fd);
        INFO("Fm device closed\n");
}

int main(int argc, char **argv)
{
        INFO("Start fm radio app\n");
        open_device();
        return 0;
}

