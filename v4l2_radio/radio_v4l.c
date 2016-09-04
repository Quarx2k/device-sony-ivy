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
#define SLIM_0_RX_Format "SLIM_0_RX Format"
#define SLIM_0_RX_SampleRate "SLIM_0_RX SampleRate"
static int initMixer() {
    int ret = 0;
    enum mixer_ctl_type type;
    struct mixer_ctl *ctl;
    struct mixer *mixer = mixer_open(0);

    if (mixer == NULL) {
        ERR("Error opening mixer 0");
        return -1;
    }

//SLIM_0_RX_Format
    ctl = mixer_get_ctl_by_name(mixer, SLIM_0_RX_Format);
    if (ctl == NULL) {
        mixer_close(mixer);
        ERR("%s: Could not find %s\n", __func__, SLIM_0_RX_Format);
        return -ENODEV;
    }

    type = mixer_ctl_get_type(ctl);
    if (type != MIXER_CTL_TYPE_ENUM) {
        ERR("%s: %s is not supported, %d\n", __func__, SLIM_0_RX_Format, type);
        mixer_close(mixer);
        return -ENOTTY;
    }
    ret = mixer_ctl_set_enum_by_string(ctl, "S16_LE");
    if (ret < 0)
        return -1;


//SLIM_0_RX_SampleRate
    ctl = mixer_get_ctl_by_name(mixer, SLIM_0_RX_SampleRate);
    if (ctl == NULL) {
        mixer_close(mixer);
        ERR("%s: Could not find %s\n", __func__, SLIM_0_RX_SampleRate);
        return -ENODEV;
    }

    type = mixer_ctl_get_type(ctl);
    if (type != MIXER_CTL_TYPE_ENUM) {
        ERR("%s: %s is not supported, %d\n", __func__, SLIM_0_RX_SampleRate, type);
        mixer_close(mixer);
        return -ENOTTY;
    }
    ret = mixer_ctl_set_enum_by_string(ctl, "KHZ_48");
    if (ret < 0)
        return -1;

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
    ret = mixer_ctl_set_value(ctl, 0, 1);
    if (ret < 0)
        return -1;

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
    ret = mixer_ctl_set_value(ctl, 0, 1);
    if (ret < 0)
        return -1;

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
    ret = mixer_ctl_set_value(ctl, 0, 1);
    if (ret < 0)
        return -1;

    mixer_close(mixer);

    return 0;
}

static int open_device()
{
    int ret = 0;
    INFO("Try open Fm device\n");
    fd = open_dev(DEFAULT_DEVICE);
    if (fd < 0) 
        return -1;
    INFO("Fm device opened\n");

    INFO("Start check tunner radio capability\n");
    ret = get_tun_radio_cap(fd);
    if (ret < 0)
        return -1;
    INFO("End check tunner radio capability\n");;

    INFO("Start get V4L tuner\n");
    struct v4l2_tuner vt;
    vt.index = 0;
    vt.audmode = V4L2_TUNER_MODE_STEREO;
    ret = get_v4l2_tuner(fd, &vt);
    if (ret < 0)
        return -1;
    INFO("End get V4L tuner\n");

    INFO("Start get fact\n");
    float fact = get_fact(fd, &vt);
    if (fact < 0)
        return -1;
    INFO("End get fact %.6f\n", fact);;

    INFO("Start set audmode stereo\n");
    ret = set_force_mono(fd, &vt, false);
    if (ret < 0)
        return -1;
    INFO("End set audmode stereo\n");

    INFO("Start set freq: %.6f\n", 103900*fact);
    ret = set_freq(fd, 103900*fact);
    if (ret < 0)
        return -1;
    INFO("End set freq\n");

    INFO("Start set DEFAULT_VOLUME\n");
    ret = set_volume(fd, 200);
    if (ret < 0)
        return -1;
    INFO("End set DEFAULT_VOLUME\n");

    INFO("Start set MUTE OFF\n");
    ret = set_mute(fd, 0);
    if (ret < 0)
        return -1;
    INFO("End set MUTE_OFF\n");

    INFO("Init mixers\n");
    ret = initMixer();
    if (ret < 0)
        return -1;
    INFO("Mixers inited\n");

    sleep(120);
    close_dev(fd);
    INFO("Fm device closed\n");
    return 0;
}

int main(int argc, char **argv)
{
    INFO("Start fm radio app\n");
    if (open_device() < 1 && fd > 0)
        close_dev(fd);

    return 0;
}

