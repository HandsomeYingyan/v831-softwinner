
/*
 ******************************************************************************
 *
 * isp_dev.c
 *
 * Hawkview ISP - isp_dev.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/05/11	VIDEO INPUT
 *
 *****************************************************************************
 */
#include <sys/mman.h>

#include <linux/videodev2.h>

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>

#include <fcntl.h>

#include <plat_systrace.h>

#include "../include/device/isp_dev.h"
#include "../include/isp_ini_parse.h"
#include "../include/isp_debug.h"

#include "isp_v4l2_helper.h"
#include "tools.h"

#define MEDIA_DEVICE "/dev/media0"
#define SUNXI_VIDEO "vin_video"
#define SUNXI_ISP "sunxi_isp"
#define SUNXI_H3A "sunxi_h3a"

unsigned int isp_dev_log_param = 0;

struct isp_cid {
	char name[64];
	int id;
};

struct isp_cid isp_cid_array[] = {
	{ "V4L2_CID_BRIGHTNESS", V4L2_CID_BRIGHTNESS, },
	{ "V4L2_CID_CONTRAST", V4L2_CID_CONTRAST, },
	{ "V4L2_CID_SATURATION", V4L2_CID_SATURATION, },
	{ "V4L2_CID_HUE", V4L2_CID_HUE, },
	{ "V4L2_CID_AUTO_WHITE_BALANCE", V4L2_CID_AUTO_WHITE_BALANCE, },
	{ "V4L2_CID_EXPOSURE", V4L2_CID_EXPOSURE, },
	{ "V4L2_CID_AUTOGAIN", V4L2_CID_AUTOGAIN, },
	{ "V4L2_CID_GAIN", V4L2_CID_GAIN, },

	{ "V4L2_CID_POWER_LINE_FREQUENCY", V4L2_CID_POWER_LINE_FREQUENCY, },
	{ "V4L2_CID_HUE_AUTO", V4L2_CID_HUE_AUTO, },
	{ "V4L2_CID_WHITE_BALANCE_TEMPERATURE", V4L2_CID_WHITE_BALANCE_TEMPERATURE, },
	{ "V4L2_CID_SHARPNESS", V4L2_CID_SHARPNESS, },
	{ "V4L2_CID_CHROMA_AGC", V4L2_CID_CHROMA_AGC, },
	{ "V4L2_CID_COLORFX", V4L2_CID_COLORFX, },
	{ "V4L2_CID_AUTOBRIGHTNESS", V4L2_CID_AUTOBRIGHTNESS, },
	{ "V4L2_CID_BAND_STOP_FILTER", V4L2_CID_BAND_STOP_FILTER, },
	{ "V4L2_CID_ILLUMINATORS_1", V4L2_CID_ILLUMINATORS_1, },
	{ "V4L2_CID_ILLUMINATORS_2", V4L2_CID_ILLUMINATORS_2, },

	{ "V4L2_CID_EXPOSURE_AUTO", V4L2_CID_EXPOSURE_AUTO, },
	{ "V4L2_CID_EXPOSURE_ABSOLUTE", V4L2_CID_EXPOSURE_ABSOLUTE, },

	{ "V4L2_CID_EXPOSURE_AUTO_PRIORITY", V4L2_CID_EXPOSURE_AUTO_PRIORITY, },
	{ "V4L2_CID_FOCUS_ABSOLUTE", V4L2_CID_FOCUS_ABSOLUTE, },
	{ "V4L2_CID_FOCUS_RELATIVE", V4L2_CID_FOCUS_RELATIVE, },
	{ "V4L2_CID_FOCUS_AUTO", V4L2_CID_FOCUS_AUTO, },
	{ "V4L2_CID_AUTO_EXPOSURE_BIAS", V4L2_CID_AUTO_EXPOSURE_BIAS, },
	{ "V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE", V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, },

	{ "V4L2_CID_WIDE_DYNAMIC_RANGE", V4L2_CID_WIDE_DYNAMIC_RANGE, },
	{ "V4L2_CID_IMAGE_STABILIZATION", V4L2_CID_IMAGE_STABILIZATION, },

