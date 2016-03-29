#pragma once

int swift_vms_init(void *W);

int swift_vms_call_rpushx(void *W);

#define swift_vms_call_lpushx swift_vms_call_rpushx

int swift_vms_call_publish(void *W);

int swift_vms_idle(void *W);

