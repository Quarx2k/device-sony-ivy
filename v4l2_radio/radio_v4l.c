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

#define INFO(fmt, ...) fprintf(stderr, "[INFO] " fmt, ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, "[ERR]  " fmt, ##__VA_ARGS__)
#define DEVICE_NAME "/dev/radio0"

static char *dev_name;
static int fd = -1;

static int getCapabilities() {
        struct v4l2_capability capability;

        int err = ioctl(fd, VIDIOC_QUERYCAP, &capability);
        if (err < 0) {
                ERR("%s, VIDIOC_QUERYCAP error %d\n", __func__, err);
                return -1;
        }
        INFO("%s driver: %s\n", __func__, capability.driver);
        INFO("%s card: %s\n", __func__, capability.card);
        INFO("%s bus_info: %s\n", __func__, capability.bus_info);
        INFO("%s version: %d\n", __func__, capability.version);
        INFO("%s capabilities: %d\n", __func__, capability.capabilities);
        INFO("%s device_caps: %d\n", __func__, capability.device_caps);
        return 0;
}

static int getAudioMode() {
        struct v4l2_audio audio_g;

        int err = ioctl(fd, VIDIOC_G_AUDIO, &audio_g);
        if (err < 0) {
                ERR("%s, VIDIOC_G_AUDIO error %d\n", __func__, err);
                return -1;
        }
        INFO("%s audio capability: %d\n", __func__, audio_g.capability);
        INFO("%s audio name: %s\n", __func__, audio_g.name);
        INFO("%s audio mode: %d\n", __func__, audio_g.mode);
        INFO("%s audio index: %d\n", __func__, audio_g.index);
        return 0;
}

static int getCurrentFreq() {
        struct v4l2_frequency frequency_g;
        frequency_g.tuner = 0;
        int err = ioctl(fd, VIDIOC_G_FREQUENCY, &frequency_g);
        if (err < 0) {
                ERR("%s, VIDIOC_G_TUNER error %d\n", __func__, err);
                return -1;
        }
        INFO("%s current freq: %d\n", __func__, frequency_g.frequency);

        return 0;
}

static int setCurrentFreq(int freq) {
        struct v4l2_frequency frequency_s;
        frequency_s.frequency = freq;
        frequency_s.tuner = 0;
        int err = ioctl(fd, VIDIOC_S_FREQUENCY, &frequency_s);
        if (err < 0) {
                ERR("%s, VIDIOC_S_FREQUENCY error %d\n", __func__, err);
                return -1;
        }
        INFO("%s set current freq: %d\n", __func__, frequency_s.frequency);

        return 0;
}

static int getTunerParams() {
        struct v4l2_tuner tuner_g;
        tuner_g.index = 0;
        int err = ioctl(fd, VIDIOC_G_TUNER, &tuner_g);
        if (err < 0) {
                ERR("%s, VIDIOC_G_TUNER error %d\n", __func__, err);
                return -1;
        }
        INFO("%s low freq: %d\n", __func__, tuner_g.rangelow);
        INFO("%s high freq: %d\n", __func__, tuner_g.rangehigh);
        INFO("%s audio mode: %d\n", __func__, tuner_g.audmode);
        return 0;
}

static int setTunerParams() {
        struct v4l2_tuner tuner_s;
        tuner_s.index = 0;
        tuner_s.audmode = V4L2_TUNER_MODE_STEREO;
        tuner_s.rangelow = 6250; //100.0mhz
        tuner_s.rangehigh = 6750; //108.0mhz
        int err = ioctl(fd, VIDIOC_S_TUNER, &tuner_s);
        if (err < 0) {
                ERR("%s, VIDIOC_S_TUNER error %d\n", __func__, err);
                return -1;
        }
        INFO("%s set audio mode: %d\n", __func__, tuner_s.audmode);
        return 0;
}

static int getControlParams(int id) {
        struct v4l2_control control_g;
        if (id == V4L2_CID_AUDIO_MUTE)
                INFO("%s, V4L2_CID_AUDIO_MUTE:\n",__func__);
        else 
                INFO("%s, V4L2_CID_AUDIO_VOLUME:\n",__func__);

        control_g.id = id;
        int err = ioctl(fd, VIDIOC_G_CTRL, &control_g);
        if (err < 0) {
                ERR("%s, ioctl get control error %d\n", __func__, err);
                return -1;
        }
        INFO("%s control value: %d\n", __func__, control_g.value);
        return 0;
}

static int setControlParams(int id, int value) {
        struct v4l2_control control_s;
        if (id == V4L2_CID_AUDIO_MUTE)
                INFO("%s, V4L2_CID_AUDIO_MUTE:\n",__func__);
        else 
                INFO("%s, V4L2_CID_AUDIO_VOLUME:\n",__func__);

        control_s.id = id;
        control_s.value = value;
        int err = ioctl(fd, VIDIOC_S_CTRL, &control_s);
        if (err < 0) {
                ERR("%s, ioctl set control error %d\n", __func__, err);
                return -1;
        }
        INFO("%s set control value: %d\n", __func__, control_s.value);
        return 0;
}


static int hwFreqSeek(int seek_upward, int wrap_around) {
        struct v4l2_hw_freq_seek hw_freeq_seek_s;
        hw_freeq_seek_s.seek_upward = seek_upward;
        hw_freeq_seek_s.wrap_around = wrap_around;
        int err = ioctl(fd, VIDIOC_S_HW_FREQ_SEEK, &hw_freeq_seek_s);
        if (err < 0) {
                ERR("%s, VIDIOC_S_HW_FREQ_SEEK error %d\n", __func__, err);
                return -1;
        }
        return 0;
}

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

static void open_device(void)
{
        struct stat st;

        if (-1 == stat(dev_name, &st)) {
                ERR("Cannot identify '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
                ERR("%s is no device\n", dev_name);
                exit(EXIT_FAILURE);
        }

        fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                ERR("Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
        INFO("Fm device opened\n");
	getCapabilities();
        setTunerParams();
        getTunerParams();
        setControlParams(V4L2_CID_AUDIO_MUTE, 0);
        setControlParams(V4L2_CID_AUDIO_VOLUME, 50);
        getControlParams(V4L2_CID_AUDIO_MUTE);
        getControlParams(V4L2_CID_AUDIO_VOLUME);
        setCurrentFreq(6525); //104.4
        //hwFreqSeek(6400, 6500);
        getCurrentFreq();
        getAudioMode();
        initMixer();
        sleep(60);
	close(fd);
        INFO("Fm device closed\n");
}

int main(int argc, char **argv)
{
        INFO("Start fm radio\n");
        dev_name = DEVICE_NAME;
        open_device();
        return 0;
}