	{ "V4L2_CID_ISO_SENSITIVITY", V4L2_CID_ISO_SENSITIVITY, },
	{ "V4L2_CID_ISO_SENSITIVITY_AUTO", V4L2_CID_ISO_SENSITIVITY_AUTO, },

	{ "V4L2_CID_EXPOSURE_METERING", V4L2_CID_EXPOSURE_METERING, },
	{ "V4L2_CID_SCENE_MODE", V4L2_CID_SCENE_MODE, },

	{ "V4L2_CID_3A_LOCK", V4L2_CID_3A_LOCK, },
	{ "V4L2_CID_AUTO_FOCUS_START", V4L2_CID_AUTO_FOCUS_START, },
	{ "V4L2_CID_AUTO_FOCUS_STOP", V4L2_CID_AUTO_FOCUS_STOP, },
	{ "V4L2_CID_AUTO_FOCUS_RANGE", V4L2_CID_AUTO_FOCUS_RANGE, },

	{ "AE windows x1", V4L2_CID_AE_WIN_X1, },
	{ "AE windows y1", V4L2_CID_AE_WIN_Y1, },
	{ "AE windows x2", V4L2_CID_AE_WIN_X2, },
	{ "AE windows Y2", V4L2_CID_AE_WIN_Y2, },


	{ "AF windows x1", V4L2_CID_AF_WIN_X1, },
	{ "AF windows y1", V4L2_CID_AF_WIN_Y1, },
	{ "AF windows x2", V4L2_CID_AF_WIN_X2, },
	{ "AF windows y2", V4L2_CID_AF_WIN_Y2, },
};

static void isp_subdev_handle_event(void *priv)
{
	struct hw_isp_device *isp = priv;
	struct v4l2_event event;
	int ret, i;
	MPP_AtraceBegin(ATRACE_TAG_ISP, __FUNCTION__);

	memset(&event, 0, sizeof(event));

	ret = ioctl(isp->subdev.fd, VIDIOC_DQEVENT, &event);
	if (ret < 0) {
		ISP_ERR("unable to retrieve isp subdev event: %s (%d)\n", strerror(errno), errno);
		return;
	}

	switch(event.type) {
	case V4L2_EVENT_CTRL:
		for(i = 0; i < array_size(isp_cid_array); i++) {
			if (isp_cid_array[i].id == event.id)
				ISP_DEV_LOG(ISP_LOG_ISP, "name is %s, value is 0x%x\n", isp_cid_array[i].name, event.u.ctrl.value);
		}
		if(isp->ops->ctrl_process)
			isp->ops->ctrl_process(isp, &event);
		break;
	case V4L2_EVENT_FRAME_SYNC:
		isp->load_type = event.u.data[0];
		if(isp->ops->fsync)
			isp->ops->fsync(isp, &event);
		break;
	case V4L2_EVENT_VIN_ISP_OFF:
		if(isp->ops->stream_off)
			isp->ops->stream_off(isp);
		break;
	default:
		ISP_ERR("Unknown enent.\n");
	}
	MPP_AtraceEnd(ATRACE_TAG_ISP);

}

static int isp_subdev_open(struct hw_isp_device *isp)
{
	struct media_entity *entity = NULL;
	struct hw_isp_media_dev *media = isp->priv;
	char name[32];

	snprintf(name, sizeof(name), SUNXI_ISP".%d", (int)isp->id);
	entity = media_get_entity_by_name(media->mdev, name);
	if (entity == NULL)
		return -ENOENT;

	isp->subdev = *entity;

	return v4l2_subdev_open(&isp->subdev);
}

static void isp_subdev_close(struct hw_isp_device *isp)
{
	v4l2_subdev_close(&isp->subdev);
}

static int isp_sensor_open(struct hw_isp_device *isp)
{
	struct media_entity *entity = NULL;

	entity = media_pipeline_get_head(&isp->subdev);
	if (entity == NULL)
		return -ENOENT;

	isp->sensor = *entity;
	return v4l2_subdev_open(&isp->sensor);
}

static void isp_sensor_close(struct hw_isp_device *isp)
{
	v4l2_subdev_close(&isp->sensor);
}

static void isp_stat_process_buffer(void *priv)
{
	struct hw_isp_device *isp = priv;
	struct vin_isp_stat_data data;
	struct v4l2_event event;
	struct vin_isp_stat_event_status *status = (struct vin_isp_stat_event_status *)event.u.data;
	int ret;
	MPP_AtraceBegin(ATRACE_TAG_ISP, __FUNCTION__);

	memset(&event, 0, sizeof event);
	ret = ioctl(isp->stat.fd, VIDIOC_DQEVENT, &event);
	if (ret < 0) {
		ISP_ERR("unable to retrieve AEWB event: %s (%d).\n",
			strerror(errno), errno);
		return;
	}

	if (status->buf_err) {
		ISP_ERR("AEWB: stats error, skipping buffer.\n");
		return;
	}

	memset(&data, 0, sizeof data);
	data.buf = isp->buffer;
	data.buf_size = isp->size;

	ret = ioctl(isp->stat.fd, VIDIOC_VIN_ISP_STAT_REQ, &data);
	if (ret < 0) {
		ISP_ERR("unable to retrieve AEWB data: %s (%d).\n", strerror(errno), errno);
		return;
	}
	if(isp->ops->stats_ready)
		isp->ops->stats_ready(isp, data.buf);
	else
		ISP_DEV_LOG(ISP_LOG_SUBDEV, "Stats: stats_ready is NULL.\n");

	MPP_AtraceEnd(ATRACE_TAG_ISP);
}

static int isp_stat_req_buffers(struct hw_isp_device *isp)
{
	struct vin_isp_h3a_config config;
	int ret;

	config.buf_size = ISP_STAT_TOTAL_SIZE;
	ret = ioctl(isp->stat.fd, VIDIOC_VIN_ISP_H3A_CFG, &config);
	if (ret < 0)
		return -errno;

	isp->size = config.buf_size;
	isp->buffer = malloc(config.buf_size);
	if (isp->buffer == NULL)
		return -ENOMEM;
	return 0;
}

static int isp_stat_free_buffers(struct hw_isp_device *isp)
{
	if (isp->buffer != NULL) {
		free(isp->buffer);
		isp->buffer = NULL;
	}

	return 0;
}

static int isp_stat_open(struct hw_isp_device *isp)
{
	struct media_entity *entity = NULL;
	struct hw_isp_media_dev *media = isp->priv;
	char name[32];

	snprintf(name, sizeof(name), SUNXI_H3A".%d", (int)isp->id);
	ISP_DEV_LOG(ISP_LOG_SUBDEV, "stats device name is %s\n", name);
	entity = media_get_entity_by_name(media->mdev, name);
	if (entity == NULL)
		return -ENOENT;

	isp->stat = *entity;

	return v4l2_subdev_open(&isp->stat);
}

static void isp_stat_close(struct hw_isp_device *isp)
{
	v4l2_subdev_close(&isp->stat);
}

int isp_dev_open(struct hw_isp_media_dev *isp_md, int id)
{
	int ret = -1;
	struct hw_isp_device *isp;
	struct media_entity *entity = NULL;

	if (id >= HW_ISP_DEVICE_NUM)
		return ret;
	if (NULL != isp_md->isp_dev[id])
		return 0;

	isp = malloc(sizeof(struct hw_isp_device));
	if (isp == NULL)
		goto error;
	memset(isp, 0, sizeof(*isp));

	isp->id = id;
	isp->priv = isp_md;

	ret = isp_stat_open(isp);
	if (ret < 0) {
		ISP_ERR("ISP%d open stat video failed!\n", (int)isp->id);
		goto free_isp;
	}

	ret = isp_stat_req_buffers(isp);
	if (ret < 0) {
		ISP_ERR("ISP%d request stat buf failed!\n", (int)isp->id);
		goto free_isp;
	}

	ret = isp_subdev_open(isp);
	if (ret < 0) {
		ISP_ERR("unable to initialize isp subdev.\n");
		goto free_isp;
	}

	ret = isp_sensor_open(isp);
	if (ret < 0) {
		ISP_ERR("unable to initialize sensor subdev.\n");
		goto free_isp;
	}

	ISP_PRINT("open isp device[%d] success!\n", id);
	isp_md->isp_dev[id] = isp;
	return 0;
free_isp:
	free(isp);
error:
	isp_md->isp_dev[id] = NULL;
	ISP_ERR("unable to open isp device[%d]\n", id);
	return -1;
}

void isp_dev_close(struct hw_isp_media_dev *isp_md, int id)
{
	if (isp_md == NULL)
		return;

	if (isp_md->isp_dev[id] == NULL)
		return;

	isp_stat_free_buffers(isp_md->isp_dev[id]);

	isp_stat_close(isp_md->isp_dev[id]);

	isp_subdev_close(isp_md->isp_dev[id]);
	isp_sensor_close(isp_md->isp_dev[id]);

	free(isp_md->isp_dev[id]);
	isp_md->isp_dev[id] = NULL;
}

/*
 * make sure that, it is only one media device
 */
struct hw_isp_media_dev *isp_md_open(const char *devname)
{
	struct hw_isp_media_dev *isp_md;

	isp_md = malloc(sizeof *isp_md);
	if (isp_md == NULL)
		return NULL;
	if((access(devname, F_OK) != 0)) {
		printf("warning: mknod media device %s c 253 0\n", devname);
		system("mknod /dev/media0 c 253 0");
	}

	memset(isp_md, 0, sizeof *isp_md);
	isp_md->mdev = media_open(devname, 0);
	if (isp_md->mdev == NULL) {
		ISP_ERR("error: unable to open isp_md device %s\n", devname);
		free(isp_md);
		return NULL;
	}

	//ISP_PRINT("media open at the first time!!!\n");

	return isp_md;
}

void isp_md_close(struct hw_isp_media_dev *isp_md)
{
	if (isp_md == NULL)
		return;

	media_close(isp_md->mdev);
	free(isp_md);
}

int isp_video_open(struct hw_isp_media_dev *isp_md, unsigned int video_id)
{
	int ret = -1;
	struct isp_video_device *video;

	if (video_id >= HW_VIDEO_DEVICE_NUM)
		return ret;
	if (NULL != isp_md->video_dev[video_id])
		return 0;

	video = malloc(sizeof(struct isp_video_device));
	if (video == NULL)
		goto error;
	memset(video, 0, sizeof(*video));

	video->id = video_id;
	video->priv = isp_md;

	ret = video_init(video);
	if (ret < 0) {
		ISP_ERR("error: unable to initialize video device.\n");
		free(video);
		goto error;
	}

	video->isp_id = media_video_to_isp_id(video->entity);
	if (video->isp_id == -1) {
		ISP_ERR("error: unable to initialize video device.\n");
		video_cleanup(video);
		free(video);
		goto error;
	}

	ISP_PRINT("open video device[%d] success!\n", (int)video_id);
	isp_md->video_dev[video_id] = video;
	return 0;
error:
	isp_md->video_dev[video_id] = NULL;
	ISP_ERR("unable to open video device[%d]!\n", (int)video_id);
	return ret;
}

void isp_video_close(struct hw_isp_media_dev *isp_md, unsigned int video_id)
{
	if (isp_md == NULL)
		return;

	if (isp_md->video_dev[video_id]) {
		video_cleanup(isp_md->video_dev[video_id]);
		free(isp_md->video_dev[video_id]);
	}
	isp_md->video_dev[video_id] = NULL;
}

int isp_video_to_isp_id(int video_id)
{
	struct media_entity *entity = NULL;
	struct media_device *mdev = NULL;
	char name[32];
	int isp_id = 0;

	if (video_id >= HW_VIDEO_DEVICE_NUM)
		return -1;

	mdev = media_open(MEDIA_DEVICE, 0);
	if (mdev == NULL) {
		ISP_ERR("unable to open %s\n", MEDIA_DEVICE);
		return -1;
	}

	snprintf(name, sizeof(name), SUNXI_VIDEO"%d", video_id);
	entity = media_get_entity_by_name(mdev, name);
	if (entity == NULL) {
		ISP_ERR("can not get entity by name %s\n", name);
		return -ENOENT;
	}
	isp_id = media_video_to_isp_id(entity);

	media_close(mdev);

	return isp_id;
}

int isp_video_to_sensor_name(int video_id, char *sensor_name)
{
	struct media_entity *entity = NULL, *head = NULL;
	struct media_device *mdev = NULL;
	char name[32];
	int isp_id = 0;

	if (video_id >= HW_VIDEO_DEVICE_NUM)
		return -1;

	mdev = media_open(MEDIA_DEVICE, 0);
	if (mdev == NULL) {
		ISP_ERR("unable to open %s\n", MEDIA_DEVICE);
		return -ENODEV;
	}

	snprintf(name, sizeof(name), SUNXI_VIDEO"%d", video_id);
	entity = media_get_entity_by_name(mdev, name);
	if (entity == NULL) {
		ISP_ERR("can not get entity by name %s\n", name);
		return -ENOENT;
	}
	head = media_pipeline_get_head(entity);
	if (head == NULL)
		return -ENOENT;

	memcpy(sensor_name, head->info.name, 32);

	media_close(mdev);

	return 0;
}

int isp_dev_start(struct hw_isp_device *isp)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_event_subscription esub;
	unsigned int enable = 1;
	int i, ret;

	/* Start the isp event. */
	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_FRAME_SYNC;
	ret = ioctl(isp->subdev.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to frame sync event: %s (%d).\n", strerror(errno), errno);
		return ret;
	}

	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_VIN_ISP_OFF;
	ret = ioctl(isp->subdev.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to stream off event: %s (%d).\n", strerror(errno), errno);
		return ret;
	}

	for(i = 0; i < array_size(isp_cid_array); i++) {
		memset(&esub, 0, sizeof(struct v4l2_event_subscription));
		esub.id = isp_cid_array[i].id;
		esub.type = V4L2_EVENT_CTRL;
		//esub.flags = V4L2_EVENT_SUB_FL_SEND_INITIAL | V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK;
		ret = ioctl(isp->subdev.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
		if (ret < 0) {
			ISP_ERR("unable to subscribe to ctrl event: %s (%d).\n", strerror(errno), errno);
			return ret;
		}
	}

	isp->ops->monitor_fd(isp->id, isp->subdev.fd, HW_ISP_EVENT_EXCEPTION, isp_subdev_handle_event, isp);

	/* Start the statistics engine. */
	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_VIN_H3A;
	ret = ioctl(isp->stat.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to AEWB event: %s (%d).\n", strerror(errno), errno);
		return ret;
	}
	ret = ioctl(isp->stat.fd, VIDIOC_VIN_ISP_STAT_EN, &enable);
	if (ret < 0) {
		ISP_ERR("unable to start AEWB engine: %s (%d).\n", strerror(errno), errno);
		return ret;
	}

	isp->ops->monitor_fd(isp->id, isp->stat.fd, HW_ISP_EVENT_EXCEPTION, isp_stat_process_buffer, isp);

	return 0;
}

int isp_dev_stop(struct hw_isp_device *isp)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_event_subscription esub;
	unsigned int enable = 0;

	isp->ops->unmonitor_fd(isp->id, isp->subdev.fd);

	memset(&esub, 0, sizeof esub);
	esub.type = V4L2_EVENT_ALL;
	ioctl(isp->subdev.fd, VIDIOC_UNSUBSCRIBE_EVENT, &esub);

	isp->ops->unmonitor_fd(isp->id, isp->stat.fd);
	ioctl(isp->stat.fd, VIDIOC_VIN_ISP_STAT_EN, &enable);

	memset(&esub, 0, sizeof esub);
	esub.type = V4L2_EVENT_VIN_H3A;
	ioctl(isp->stat.fd, VIDIOC_UNSUBSCRIBE_EVENT, &esub);

	return 0;
}

void isp_dev_register(struct hw_isp_device *isp, const struct isp_dev_operations *ops)
{
	isp->ops = ops;
}

void isp_dev_banding_ctx(struct hw_isp_device *isp, void *ctx)
{
	isp->ctx = ctx;
}

void isp_dev_unbanding_ctx(struct hw_isp_device *isp)
{
	isp->ctx = NULL;
}

void *isp_dev_get_ctx(struct hw_isp_device *isp)
{
	if (!isp->ctx) {
		ISP_ERR("ISP[%d] CTX is NULL.\n", (int)isp->id);
		return NULL;
	}
	return isp->ctx;
}

void isp_dev_banding_tuning(struct hw_isp_device *isp, void *tuning)
{
	isp->tuning = tuning;
}

void isp_dev_unbanding_tuning(struct hw_isp_device *isp)
{
	isp->tuning = NULL;
}

void *isp_dev_get_tuning(struct hw_isp_device *isp)
{
	if (NULL == isp->tuning) {
		ISP_ERR("ISP[%d] tuning is NULL.\n", (int)isp->id);
		return NULL;
	}

	return isp->tuning;
}

char *isp_dev_get_sensor_name(struct hw_isp_device *isp)
{
	if (NULL == isp) {
		ISP_ERR("ISP device is NULL.\n");
		return NULL;
	}

	return isp->sensor.info.name;
}

/*
* isp set api
*/
int isp_set_load_reg(struct hw_isp_device *isp, struct isp_table_reg_map *reg)
{
	int ret = 0;

	if (isp->load_type)
		reg->size = ISP_LOAD_DRAM_SIZE;

	ret = ioctl(isp->subdev.fd, VIDIOC_VIN_ISP_LOAD_REG, reg);
	if (ret)
		ISP_ERR("VIDIOC_VIN_ISP_LOAD_REG error!\n");
	return ret;
}

int isp_set_table1_map(struct hw_isp_device *isp, struct isp_table_reg_map *tbl)
{
	int ret = 0;

	if (isp->load_type)
		return ret;

	ret = ioctl(isp->subdev.fd, VIDIOC_VIN_ISP_TABLE1_MAP, tbl);
	if (ret)
		ISP_ERR("VIDIOC_VIN_ISP_TABLE_MAPPING error!\n");
	return ret;
}

int isp_set_table2_map(struct hw_isp_device *isp, struct isp_table_reg_map *tbl)
{
	int ret = 0;

	if (isp->load_type)
		return ret;

	ret = ioctl(isp->subdev.fd, VIDIOC_VIN_ISP_TABLE2_MAP, tbl);
	if (ret)
		ISP_ERR("VIDIOC_VIN_ISP_TABLE_MAPPING error!\n");
	return ret;
}

/*
 * isp sensor api helper
 */
int isp_sensor_get_exposure(struct hw_isp_device *isp,
				  unsigned int *exposure)
{
	struct v4l2_ext_control ctrls[1];
	int ret;

	ctrls[0].id = V4L2_CID_EXPOSURE;

	ret = v4l2_get_controls(&isp->sensor, array_size(ctrls), ctrls);
	if (ret < 0)
		return ret;

	*exposure = ctrls[0].value;
	return 0;
}

int isp_sensor_set_exposure(struct hw_isp_device *isp,
				  unsigned int exposure)
{
	struct v4l2_ext_control ctrls[1];

	ctrls[0].id = V4L2_CID_EXPOSURE;
	ctrls[0].value = exposure;

	return v4l2_set_controls(&isp->sensor, array_size(ctrls), ctrls);
}

int isp_sensor_set_gain(struct hw_isp_device *isp, unsigned int gain)
{
	struct v4l2_ext_control ctrls[1];

	ctrls[0].id = V4L2_CID_GAIN;
	ctrls[0].value = gain;

	return v4l2_set_controls(&isp->sensor, array_size(ctrls), ctrls);
}

int isp_sensor_get_configs(struct hw_isp_device *isp,
						  struct sensor_config *cfg)
{
	int ret;
	ret = ioctl(isp->sensor.fd, VIDIOC_VIN_SENSOR_CFG_REQ, cfg);
	if (ret < 0)
		ISP_ERR("%s get config failed: %s (%d).\n", isp->sensor.info.name, strerror(errno), errno);

	ISP_DEV_LOG(ISP_LOG_SUBDEV, "get sensor config,\n"
		" width: %d, height: %d,\n hoffset: %d, voffset: %d\n"
		" hts: %d, vts: %d,\n pclk: %d, bin_factor: %d\n"
		" intg_min: %d, intg_max: %d,\n gain_min: %d, gain_max: %d\n"
		" mbus code: 0x%08x, wdr mode: 0x%08x\n",
		cfg->width, cfg->height, (int)cfg->hoffset, (int)cfg->voffset,
		(int)cfg->hts, (int)cfg->vts, (int)cfg->pclk, (int)cfg->bin_factor,
		(int)cfg->intg_min, (int)cfg->intg_max, (int)cfg->gain_min, (int)cfg->gain_max,
		(int)cfg->mbus_code, (int)cfg->wdr_mode);
	return ret;
}

int isp_sensor_set_exp_gain(struct hw_isp_device *isp, struct sensor_exp_gain *exp_gain)
{
	int ret;
	static struct sensor_exp_gain exp_gain_save[HW_ISP_DEVICE_NUM];

	if (memcmp(&exp_gain_save[isp->id], exp_gain, sizeof(struct sensor_exp_gain))) {
		exp_gain_save[isp->id] = *exp_gain;
	} else {
		return 0;
	}

	ret = ioctl(isp->sensor.fd, VIDIOC_VIN_SENSOR_EXP_GAIN, exp_gain);
	if (ret < 0)
		ISP_ERR("%s set exp and gain failed: %s (%d).\n",
			isp->sensor.info.name, strerror(errno), errno);

	ISP_DEV_LOG(ISP_LOG_SUBDEV, "set sensor exp gain, exp_val: %d, gain_val: %d, r_gain: %d, b_gain: %d.\n",
		exp_gain->exp_val, exp_gain->gain_val, exp_gain->r_gain, exp_gain->b_gain);
	return ret;
}

int isp_sensor_set_fps(struct hw_isp_device *isp, struct sensor_fps *fps)
{
	int ret;
	ret = ioctl(isp->sensor.fd, VIDIOC_VIN_SENSOR_SET_FPS, fps);
	if (ret < 0)
		ISP_ERR("%s set fps failed: %s (%d).\n",
			isp->sensor.info.name, strerror(errno), errno);
	return ret;
}

int isp_sensor_get_temp(struct hw_isp_device *isp, struct sensor_temp *temp)
{

	int ret = 0;
	ret = ioctl(isp->sensor.fd, VIDIOC_VIN_SENSOR_GET_TEMP, temp);
	if (ret < 0)
		ISP_ERR("%s get sensor_temp failed: %s (%d).\n",
			isp->sensor.info.name, strerror(errno), errno);
	return ret;
}

int isp_act_init_range(struct hw_isp_device *isp, unsigned int min, unsigned int max)
{
	struct actuator_para vcm_range;

	vcm_range.code_min = min;
	vcm_range.code_max = max;

	if (-1 == ioctl(isp->sensor.fd, VIDIOC_VIN_ACT_INIT, &vcm_range)) {
		ISP_ERR("%s init act range failed: %s (%d).\n",
			isp->sensor.info.name, strerror(errno), errno);
		return -1;
	}
	return 0;
}

int isp_act_set_pos(struct hw_isp_device *isp, unsigned int pos)
{
	struct actuator_ctrl vcm_pos;

	vcm_pos.code = pos;

	if (-1 == ioctl(isp->sensor.fd, VIDIOC_VIN_ACT_SET_CODE, &vcm_pos)) {
		ISP_ERR("%s set act position failed: %s (%d).\n",
			isp->sensor.info.name, strerror(errno), errno);
		return -1;
	}
	return 0;
}

